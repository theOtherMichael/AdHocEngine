#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

PROJECTS=(Engine EngineTests Editor EditorTests Launcher)

for project in "${PROJECTS[@]}"; do
    dir="$REPO_ROOT/$project/vcpkg_installed"
    if [ -d "$dir" ]; then
        echo "Removing $dir"
        rm -rf "$dir"
    else
        echo "Skipping $dir (not found)"
    fi
done
