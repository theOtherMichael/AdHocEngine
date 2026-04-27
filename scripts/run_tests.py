#!/usr/bin/env python3
import argparse
import pathlib
import subprocess
import sys

SOLUTION = "AdHocEngine.sln"
PLATFORM = "x64"

ALL_CONFIGS = ["Debug", "Dev", "Release", "StaticDebug", "StaticDev", "StaticRelease"]
EDITOR_CONFIGS = {"Debug", "Dev", "Release"}

# Maps config -> (EngineTests exe stem, EditorTests exe stem or None)
TEST_EXES = {
    "Debug":         ("EngineTestsD",        "EditorTestsD"),
    "Dev":           ("EngineTestsDev",       "EditorTestsDev"),
    "Release":       ("EngineTests",          "EditorTests"),
    "StaticDebug":   ("EngineTestsStaticD",   None),
    "StaticDev":     ("EngineTestsStaticDev", None),
    "StaticRelease": ("EngineTestsStatic",    None),
}


def find_msbuild():
    vswhere = pathlib.Path(
        r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    )
    if not vswhere.exists():
        sys.exit("vswhere.exe not found — is Visual Studio installed?")
    result = subprocess.run(
        [
            str(vswhere), "-latest",
            "-requires", "Microsoft.Component.MSBuild",
            "-find", r"MSBuild\**\Bin\MSBuild.exe",
        ],
        capture_output=True,
        text=True,
    )
    lines = [line.strip() for line in result.stdout.strip().splitlines() if line.strip()]
    if not lines:
        sys.exit("MSBuild not found via vswhere.")
    return lines[0]


def build(msbuild, root, config, project):
    print(f"  Building {project} ({config})...")
    result = subprocess.run(
        [
            msbuild, str(root / SOLUTION),
            f"/p:Configuration={config}",
            f"/p:Platform={PLATFORM}",
            f"/t:{project}",
            "/m",
            "/v:minimal",
            "/nologo",
        ],
        cwd=root,
    )
    succeeded = result.returncode == 0
    print(f"  [{'OK' if succeeded else 'FAIL'}] Build {'succeeded' if succeeded else 'FAILED'}: {project} ({config})")
    return succeeded


def run_tests(exe, config, project):
    print(f"\n  Running {project} ({config})...")
    result = subprocess.run([str(exe)])
    return result.returncode == 0


def suites_for(config):
    engine_exe, editor_exe = TEST_EXES[config]
    suites = [("EngineTests", engine_exe)]
    if config in EDITOR_CONFIGS:
        suites.append(("EditorTests", editor_exe))
    return suites


def main():
    parser = argparse.ArgumentParser(
        prog="python3 run_tests.py",
        description="Build and run all Ad Hoc Engine tests across all configurations.",
    )
    parser.add_argument(
        "--config",
        choices=ALL_CONFIGS,
        nargs="+",
        metavar="CONFIG",
        help="limit to specific configurations (default: all)",
    )
    parser.add_argument(
        "--build-only",
        action="store_true",
        help="build without running",
    )
    parser.add_argument(
        "--run-only",
        action="store_true",
        help="run without rebuilding (binaries must already exist)",
    )
    args = parser.parse_args()

    if args.build_only and args.run_only:
        sys.exit("--build-only and --run-only are mutually exclusive.")

    root = pathlib.Path(__file__).parent.parent
    configs = args.config or ALL_CONFIGS
    msbuild = None if args.run_only else find_msbuild()

    results = {}  # (config, project) -> {"build": bool|None, "run": bool|None}

    for config in configs:
        print(f"\n{'=' * 64}")
        print(f"  {config}")
        print(f"{'=' * 64}")

        for project, exe_stem in suites_for(config):
            key = (config, project)
            results[key] = {"build": None, "run": None}
            exe = root / "build" / project / config / (exe_stem + ".exe")

            if not args.run_only:
                ok = build(msbuild, root, config, project)
                results[key]["build"] = ok
                if not ok:
                    continue

            if not args.build_only:
                if not exe.exists():
                    print(f"  [FAIL] Executable not found: {exe}")
                    results[key]["run"] = False
                    continue
                ok = run_tests(exe, config, project)
                results[key]["run"] = ok

    print(f"\n{'=' * 64}")
    print("  SUMMARY")
    print(f"{'=' * 64}")

    failures = []
    for config in configs:
        for project, _ in suites_for(config):
            key = (config, project)
            r = results[key]
            parts = []
            if r["build"] is not None:
                parts.append(f"build {'OK' if r['build'] else 'FAIL'}")
            if r["run"] is not None:
                parts.append(f"run {'OK' if r['run'] else 'FAIL'}")
            print(f"  {config:<16}  {project:<16}  {' | '.join(parts)}")
            if r["build"] is False or r["run"] is False:
                failures.append(f"{config}/{project}")

    print(f"{'-' * 64}")
    if not failures:
        print("  All tests passed.")
    else:
        print(f"  FAILED: {', '.join(failures)}")
        sys.exit(1)


if __name__ == "__main__":
    main()
