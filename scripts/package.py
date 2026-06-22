#!/usr/bin/env python3
"""Build and assemble the shipping AdHocEngine bundle/installer.

Workflow:
  1. Build every shipped (platform, config) preset in parallel.
  2. Install each tree's SourceMode component into a per-preset staging dir.
  3. Merge the staged bundles into a single dist tree. Debug-tree binaries
     already carry a `D` suffix from the build (DEBUG_POSTFIX), so they coexist
     with their Release counterparts without renaming.
  4. Drop in the embedded LLVM toolchain and cross-compiled static libs.
  5. Mac: codesign the merged bundle inside-out (optionally notarize + staple).
     Windows: run makensis on the merged tree.

Usage:
  python scripts/package.py                       # build + assemble (ad-hoc sign)
  python scripts/package.py --platform mac
  python scripts/package.py --identity "Developer ID Application: …" --notarize --keychain-profile AC_NOTARY
"""
from __future__ import annotations

import argparse
import concurrent.futures
import platform
import shutil
import subprocess
import sys
from pathlib import Path

from cmake_utils import ToolNotFoundError, find_cmake


REPO_ROOT = Path(__file__).resolve().parent.parent

# Codesigning entitlements (see cmake/codesign/). The editor set is applied to
# every editor executable; the debugger set is re-applied to debugserver only.
CODESIGN_DIR          = REPO_ROOT / "cmake" / "codesign"
EDITOR_ENTITLEMENTS   = CODESIGN_DIR / "editor.entitlements"
DEBUGGER_ENTITLEMENTS = CODESIGN_DIR / "debugger.entitlements"

# First 4 bytes of a Mach-O image as they appear on disk (thin 32/64 + fat, both
# byte orders). Used to pick code files out of the embedded LLVM toolchain, which
# mixes ~150 Mach-Os among hundreds of headers, scripts, and .a archives.
_MACHO_MAGICS = {
    b"\xcf\xfa\xed\xfe", b"\xfe\xed\xfa\xcf",  # 64-bit thin
    b"\xce\xfa\xed\xfe", b"\xfe\xed\xfa\xce",  # 32-bit thin
    b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca",  # fat (universal)
}

# Shipped (platform, config) matrix per host platform.
# Cross-compile presets are commented out until each platform's triplet/toolchain lands.
SHIP_MATRIX = {
    "mac": [
        "host-mac-debug",
        "host-mac-release",
        # "cross-ios-arm64-debug",
        # "cross-ios-arm64-release",
        # "cross-ios-simulator-arm64-debug",
        # "cross-ios-simulator-arm64-release",
        # "cross-android-arm64-debug",
        # "cross-android-arm64-release",
    ],
    "windows": [
        "host-windows-debug",
        "host-windows-release",
    ],
}


def run(cmd: list[str], **kwargs) -> None:
    print(f"+ {' '.join(str(c) for c in cmd)}", flush=True)
    result = subprocess.run([str(c) for c in cmd], **kwargs)
    if result.returncode != 0:
        sys.exit(result.returncode)


def is_host_preset(preset: str) -> bool:
    return preset.startswith("host-")


def is_cross_preset(preset: str) -> bool:
    return preset.startswith("cross-")


def build_one(preset: str) -> tuple[str, bool, str]:
    """Configure + build one preset. Returns (preset, ok, captured_output)."""
    build_dir = REPO_ROOT / "build" / preset
    try:
        cmake = find_cmake(build_dir)
    except ToolNotFoundError as e:
        return preset, False, f"[{preset}] ERROR: {e}\n"
    captured = []
    for cmd in [
        [cmake, "--preset", preset],
        [cmake, "--build", str(build_dir)],
    ]:
        result = subprocess.run(
            [str(c) for c in cmd],
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        captured.append(f"[{preset}] $ {' '.join(cmd)}\n{result.stdout}")
        if result.returncode != 0:
            return preset, False, "".join(captured)
    return preset, True, "".join(captured)


def stage_preset(preset: str, staging_root: Path) -> Path:
    """Install the SourceMode component for `preset` into a per-preset staging dir."""
    build_dir   = REPO_ROOT / "build" / preset
    staging_dir = staging_root / preset
    if staging_dir.exists():
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True)
    try:
        cmake = find_cmake(build_dir)
    except ToolNotFoundError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
    run([
        cmake, "--install", str(build_dir),
        "--component", "SourceMode",
        "--prefix", str(staging_dir),
    ], cwd=REPO_ROOT)
    return staging_dir


def merge_into_mac_bundle(staging: Path, preset: str, dist_bundle: Path) -> None:
    """Copy a staged "Ad Hoc Editor.app" into dist_bundle, merging from each tree.

    First-party binaries carry their config suffix (e.g. libEngineD.dylib) and
    Debug deps live in Frameworks/debug, so binaries from different configs never
    collide. The release tree seeds the bundle; the debug tree contributes
    MacOS/AdHocEditorD and the Frameworks/debug subtree.
    """
    src = staging / "Ad Hoc Editor.app"
    if not src.exists():
        raise RuntimeError(f"{src} not found — SourceMode install for {preset} failed.")

    def _skip_symlinks(src: str, names: list[str]) -> set[str]:
        return {n for n in names if (Path(src) / n).is_symlink()}

    # First preset seeds the bundle (Info.plist, Resources/, MacOS/, Frameworks/).
    if not dist_bundle.exists():
        shutil.copytree(src, dist_bundle, ignore=_skip_symlinks)
        return

    def merge_dir(src_dir: Path, dst_dir: Path) -> None:
        """Recursively copy files/dirs from src_dir into dst_dir, never overwriting."""
        if not src_dir.exists():
            return
        dst_dir.mkdir(parents=True, exist_ok=True)
        for f in src_dir.iterdir():
            target = dst_dir / f.name
            if f.is_dir() and not f.is_symlink():
                merge_dir(f, target)
            elif not target.exists() and not f.is_symlink():
                shutil.copy2(f, target)

    # Merge Frameworks (incl. the Frameworks/debug subtree) and MacOS binaries.
    merge_dir(src / "Contents" / "Frameworks", dist_bundle / "Contents" / "Frameworks")
    merge_dir(src / "Contents" / "MacOS", dist_bundle / "Contents" / "MacOS")


def _is_macho(path: Path) -> bool:
    """True if `path` is a Mach-O image (thin or fat), by its magic bytes."""
    try:
        with open(path, "rb") as f:
            return f.read(4) in _MACHO_MAGICS
    except OSError:
        return False


def _iter_machos(root: Path):
    """Yield every regular-file Mach-O under `root`, skipping symlinks.

    Symlinks are skipped because Frameworks/ uses versioned symlink chains
    (libfmt.dylib -> …12.dylib -> …12.1.0.dylib); only the real file is signed.
    """
    if not root.exists():
        return
    for p in root.rglob("*"):
        if p.is_file() and not p.is_symlink() and _is_macho(p):
            yield p


def _sign(path: Path, identity: str, *, entitlements: Path | None = None) -> None:
    """codesign one Mach-O / bundle with hardened runtime, replacing any existing sig.

    A secure timestamp is added only for real identities; ad-hoc ("-") can't take
    one. Deliberately no --deep: nested code is signed individually by the caller.
    """
    cmd = ["codesign", "--force", "--options", "runtime"]
    if identity != "-":
        cmd.append("--timestamp")
    if entitlements is not None:
        cmd += ["--entitlements", str(entitlements)]
    cmd += ["--sign", identity, str(path)]
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if result.returncode != 0:
        sys.exit(f"codesign failed for {path}:\n{result.stdout}")


def sign_bundle_inside_out(bundle: Path, identity: str = "-") -> None:
    """Sign every Mach-O in the bundle from the inside out, then seal the .app.

    Each binary gets exactly the entitlements it needs, and the outer .app is
    sealed last so the per-binary signatures are preserved. This is why we avoid
    `codesign --deep`: a deep pass re-signs nested code with empty/wrong
    entitlements, stripping get-task-allow off AdHocEditorD and cs.debugger off
    debugserver. Process entitlements come from the main executable, so dylibs and
    ordinary toolchain tools are signed with none; debugserver is the lone
    exception because it must call task_for_pid to attach a debugger.
    """
    contents   = bundle / "Contents"
    frameworks = contents / "Frameworks"
    llvm       = contents / "Resources" / "llvm"
    macos      = contents / "MacOS"

    # 1. First-party + vcpkg dylibs (incl. Frameworks/debug) — no entitlements.
    fw = list(_iter_machos(frameworks))
    for lib in fw:
        _sign(lib, identity)
    print(f"  Frameworks: signed {len(fw)} Mach-O(s)")

    # 2. Embedded LLVM toolchain — no entitlements, except debugserver.
    tools = list(_iter_machos(llvm))
    for m in tools:
        _sign(m, identity)
    debugserver = llvm / "bin" / "debugserver"
    if debugserver.exists():
        _sign(debugserver, identity, entitlements=DEBUGGER_ENTITLEMENTS)
    print(f"  Toolchain: signed {len(tools)} Mach-O(s)"
          f"{' (+ debugserver: cs.debugger)' if debugserver.exists() else ''}")

    # 3. Editor executables (AdHocEditor, AdHocEditorD) — editor entitlements.
    execs = [p for p in macos.iterdir() if p.is_file() and not p.is_symlink()]
    for exe in execs:
        _sign(exe, identity, entitlements=EDITOR_ENTITLEMENTS)
    print(f"  MacOS: signed {len(execs)} executable(s): {', '.join(e.name for e in execs)}")

    # 4. Seal the bundle last (no --deep) — records the nested signatures above.
    _sign(bundle, identity, entitlements=EDITOR_ENTITLEMENTS)
    print(f"  Sealed bundle ({'ad-hoc' if identity == '-' else identity}).")


def notarize_and_staple(bundle: Path, keychain_profile: str) -> None:
    """Submit the signed bundle to Apple's notary service and staple the ticket.

    Gated behind --notarize (and a real Developer ID identity); never runs by
    default. notarytool needs an archive, not a raw .app.
    """
    zip_path = bundle.with_suffix(".zip")
    run(["ditto", "-c", "-k", "--keepParent", str(bundle), str(zip_path)])
    run(["xcrun", "notarytool", "submit", str(zip_path),
         "--keychain-profile", keychain_profile, "--wait"])
    run(["xcrun", "stapler", "staple", str(bundle)])
    print(f"\n==> Notarized and stapled: {bundle}")


def assemble_mac(presets: list[str], dist_root: Path, identity: str = "-",
                 notarize: bool = False, keychain_profile: str | None = None) -> None:
    staging_root = dist_root / "staging"
    bundle = dist_root / "Ad Hoc Editor.app"
    if bundle.exists():
        shutil.rmtree(bundle)

    host_presets  = [p for p in presets if is_host_preset(p)]
    cross_presets = [p for p in presets if is_cross_preset(p)]

    # Seed bundle from host Release if present, else first host preset.
    seed_order = sorted(host_presets, key=lambda p: 0 if p.endswith("-release") else 1)
    for preset in seed_order:
        merge_into_mac_bundle(stage_preset(preset, staging_root), preset, bundle)

    resources = bundle / "Contents" / "Resources"
    resources.mkdir(parents=True, exist_ok=True)

    # Embedded LLVM toolchain.
    llvm_dest = resources / "llvm"
    if not llvm_dest.exists():
        version = _read_llvm_version()
        llvm_src = REPO_ROOT / "tools" / "llvm" / version
        if llvm_src.exists():
            shutil.copytree(llvm_src, llvm_dest, symlinks=True)

    # Cross-compiled static engine libs (one per cross preset).
    build_libs_dest = resources / "build-libs"
    build_libs_dest.mkdir(exist_ok=True)
    for preset in cross_presets:
        staged = stage_preset(preset, staging_root)
        # cross-* trees install whatever SourceMode component is defined for them.
        # If a cross tree only builds EngineStatic, the staged dir should contain
        # the .a file; copy it under build-libs/<preset>/.
        out_dir = build_libs_dest / preset
        if out_dir.exists():
            shutil.rmtree(out_dir)
        shutil.copytree(staged, out_dir, symlinks=True)

    print("\n==> Signing bundle (inside-out)…")
    sign_bundle_inside_out(bundle, identity)
    print(f"\n==> Bundle assembled at: {bundle}")
    if notarize:
        assert keychain_profile is not None
        notarize_and_staple(bundle, keychain_profile)


def _read_llvm_version() -> str:
    text = (REPO_ROOT / "cmake" / "AdHocLLVMVersion.cmake").read_text()
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("set(ADHOC_LLVM_VERSION"):
            # set(ADHOC_LLVM_VERSION     "20.1.4")
            return line.split('"')[1]
    raise RuntimeError("Could not parse ADHOC_LLVM_VERSION from cmake/AdHocLLVMVersion.cmake")


def assemble_windows(presets: list[str], dist_root: Path) -> None:
    """Merge staged Windows trees into the ship dir.

    NSIS packaging is intentionally deferred — this routine assembles the final
    layout (release AdHocEditor.exe / Engine.dll at the top level; the debug tree
    contributes a debug/ subfolder with AdHocEditorD.exe / EngineD.dll / debug
    DLLs) and emits a manifest. Wire up makensis or cpack against this directory
    once the NSIS template is in place.
    """
    staging_root = dist_root / "staging"
    ship_dir = dist_root / "ship"
    if ship_dir.exists():
        shutil.rmtree(ship_dir)
    ship_dir.mkdir(parents=True)

    for preset in presets:
        staged = stage_preset(preset, staging_root)
        for f in staged.iterdir():
            target = ship_dir / f.name
            if not target.exists():
                if f.is_file():
                    shutil.copy2(f, target)
                elif f.is_dir():
                    shutil.copytree(f, target, symlinks=True)

    # Embedded LLVM toolchain.
    version = _read_llvm_version()
    llvm_src = REPO_ROOT / "tools" / "llvm" / version
    llvm_dest = ship_dir / "llvm"
    if llvm_src.exists() and not llvm_dest.exists():
        shutil.copytree(llvm_src, llvm_dest, symlinks=True)

    print(f"\n==> Ship layout assembled at: {ship_dir}")
    print("    Run makensis (or cpack) against this directory to produce the installer.")


def main() -> None:
    system = platform.system()
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--platform", choices=["mac", "windows"],
                   default="mac" if system == "Darwin" else "windows")
    p.add_argument("--identity", default="-",
                   help='codesign identity; "-" = ad-hoc (default). Pass a '
                        '"Developer ID Application: …" identity to sign for notarization.')
    p.add_argument("--notarize", action="store_true",
                   help="after signing, submit to the notary service and staple "
                        "(requires --identity and --keychain-profile).")
    p.add_argument("--keychain-profile",
                   help="notarytool keychain profile (see `xcrun notarytool store-credentials`).")
    args = p.parse_args()

    if args.notarize:
        if args.platform != "mac":
            p.error("--notarize applies only to the macOS bundle.")
        if args.identity == "-":
            p.error("--notarize needs a Developer ID identity via --identity "
                    "(ad-hoc signatures cannot be notarized).")
        if not args.keychain_profile:
            p.error("--notarize needs --keychain-profile.")

    presets = SHIP_MATRIX[args.platform]
    dist_root = REPO_ROOT / "out" / "dist" / f"host-{args.platform}"
    dist_root.mkdir(parents=True, exist_ok=True)

    # 1. Build every preset in parallel.
    print(f"==> Building {len(presets)} tree(s) in parallel: {', '.join(presets)}",
          flush=True)
    failures: list[tuple[str, str]] = []
    with concurrent.futures.ThreadPoolExecutor() as pool:
        for preset, ok, out in pool.map(build_one, presets):
            print(f"  {preset}: {'OK' if ok else 'FAILED'}", flush=True)
            if not ok:
                failures.append((preset, out))
    if failures:
        print()
        for preset, out in failures:
            print(out, flush=True)
        sys.exit(1)

    # 2. Assemble.
    if args.platform == "mac":
        assemble_mac(presets, dist_root, identity=args.identity,
                     notarize=args.notarize, keychain_profile=args.keychain_profile)
    else:
        assemble_windows(presets, dist_root)


if __name__ == "__main__":
    main()
