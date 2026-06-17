#!/usr/bin/env python3
"""Reset all CMake Tools state for this workspace to a clean slate.

CMake Tools persists its workspace state (selected preset, build/test/launch
targets, cached presets, variant settings) in VS Code's global SQLite state
database. When any of it gets stuck or out of sync and can't be fixed through
the UI, this script removes every stored field for this workspace so the next
configure starts from scratch.

It was originally written to clear a build target stuck on [all]
(vscode-cmake-tools#3587), but now wipes the whole workspace's state to address
any such problem.

VS Code must be closed before running this script.
"""
from __future__ import annotations

import json
import platform
import sqlite3
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent


def get_db_path() -> Path | None:
    system = platform.system()
    if system == "Darwin":
        return Path.home() / "Library/Application Support/Code/User/globalStorage/state.vscdb"
    if system == "Windows":
        return Path.home() / "AppData/Roaming/Code/User/globalStorage/state.vscdb"
    return None


def vscode_running() -> bool:
    """Best-effort check for a running VS Code instance.

    Returns False if the probe can't run; callers treat that as "unknown"
    and proceed, so we never block an operation just because detection failed.
    """
    system = platform.system()
    try:
        if system == "Windows":
            out = subprocess.run(
                ["tasklist", "/FI", "IMAGENAME eq Code.exe", "/NH"],
                capture_output=True, text=True, check=False,
            ).stdout
            return "Code.exe" in out
        return subprocess.run(
            ["pgrep", "-fi", "Visual Studio Code"],
            capture_output=True, check=False,
        ).returncode == 0
    except (OSError, subprocess.SubprocessError):
        return False


def reset_workspace_cmake_state() -> None:
    """Drop every CMake Tools field VS Code persists for this workspace.

    CMake Tools stores all workspace state as a single JSON blob keyed by
    'ms-vscode.cmake-tools' in VS Code's global SQLite database. Each per-
    workspace field has the workspace path prepended to its name, e.g.
    '/path/to/repoconfigurePresetName'. Removing every field prefixed with the
    repo path clears the remembered preset, build/test/launch targets, cached
    presets, and variant settings so the next configure starts from scratch.

    VS Code should be closed first, or it may rewrite the blob on exit.
    """
    label = "VS Code CMake Tools state"
    db_path = get_db_path()
    if db_path is None:
        print(f"Skipping  {label}  (unsupported platform)")
        return
    if not db_path.exists():
        print(f"Skipping  {label}  (state database not found)")
        return
    if vscode_running():
        print(
            f"Skipping  {label}  (VS Code is running)\n"
            "          Close VS Code and rerun to reset the preset/targets — "
            "otherwise VS Code rewrites the state on exit."
        )
        return

    prefix = str(REPO_ROOT)
    con = sqlite3.connect(db_path)
    try:
        cur = con.execute("SELECT value FROM ItemTable WHERE key = 'ms-vscode.cmake-tools'")
        row = cur.fetchone()
        if row is None:
            print(f"Skipping  {label}  (no stored state)")
            return

        data = json.loads(row[0])
        stale = [field for field in data if field.startswith(prefix)]
        if not stale:
            print(f"Skipping  {label}  (nothing for this workspace)")
            return

        for field in stale:
            del data[field]
        con.execute(
            "UPDATE ItemTable SET value = ? WHERE key = 'ms-vscode.cmake-tools'",
            (json.dumps(data),),
        )
        con.commit()
        print(f"Removing  {label}  ({len(stale)} workspace fields cleared)")
    finally:
        con.close()


def main() -> None:
    reset_workspace_cmake_state()


if __name__ == "__main__":
    main()
