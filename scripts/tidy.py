#!/usr/bin/env python3
"""Run clang-tidy across all first-party sources using a compiled build tree.

Requires the build tree to be configured first (compile_commands.json must exist).
Uses the project-pinned clang-tidy from tools/llvm/current/.

Examples:
  python scripts/tidy.py debug
  python scripts/tidy.py host-windows-release
  python scripts/tidy.py debug release            # multiple configs
  python scripts/tidy.py --all                    # all host configs
  python scripts/tidy.py --fix                    # apply suggested fixes in-place
  python scripts/tidy.py --filter Engine/src      # restrict to a source subtree
"""
from __future__ import annotations

import argparse
import platform
import subprocess
import sys
from pathlib import Path

from cmake_utils import ToolNotFoundError, find_cmake

REPO_ROOT = Path(__file__).resolve().parent.parent

CONFIGS = ("debug", "dev", "release")

SOURCE_DIRS = [
    "Engine/src",
    "Engine/include",
    "EngineTests/src",
    "Editor/src",
    "Editor/include",
    "EditorTests/src",
    "Launcher/src",
]


def find_clang_tidy() -> Path | None:
    exe = "clang-tidy.exe" if sys.platform == "win32" else "clang-tidy"
    pinned = REPO_ROOT / "tools" / "llvm" / "current" / "bin" / exe
    return pinned if pinned.exists() else None


def find_run_clang_tidy() -> Path | None:
    script = REPO_ROOT / "tools" / "llvm" / "current" / "bin" / "run-clang-tidy"
    return script if script.exists() else None


def is_configured(build_dir: Path) -> bool:
    return (build_dir / "CMakeCache.txt").exists()


def configure_preset(preset: str, build_dir: Path) -> int:
    print(f"Configuring preset '{preset}' (build tree not present)...", flush=True)
    try:
        cmake = find_cmake(build_dir)
    except ToolNotFoundError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1
    return subprocess.run([cmake, "--preset", preset], cwd=REPO_ROOT).returncode


def host_prefix() -> str:
    system = platform.system()
    if system == "Darwin":
        return "host-mac"
    elif system == "Windows":
        return "host-windows"
    else:
        print(f"ERROR: Unsupported platform: {system}", file=sys.stderr)
        sys.exit(1)


def run_tidy(preset: str, build_dir: Path, clang_tidy: Path, run_clang_tidy: Path,
             fix: bool, filter_path: str | None) -> int:
    compile_commands = build_dir / "compile_commands.json"
    if not compile_commands.exists():
        print(f"ERROR: {compile_commands} not found.")
        print(f"       Configure the preset first: cmake --preset {preset}")
        return 1

    dirs = SOURCE_DIRS
    if filter_path:
        dirs = [d for d in SOURCE_DIRS if filter_path in d]
        if not dirs:
            dirs = [filter_path]

    # run-clang-tidy resolves each compilation database entry via
    # os.path.abspath, which on Windows produces native backslash paths.
    # The source-filter regex must match those, so escape backslashes.
    def _native_re(p: Path) -> str:
        return str(p).replace("\\", r"\\")

    # run-clang-tidy uses re.match (anchored at start), so wrap alternatives.
    alts = "|".join(_native_re(REPO_ROOT / d) for d in dirs)
    source_regex = f"({alts})"

    cmd = [
        sys.executable,
        str(run_clang_tidy),
        "-clang-tidy-binary", str(clang_tidy.resolve()),
        "-p", str(build_dir),
        "-source-filter", source_regex,
    ]
    if fix:
        cmd.append("-fix")

    print(f"==> Running clang-tidy on preset '{preset}'")
    if filter_path:
        print(f"    Filter: {filter_path}")
    print(f"    Build:  {build_dir}")
    print()

    return subprocess.run(cmd, cwd=REPO_ROOT).returncode


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="python scripts/tidy.py",
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "presets",
        nargs="*",
        metavar="PRESET",
        help="CMake preset name(s) or config shorthand (debug, dev, release). "
             "Defaults to host-<platform>-debug.",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Run clang-tidy on all host configs (debug, dev, release).",
    )
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Apply suggested fixes in-place (clang-tidy --fix).",
    )
    parser.add_argument(
        "--filter",
        metavar="PATH",
        help="Only analyse files whose path contains this substring "
             "(e.g. 'Engine/src', 'Editor').",
    )
    args = parser.parse_args()

    clang_tidy = find_clang_tidy()
    if clang_tidy is None:
        print("ERROR: Pinned clang-tidy not found. Bootstrap the LLVM toolchain first:")
        print("   cmake --preset host-mac-debug      (macOS)")
        print("   cmake --preset host-windows-debug  (Windows)")
        sys.exit(1)

    run_clang_tidy = find_run_clang_tidy()
    if run_clang_tidy is None:
        print("ERROR: run-clang-tidy not found alongside clang-tidy.")
        sys.exit(1)

    if args.all and args.presets:
        print("ERROR: --all cannot be combined with explicit presets.", file=sys.stderr)
        sys.exit(1)

    prefix = host_prefix()
    if args.all:
        targets = [(f"{prefix}-{c}", REPO_ROOT / "build" / f"{prefix}-{c}") for c in CONFIGS]
    elif args.presets:
        targets = []
        for p in args.presets:
            if p in CONFIGS:
                preset = f"{prefix}-{p}"
                targets.append((preset, REPO_ROOT / "build" / preset))
            else:
                preset_path = Path(p)
                build_dir = preset_path if preset_path.is_absolute() else REPO_ROOT / "build" / p
                targets.append((p, build_dir))
    else:
        parser.print_help()
        sys.exit(0)

    results: list[tuple[str, str]] = []
    multiple = len(targets) > 1
    for preset, build_dir in targets:
        if multiple:
            print(f"\n{'=' * 70}\nTidying {preset}  ({build_dir})\n{'=' * 70}", flush=True)
        if not is_configured(build_dir):
            if configure_preset(preset, build_dir) != 0:
                results.append((preset, "CONFIGURE FAILED"))
                continue
        code = run_tidy(preset, build_dir, clang_tidy, run_clang_tidy, args.fix, args.filter)
        results.append((preset, "PASSED" if code == 0 else "FAILED"))

    if multiple:
        width = max(len(label) for label, _ in results)
        print(f"\n{'=' * 70}\nSummary\n{'=' * 70}")
        for label, status in results:
            print(f"  {label.ljust(width)}  {status}")

    sys.exit(0 if all(status == "PASSED" for _, status in results) else 1)


if __name__ == "__main__":
    main()
