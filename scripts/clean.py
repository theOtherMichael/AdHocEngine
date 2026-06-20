#!/usr/bin/env python3
"""Remove build artifacts to test the CMake build system from scratch.

Default: removes build trees, out/ slots, and vcpkg package caches,
         and resets this workspace's CMake Tools state in VS Code (selected
         preset, build/test/launch targets) so a fresh configure starts clean.
         Leaves the LLVM toolchain and the bootstrapped vcpkg binary intact.

  --all: also removes tools/llvm and re-bootstrappable vcpkg artifacts,
         forcing a full re-download on the next configure.
"""
from __future__ import annotations

import argparse
import errno
import os
import shutil
import stat
import sys
from pathlib import Path

from reset_cmake_tools import reset_workspace_cmake_state

REPO_ROOT = Path(__file__).resolve().parent.parent


def _on_rmtree_error(func, path, exc_info) -> None:
    """rmtree error handler: clear read-only/locked entries and retry once.

    Covers files made read-only or flagged (e.g. macOS app-bundle artifacts)
    that block unlink/rmdir.
    """
    os.chmod(path, stat.S_IWRITE | stat.S_IREAD)
    try:
        os.chflags(path, 0)  # macOS only; clears uchg/etc.
    except (AttributeError, OSError):
        pass
    func(path)


def _rmtree(path: Path) -> None:
    """rmtree that tolerates macOS regenerating .DS_Store mid-walk.

    Finder/Spotlight can recreate a .DS_Store after rmtree has listed a
    directory but before it rmdir's it, yielding a spurious ENOTEMPTY. Retry a
    few times so the late-arriving file is swept up.
    """
    for attempt in range(5):
        try:
            shutil.rmtree(path, onerror=_on_rmtree_error)
            return
        except OSError as exc:
            if exc.errno == errno.ENOTEMPTY and attempt < 4:
                continue
            raise


def _remove(path: Path, label: str) -> None:
    if path.exists():
        print(f"Removing {path.relative_to(REPO_ROOT)}  ({label})")
        if path.is_dir():
            _rmtree(path)
        else:
            path.unlink()
    else:
        print(f"Skipping  {path.relative_to(REPO_ROOT)}  (not found)")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "--all",
        action="store_true",
        help="also remove the LLVM toolchain and vcpkg bootstrap artifacts",
    )
    args = parser.parse_args()

    # Reset VS Code's remembered preset / targets for this workspace first, so
    # the "VS Code is running" warning surfaces before anything is deleted.
    reset_workspace_cmake_state()

    # Always removed -------------------------------------------------
    _remove(REPO_ROOT / "build",   "CMake build trees")
    _remove(REPO_ROOT / "out",     "Build output folder")
    _remove(REPO_ROOT / ".cache",  "clangd cache")
    _remove(REPO_ROOT / "Engine" / "include" / "Engine" / "Core" / "Version.h",  "Generated header")

    # vcpkg package cache (buildtrees / packages / downloads are re-fetchable)
    for subdir in ("buildtrees", "packages", "downloads"):
        _remove(REPO_ROOT / "tools" / "vcpkg" / subdir, f"vcpkg {subdir}")

    # --all: heavy artifacts that take significant time to re-acquire --------
    if args.all:
        _remove(REPO_ROOT / "tools" / "llvm", "LLVM toolchain")

        # Remove the bootstrapped vcpkg binary and its marker so bootstrap reruns
        vcpkg_bin = "vcpkg.exe" if sys.platform == "win32" else "vcpkg"
        _remove(REPO_ROOT / "tools" / "vcpkg" / vcpkg_bin,    "vcpkg binary")
        _remove(REPO_ROOT / "tools" / "vcpkg_bootstrapped_commit.txt",   "vcpkg bootstrap marker")
    else:
        print("\nLLVM toolchain and vcpkg binary preserved. Pass --all to remove them too.")

    print("\nDone.")


if __name__ == "__main__":
    main()
