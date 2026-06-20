#!/usr/bin/env python3
"""Check or apply clang-format across all Ad Hoc Engine source files.

Usage:
    python scripts/format.py           # check for formatting issues
    python scripts/format.py --apply   # apply formatting in-place
"""
from __future__ import annotations

import argparse
import difflib
import os
import pathlib
import subprocess
import sys
from collections.abc import Iterator

ROOT = pathlib.Path(__file__).parent.parent

SOURCE_DIRS = [
    "Engine/src",
    "Engine/include",
    "EngineTests/src",
    "Editor/src",
    "Editor/include",
    "EditorTests/src",
    "Launcher/src",
]

SOURCE_EXTENSIONS = (".h", ".cpp", ".mm")

EXCLUDED_PATHS: set[str] = set()


def find_clang_format() -> str | None:
    """Return the path to the project-pinned clang-format, or None if not bootstrapped."""
    exe = "clang-format.exe" if sys.platform == "win32" else "clang-format"
    pinned = ROOT / "tools" / "llvm" / "current" / "bin" / exe
    return str(pinned) if pinned.exists() else None


def get_source_files() -> Iterator[pathlib.Path]:
    for directory in SOURCE_DIRS:
        for path in (ROOT / directory).rglob("*"):
            if path.suffix in SOURCE_EXTENSIONS:
                if path.relative_to(ROOT).as_posix() not in EXCLUDED_PATHS:
                    yield path


def format_files(files: list[pathlib.Path], clang_format: str) -> None:
    for f in files:
        subprocess.run([clang_format, "-i", "--verbose", str(f)], check=True)


def check_files(files: list[pathlib.Path], clang_format: str) -> None:
    failed = []
    for f in files:
        result = subprocess.run([clang_format, str(f)], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"\nERROR: clang-format failed on {f}")
            print(result.stderr.strip())
            sys.exit(1)

        original = f.read_text()
        if result.stdout != original:
            failed.append(f)
            diff = difflib.unified_diff(
                original.splitlines(),
                result.stdout.splitlines(),
                fromfile=f"BAD:  {os.path.relpath(f, ROOT)} (original)",
                tofile=f"GOOD: {os.path.relpath(f, ROOT)} (formatted)",
                lineterm="",
            )
            print("\n" + "\n".join(diff))

    if failed:
        print(f"\nFAIL: Formatting issues found in {len(failed)} file(s):")
        for f in failed:
            print(f"   {os.path.relpath(f, ROOT)}")
        sys.exit(1)
    else:
        print("OK: All files properly formatted.")


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="python3 scripts/format.py",
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("-a", "--apply", action="store_true", help="apply formatting in-place")
    args = parser.parse_args()

    clang_format = find_clang_format()
    if clang_format is None:
        print("ERROR: Pinned clang-format not found. Bootstrap the LLVM toolchain first:")
        print("   cmake --preset host-mac-debug      (macOS)")
        print("   cmake --preset host-windows-debug  (Windows)")
        sys.exit(1)

    files = list(get_source_files())
    if not files:
        print("No source files found.")
        return

    if args.apply:
        format_files(files, clang_format)
    else:
        check_files(files, clang_format)


if __name__ == "__main__":
    main()
