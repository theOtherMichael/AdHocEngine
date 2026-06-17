#!/usr/bin/env python3
"""Configure and build one or more CMake presets in parallel.

Each preset is single-config (e.g. host-mac-debug, cross-ios-arm64-release).
Already-configured trees are not reconfigured unless --reconfigure is passed.

Examples:
  python scripts/build.py                                 # host-<platform>-debug
  python scripts/build.py host-mac-release
  python scripts/build.py host-mac-debug host-mac-release
  python scripts/build.py host-mac-dev cross-ios-arm64-dev
  python scripts/build.py --reconfigure host-mac-debug
"""
from __future__ import annotations

import argparse
import concurrent.futures
import platform
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent


def _run_captured(cmd: list[str], label: str) -> tuple[bool, str]:
    result = subprocess.run(
        [str(c) for c in cmd],
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    header = f"[{label}] $ {' '.join(str(c) for c in cmd)}"
    return result.returncode == 0, f"{header}\n{result.stdout}"


def configure_preset(preset: str, force: bool) -> tuple[bool, str]:
    cache_file = REPO_ROOT / "build" / preset / "CMakeCache.txt"
    if cache_file.exists() and not force:
        return True, f"[{preset}] Already configured — skipping.\n"
    return _run_captured(["cmake", "--preset", preset], preset)


def build_preset(preset: str) -> tuple[bool, str]:
    build_dir = REPO_ROOT / "build" / preset
    return _run_captured(["cmake", "--build", str(build_dir)], preset)


def default_preset() -> str:
    return "host-mac-debug" if platform.system() == "Darwin" else "host-windows-debug"


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("presets", nargs="*",
                   help="Preset names to build (default: host-<platform>-debug).")
    p.add_argument("--reconfigure", action="store_true",
                   help="Force cmake reconfigure even if a build dir already exists.")
    args = p.parse_args()

    presets = args.presets or [default_preset()]

    # Configure sequentially to avoid vcpkg bootstrap races on first run.
    print(f"==> Configuring {len(presets)} preset(s): {', '.join(presets)}", flush=True)
    for preset in presets:
        ok, out = configure_preset(preset, args.reconfigure)
        print(out, end="", flush=True)
        if not ok:
            print(f"\nERROR: configure failed for '{preset}'.", file=sys.stderr)
            sys.exit(1)

    # Build in parallel; ninja parallelises within each tree.
    print(f"\n==> Building {len(presets)} tree(s) in parallel...", flush=True)
    failures: list[str] = []
    outputs: dict[str, str] = {}

    with concurrent.futures.ThreadPoolExecutor() as pool:
        fut_to_preset = {pool.submit(build_preset, pr): pr for pr in presets}
        for fut in concurrent.futures.as_completed(fut_to_preset):
            preset = fut_to_preset[fut]
            ok, out = fut.result()
            outputs[preset] = out
            print(f"  {preset}: {'OK' if ok else 'FAILED'}", flush=True)
            if not ok:
                failures.append(preset)

    if failures:
        print()
        for preset in failures:
            print(outputs[preset], flush=True)
        print(f"\nERROR: build failed for: {', '.join(failures)}", file=sys.stderr)
        sys.exit(1)

    print(f"\n==> Done. Built: {', '.join(presets)}")


if __name__ == "__main__":
    main()
