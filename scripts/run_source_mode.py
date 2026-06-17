#!/usr/bin/env python3
"""Everyday source-mode Run/Reload: build, stage, then reload-or-cold-launch.

The one-button entry point for working on the editor, above CMake (which only
builds + stages). It builds, then connect-probes the running editor's IPC
endpoint (see docs/SourceMode.md):
  * connected -> editor is alive: send `reload` and stop. The editor stages the
    FREE slot itself, so we must NOT stage here or we'd clobber its running slot.
  * refused/absent -> cold start: stage slot A, launch it detached, no debugger.

Staging therefore happens ONLY on the cold-start branch -- the core invariant is
that the running slot is never the stage target. Building is unconditional: it
touches build/ alone, which no running editor maps.

DEFERRED: the editor does not bind the IPC endpoint yet, so the probe always
fails closed and every run cold-starts. Until it does (plus a single-instance
guard), running this while an editor is up re-stages slot A and launches a
duplicate; on Windows the re-stage can fail on the locked mapped DLLs.

--stage-only: build + stage slot A and stop -- no probe, no launch. Pre-launch
step for debug-from-main, where the debugger launches the staged binary. On
Windows it also refreshes the out/source/a-bin junction (see
refresh_windows_launch_junction).

Examples:
  # Everyday Run/Reload (build -> reload-or-cold-launch):
  python scripts/run_source_mode.py --build-dir build/host-mac-debug

  # Pre-launch staging for debug-from-main (build + stage, no launch):
  python scripts/run_source_mode.py --stage-only --build-dir build/host-mac-debug
"""
from __future__ import annotations

import argparse
import socket
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
# Use sys.platform (not platform.system()) so static type checkers narrow the
# Windows-only subprocess constants in launch_cold -- see the guard there.
IS_WINDOWS = sys.platform == "win32"

# Well-known editor endpoint. The running editor binds this; the connect-probe
# below is the liveness test. (Deferred: the editor does not bind it yet.)
WINDOWS_PIPE = r"\\.\pipe\AdHocEditor"

# Per-config suffix on first-party binaries. Mirrors ADHOC_CONFIG_SUFFIX in
# CMakeLists.txt: Debug -> D, Dev -> Dev, Release (and anything else) -> none.
CONFIG_SUFFIX = {"Debug": "D", "Dev": "Dev"}


def read_build_type(build_dir: Path) -> str:
    """Read CMAKE_BUILD_TYPE from the build tree's CMakeCache.txt."""
    cache = build_dir / "CMakeCache.txt"
    try:
        for line in cache.read_text().splitlines():
            # Cache entries look like: CMAKE_BUILD_TYPE:STRING=Dev
            if line.startswith("CMAKE_BUILD_TYPE:"):
                return line.split("=", 1)[1].strip()
    except OSError:
        pass
    print(f"\nERROR: could not read CMAKE_BUILD_TYPE from {cache}. "
          f"Configure the preset first.", file=sys.stderr)
    sys.exit(1)


def editor_binary_name(build_dir: Path) -> str:
    """Derive the staged editor binary filename from the build tree's config.

    Replaces CMake Tools' ${command:cmake.launchTargetFilename} so the everyday
    task needs no launch-target selection: AdHocEditor + per-config suffix, plus
    .exe on Windows.
    """
    suffix = CONFIG_SUFFIX.get(read_build_type(build_dir), "")
    name = f"AdHocEditor{suffix}"
    return f"{name}.exe" if IS_WINDOWS else name


def build(build_dir: Path) -> None:
    """Build the editor binaries only -- no staging. Touches build/ alone, so it
    is safe even while an editor runs from a slot (it maps the staged copies)."""
    cmd = ["cmake", "--build", str(build_dir), "--target", "Launcher", "Engine", "Editor"]
    print(f"==> Building editor: {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, cwd=REPO_ROOT)
    if result.returncode != 0:
        print("\nERROR: build failed.", file=sys.stderr)
        sys.exit(result.returncode)


def stage_slot_a(build_dir: Path) -> None:
    """Stage the freshly-built binaries into slot A (the `source-mode` target).

    Cold-start branch ONLY -- never call this when an editor is live; on reload
    the editor stages the free slot itself.
    """
    cmd = ["cmake", "--build", str(build_dir), "--target", "source-mode"]
    print(f"==> Staging slot A: {' '.join(cmd)}", flush=True)
    result = subprocess.run(cmd, cwd=REPO_ROOT)
    if result.returncode != 0:
        print("\nERROR: staging (source-mode target) failed.", file=sys.stderr)
        sys.exit(result.returncode)


def unix_socket_path(slot_root: Path) -> Path:
    return slot_root / "editor.sock"


def signal_running_editor(slot_root: Path) -> bool:
    """Try to reach a live editor and tell it to reload.

    Returns True if a listening editor was signaled (reload path), False if none
    is running (cold-start path). A successful connect both proves liveness and
    carries the message, so there is no probe-then-deliver TOCTOU gap.
    """
    try:
        if IS_WINDOWS:
            # Named pipes vanish when the server dies, so a missing pipe is a
            # clean "no editor". AF_UNIX is available on modern Windows too, but
            # the named pipe is the idiomatic server side the editor will bind.
            with open(WINDOWS_PIPE, "r+b", buffering=0) as pipe:
                pipe.write(b"reload\n")
            return True

        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
            sock.connect(str(unix_socket_path(slot_root)))
            sock.sendall(b"reload\n")
        return True
    except (ConnectionRefusedError, FileNotFoundError, OSError):
        # Refused / absent / stale node -> nobody is listening -> cold start.
        return False


def resolve_slot_a_binary(slot_root: Path, binary_name: str) -> Path:
    """Resolve the staged slot-A executable for the active platform/config.

    The path is config-dependent (see docs/BuildLayout.md), so probe the
    filesystem rather than parsing the suffix:
      * macOS  -> a/Ad Hoc Editor.app/Contents/MacOS/<binary_name>
      * Windows-> a/debug/<binary_name> (Debug) else a/<binary_name>
    """
    slot_a = slot_root / "a"
    if IS_WINDOWS:
        debug_path = slot_a / "debug" / binary_name
        return debug_path if debug_path.exists() else slot_a / binary_name
    return slot_a / "Ad Hoc Editor.app" / "Contents" / "MacOS" / binary_name


def refresh_windows_launch_junction(slot_root: Path, build_dir: Path) -> None:
    """Point out/source/a-bin at the dir holding the staged slot-A binary.

    Windows-only. The slot-A binary lives in a config-dependent directory
    (`a/debug/` for Debug, `a/` otherwise; see docs/BuildLayout.md), but a
    launch.json `windows` block is a single fixed string that cannot branch on
    CMAKE_BUILD_TYPE. This junction (mklink /J -- no admin rights) gives the debug
    launch config one stable program directory; the binary's colocated DLLs/PDBs
    resolve transparently because Windows derives the DLL search dir from the
    path the exe was loaded through. macOS needs no equivalent (the .app dir is
    config-fixed).
    """
    target_dir = resolve_slot_a_binary(slot_root, editor_binary_name(build_dir)).parent
    junction = slot_root / "a-bin"
    if junction.exists() or junction.is_symlink():
        # rmdir removes the junction (reparse point), never the target's contents.
        subprocess.run(["cmd", "/c", "rmdir", str(junction)], check=False)
    print(f"==> Refreshing launch junction: {junction} -> {target_dir}", flush=True)
    subprocess.run(["cmd", "/c", "mklink", "/J", str(junction), str(target_dir)], check=True)


def launch_cold(binary: Path, build_dir: Path, slot_root: Path) -> None:
    """Launch slot A detached, without a debugger.

    No DYLD preload: an un-traced Launcher self-injects mimalloc via its normal
    re-exec (the production path). See docs/SourceMode.md > Debugging.

    Stdio is redirected to <slot-root>/editor.log, NOT inherited. This script (and
    the VS Code task running it) exits the instant this Popen returns, which closes
    the task terminal's pipe; an editor still writing to that inherited pipe would
    get EPIPE on its next log line, and the logger's fmt::println throws on a short
    write -- an uncaught exception that aborts the editor ("quit unexpectedly").
    Pointing the editor's stdout/stderr at a file decouples it from the launching
    terminal's lifecycle entirely.
    """
    if not binary.exists():
        print(f"\nERROR: staged slot-A binary not found: {binary}", file=sys.stderr)
        sys.exit(1)

    args = [
        str(binary),
        "--source-mode",
        "--build-tree", str((REPO_ROOT / build_dir).resolve()),
        "--slot-root", str((REPO_ROOT / slot_root).resolve()),
    ]
    log_path = slot_root / "editor.log"
    print(f"==> Cold start (no debugger): {binary}", flush=True)
    print(f"    editor stdout/stderr -> {log_path}", flush=True)

    kwargs: dict = {
        "cwd": str(binary.parent),
        "stdin": subprocess.DEVNULL,
        "stderr": subprocess.STDOUT,
    }
    # Direct sys.platform check (not IS_WINDOWS) so the type checker narrows to
    # win32 and recognizes the Windows-only subprocess flags below; an aliased
    # boolean would leave them flagged as unknown attributes off-Windows.
    if sys.platform == "win32":
        kwargs["creationflags"] = (
            subprocess.DETACHED_PROCESS | subprocess.CREATE_NEW_PROCESS_GROUP
        )
    else:
        kwargs["start_new_session"] = True

    # The child dups this fd, so closing our copy after Popen is correct (and the
    # parent exits immediately anyway). "w" truncates: one fresh log per cold start.
    with open(log_path, "w") as log_file:
        subprocess.Popen(args, stdout=log_file, **kwargs)


def main() -> None:
    p = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    p.add_argument("--build-dir", required=True,
                   help="Active CMake build tree, e.g. build/host-mac-debug.")
    p.add_argument("--slot-root", default="out/source",
                   help="Slot root directory (default: out/source).")
    p.add_argument("--stage-only", action="store_true",
                   help="Build + stage slot A and stop (no probe, no launch). "
                        "Pre-launch step for debug-from-main; the debugger "
                        "launches the staged binary itself.")
    args = p.parse_args()

    # Resolve to absolute up front: the cold launch sets the child's cwd to the
    # binary's own directory, so a relative binary path would be re-resolved
    # against that new cwd and fail to exec. Absolute paths are cwd-independent.
    build_dir = (REPO_ROOT / args.build_dir).resolve()
    slot_root = (REPO_ROOT / args.slot_root).resolve()

    build(build_dir)

    if args.stage_only:
        # Debug-from-main pre-launch: (re)stage slot A and stop; the debugger
        # launches the staged binary itself.
        stage_slot_a(build_dir)
        if IS_WINDOWS:
            refresh_windows_launch_junction(slot_root, build_dir)
        print("==> Staged slot A (no launch).", flush=True)
        return

    if signal_running_editor(slot_root):
        # Editor is live: it stages the free slot itself, so we stage nothing.
        print("==> Signaled running editor to reload.", flush=True)
        return

    # Cold start: nothing is running, so slot A is free to (re)stage.
    stage_slot_a(build_dir)
    launch_cold(resolve_slot_a_binary(slot_root, editor_binary_name(build_dir)),
                build_dir, slot_root)


if __name__ == "__main__":
    main()
