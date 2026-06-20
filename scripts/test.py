#!/usr/bin/env python3
"""Wrapper for running AdHocEngine test suites with ctest.

Ad Hoc's configs are expressed as single-config build trees (a separate tree for
Debug, Dev, and Release), and there are multiple gtest projects to run, so this
script helps you run tests for each config, or multiple configs, in one go.

Examples:
  python scripts/test.py debug
  python scripts/test.py release
  python scripts/test.py debug release
  python scripts/test.py --all
  python scripts/test.py dev --filter Assertion
  python scripts/test.py debug --no-build
"""
from __future__ import annotations

import argparse
import platform
import subprocess
import sys
from pathlib import Path

from cmake_utils import find_cmake, find_ctest


REPO_ROOT = Path(__file__).resolve().parent.parent

CONFIGS = ("debug", "dev", "release")


def host_prefix() -> str:
    system = platform.system()
    if system == "Darwin":
        return "host-mac"
    elif system == "Windows":
        return "host-windows"
    else:
        print(f"ERROR: Unsupported platform: {system}", file=sys.stderr)
        sys.exit(1)


def build_tree(build_dir: Path) -> int:
    return subprocess.run([find_cmake(build_dir), "--build", str(build_dir)]).returncode


def run_ctest(build_dir: Path, filter_pattern: str | None, verbose: bool) -> int:
    cmd = [find_ctest(build_dir), "--test-dir", str(build_dir), "--output-on-failure"]
    if filter_pattern:
        cmd += ["-R", filter_pattern]
    if verbose:
        cmd += ["--verbose"]
    return subprocess.run(cmd).returncode


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("configs", nargs="*", metavar="CONFIG",
                   help=f"Config name(s) to test ({', '.join(CONFIGS)}); each is "
                        "expanded to the host-<platform>-<config> preset.")
    p.add_argument("--all", action="store_true",
                   help="Test all host configs (debug, dev, release) for this platform.")
    p.add_argument("--build-dir", default=None,
                   help="Explicit build directory (overrides configs).")
    p.add_argument("--filter", default=None, help="ctest -R pattern.")
    p.add_argument("--no-build", action="store_true",
                   help="Skip building before running tests (assumes binaries are up to date).")
    p.add_argument("--verbose", action="store_true")
    args = p.parse_args()

    # No way to pick a target -> show usage rather than guessing a default.
    if not args.configs and not args.all and not args.build_dir:
        p.print_help()
        sys.exit(0)

    if args.build_dir:
        if args.all or args.configs:
            print("ERROR: --build-dir cannot be combined with configs or --all.",
                  file=sys.stderr)
            sys.exit(1)
        targets = [("(explicit)", Path(args.build_dir))]
    else:
        configs = list(CONFIGS) if args.all else args.configs
        if args.all and args.configs:
            print("ERROR: --all cannot be combined with explicit configs.",
                  file=sys.stderr)
            sys.exit(1)
        unknown = [config for config in configs if config not in CONFIGS]
        if unknown:
            print(f"ERROR: Unknown config(s): {', '.join(unknown)}. "
                  f"Valid configs: {', '.join(CONFIGS)}.", file=sys.stderr)
            sys.exit(1)
        prefix = host_prefix()
        targets = [(f"{prefix}-{config}", REPO_ROOT / "build" / f"{prefix}-{config}")
                   for config in configs]

    results: list[tuple[str, str]] = []
    multiple = len(targets) > 1
    for label, build_dir in targets:
        if multiple:
            print(f"\n{'=' * 70}\nTesting {label}  ({build_dir})\n{'=' * 70}",
                  flush=True)
        if not build_dir.exists():
            print(f"ERROR: Build directory not found: {build_dir}", file=sys.stderr)
            print("Run cmake --preset <preset> and build first.", file=sys.stderr)
            results.append((label, "MISSING"))
            continue
        if not args.no_build:
            code = build_tree(build_dir)
            if code != 0:
                results.append((label, "BUILD FAILED"))
                continue
        code = run_ctest(build_dir, args.filter, args.verbose)
        results.append((label, "PASSED" if code == 0 else "FAILED"))

    if multiple:
        width = max(len(label) for label, _ in results)
        print(f"\n{'=' * 70}\nSummary\n{'=' * 70}")
        for label, status in results:
            print(f"  {label.ljust(width)}  {status}")

    sys.exit(0 if all(status == "PASSED" for _, status in results) else 1)


if __name__ == "__main__":
    main()
