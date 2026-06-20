"""Shared CMake utilities for build scripts."""
from __future__ import annotations

import sys
from pathlib import Path


def read_cache_value(build_dir: Path, key: str) -> str | None:
    """Read a typed entry from build_dir/CMakeCache.txt; returns None if absent.

    Cache entries have the form  KEY:TYPE=value  (e.g. CMAKE_BUILD_TYPE:STRING=Debug).
    Matching is on the key name only, before the colon.
    """
    cache = build_dir / "CMakeCache.txt"
    try:
        for line in cache.read_text().splitlines():
            if line.startswith(f"{key}:"):
                return line.split("=", 1)[1].strip()
    except OSError:
        pass
    return None


def read_cmake_command(build_dir: Path) -> str:
    """Return the cmake executable recorded in build_dir/CMakeCache.txt.

    CMAKE_COMMAND:INTERNAL is written by cmake after every configure, so this
    returns the exact binary that owns the build tree — no PATH dependency.
    Exits with an error when the cache is absent (tree not yet configured).
    """
    cmake = read_cache_value(build_dir, "CMAKE_COMMAND")
    if cmake:
        return cmake
    cache = build_dir / "CMakeCache.txt"
    print(f"\nERROR: could not read CMAKE_COMMAND from {cache}. "
          f"Configure the preset first.", file=sys.stderr)
    sys.exit(1)


def find_cmake(build_dir: Path | None = None) -> str:
    """Return a cmake executable, preferring the one recorded in the build cache.

    When build_dir is given and its CMakeCache.txt exists, returns the cmake
    that configured the tree (CMAKE_COMMAND:INTERNAL). Falls back to the string
    'cmake' (resolved at subprocess time via PATH) when the cache is absent —
    this covers the initial configure, before the cache exists.
    """
    if build_dir is not None:
        cmake = read_cache_value(build_dir, "CMAKE_COMMAND")
        if cmake:
            return cmake
    return "cmake"


def find_ctest(build_dir: Path | None = None) -> str:
    """Return the ctest executable that matches the cmake owning the build tree.

    Derives ctest from CMAKE_COMMAND (they live in the same directory). Falls
    back to the string 'ctest' when the cache is absent.
    """
    if build_dir is not None:
        cmake = read_cache_value(build_dir, "CMAKE_COMMAND")
        if cmake:
            return str(Path(cmake).with_name("ctest"))
    return "ctest"
