#!/usr/bin/env python3
import argparse
import subprocess
import pathlib
import sys
import difflib
import os
import shutil

directories = [
    "Engine/src",
    "Engine/include",
    "EngineTests/src",
    "Editor/src",
    "Editor/include",
    "EditorTests/src",
    "Launcher/src",
]

extensions = (".h", ".cpp", ".mm")


excluded_paths = {
    "Launcher/src/_platform/Windows/resource.h",
    "Engine/src/_platform/Windows/resource.h",
    "Editor/src/_platform/Windows/resource.h",
}


def get_source_files():
    root = pathlib.Path(__file__).parent.parent
    for directory in directories:
        for path in (root / directory).rglob("*"):
            if path.suffix in extensions:
                # Get path relative to project root
                project_relative_path = path.relative_to(root).as_posix()
                if project_relative_path not in excluded_paths:
                    yield path


def format_files(files):
    subprocess.run(["clang-format", "-i", "--verbose", *map(str, files)], check=True)


def check_files(files):
    failed = []
    for f in files:
        result = subprocess.run(
            ["clang-format", str(f)], capture_output=True, text=True
        )

        if result.returncode != 0:
            print(f"\n❌ clang-format failed on {f}")
            print(result.stderr.strip())
            sys.exit(1)  # immediate stop, since it's not just bad formatting

        original = f.read_text()
        formatted = result.stdout

        root = pathlib.Path(__file__).parent.parent

        if original != formatted:
            failed.append(f)
            
            # Print a diff

            print()
            diff = difflib.unified_diff(
                original.splitlines(),
                formatted.splitlines(),
                fromfile=f"❌ {os.path.relpath(f, root)} (original)",
                tofile=f"✅ {os.path.relpath(f, root)} (formatted)",
                lineterm="",
            )
            print("\n".join(diff))

    if failed:
        print(f"\n❌ Formatting issues found in {len(failed)} file(s):")
        for f in failed:
            print(f"   {f}")
        sys.exit(1)
    else:
        print("✅ All files properly formatted.")


def main():
    parser = argparse.ArgumentParser(
        prog="python3 clang_format.py",
        description="Check or format all code files in Ad Hoc Engine.",
    )

    parser.add_argument(
        "-f",
        "--format",
        action="store_true",
        help="if specified, formatting changes are saved to file",
    )
    args = parser.parse_args()

    if shutil.which("clang-format") is None:
        print("❌ clang-format is not installed or not on PATH.")
        print("   macOS:   brew install clang-format")
        print("   Windows: install LLVM from https://llvm.org/releases/download.html")
        sys.exit(1)

    files = list(get_source_files())
    if not files:
        print("No source files found.")
        return

    if args.format:
        format_files(files)
    else:
        check_files(files)


if __name__ == "__main__":
    main()
