#!/bin/bash
# This script is triggered as an Xcode scheme pre-build action.
# The working directory is the workspace folder (repository root).

engine_manifest_path="./Engine/vcpkg.json"
editor_manifest_path="./Editor/vcpkg.json"
engine_tests_manifest_path="./EngineTests/vcpkg.json"
editor_tests_manifest_path="./EditorTests/vcpkg.json"

engine_checksum_path="./Engine/vcpkg_installed/manifest_checksum.txt"
editor_checksum_path="./Editor/vcpkg_installed/manifest_checksum.txt"
engine_tests_checksum_path="./EngineTests/vcpkg_installed/manifest_checksum.txt"
editor_tests_checksum_path="./EditorTests/vcpkg_installed/manifest_checksum.txt"

engine_manifest_checksum=""
editor_manifest_checksum=""
engine_tests_manifest_checksum=""
editor_tests_manifest_checksum=""

is_engine_dependency_install_required=false
is_editor_dependency_install_required=false
is_engine_tests_dependency_install_required=false
is_editor_tests_dependency_install_required=false

host_arch=$(uname -m)
case "$host_arch" in
    arm64)  vcpkg_arch="arm64" ;;
    *)
        echo "Unsupported host architecture: $host_arch"
        exit 1
        ;;
esac

check_manifests_for_changes() {
    if [ ! -f "$engine_manifest_path" ]; then
        echo "Engine manifest not found at \"$engine_manifest_path\""
        return 1
    else
        echo "Checking \"$engine_manifest_path\" for changes..."
        engine_manifest_checksum=$(shasum $engine_manifest_path | awk '{ print $1 }')
        echo "Engine manifest checksum: $engine_manifest_checksum"

        if [ -f $engine_checksum_path ]; then
            previous_engine_checksum=$(cat $engine_checksum_path)
            if [ "$engine_manifest_checksum" = "$previous_engine_checksum" ]; then
                echo "No changes to Engine/vcpkg.json detected since last build. Dependencies will not be reinstalled"
            else
                echo "Detected changes to Engine/vcpkg.json. Engine dependencies will be reinstalled"
                is_engine_dependency_install_required=true
            fi
        else
            echo "Engine checksum file \"$engine_checksum_path\" was not found. Engine dependencies will be installed"
            is_engine_dependency_install_required=true
        fi
    fi

    if [ ! -f "$editor_manifest_path" ]; then
        echo "Editor manifest not found at \"$editor_manifest_path\""
        return 1
    else
        echo "Checking Editor/vcpkg.json for changes..."
        editor_manifest_checksum=$(shasum $editor_manifest_path | awk '{ print $1 }')
        echo "Editor manifest checksum: $editor_manifest_checksum"

        if [ -f $editor_checksum_path ]; then
            previous_editor_checksum=$(cat $editor_checksum_path)
            if [ "$editor_manifest_checksum" = "$previous_editor_checksum" ]; then
                echo "No changes to Editor/vcpkg.json detected since last build. Dependencies will not be reinstalled"
            else
                echo "Detected changes to Editor/vcpkg.json. Editor dependencies will be reinstalled"
                is_editor_dependency_install_required=true
            fi
        else
            echo "Editor checksum file \"$editor_checksum_path\" was not found. Editor dependencies will be installed"
            is_editor_dependency_install_required=true
        fi
    fi

    if [ ! -f "$engine_tests_manifest_path" ]; then
        echo "EngineTests manifest not found at \"$engine_tests_manifest_path\""
        return 1
    else
        echo "Checking EngineTests/vcpkg.json for changes..."
        engine_tests_manifest_checksum=$(shasum $engine_tests_manifest_path | awk '{ print $1 }')
        echo "EngineTests manifest checksum: $engine_tests_manifest_checksum"

        if [ -f $engine_tests_checksum_path ]; then
            previous_engine_tests_checksum=$(cat $engine_tests_checksum_path)
            if [ "$engine_tests_manifest_checksum" = "$previous_engine_tests_checksum" ]; then
                echo "No changes to EngineTests/vcpkg.json detected since last build. Dependencies will not be reinstalled"
            else
                echo "Detected changes to EngineTests/vcpkg.json. EngineTests dependencies will be reinstalled"
                is_engine_tests_dependency_install_required=true
            fi
        else
            echo "EngineTests checksum file \"$engine_tests_checksum_path\" was not found. EngineTests dependencies will be installed"
            is_engine_tests_dependency_install_required=true
        fi
    fi

    if [ ! -f "$editor_tests_manifest_path" ]; then
        echo "EditorTests manifest not found at \"$editor_tests_manifest_path\""
        return 1
    else
        echo "Checking EditorTests/vcpkg.json for changes..."
        editor_tests_manifest_checksum=$(shasum $editor_tests_manifest_path | awk '{ print $1 }')
        echo "EditorTests manifest checksum: $editor_tests_manifest_checksum"

        if [ -f $editor_tests_checksum_path ]; then
            previous_editor_tests_checksum=$(cat $editor_tests_checksum_path)
            if [ "$editor_tests_manifest_checksum" = "$previous_editor_tests_checksum" ]; then
                echo "No changes to EditorTests/vcpkg.json detected since last build. Dependencies will not be reinstalled"
            else
                echo "Detected changes to EditorTests/vcpkg.json. EditorTests dependencies will be reinstalled"
                is_editor_tests_dependency_install_required=true
            fi
        else
            echo "EditorTests checksum file \"$editor_tests_checksum_path\" was not found. EditorTests dependencies will be installed"
            is_editor_tests_dependency_install_required=true
        fi
    fi
}

install_dependencies_for_project_and_linkage() {
    local project_dir=$1
    local linkage_flag=$2

    local triplet="${vcpkg_arch}-osx-${linkage_flag}-adhoc"

    echo "Installing dependencies for $project_dir ($triplet)..."

    vcpkg="./vcpkg/vcpkg"

    if ! "$vcpkg" install \
        --no-print-usage \
        --overlay-triplets="./triplets" \
        --triplet="$triplet" \
        --x-manifest-root="$project_dir" \
        --x-install-root="$project_dir/vcpkg_installed/${linkage_flag}"; then
        echo "vcpkg returned an error on triplet $triplet"
        return 1
    fi
}

# -----------------------------------------------------------------------------

echo "Running vcpkg install step..."

if [ ! -f ./vcpkg/bootstrap-vcpkg.sh ]; then
    echo "bootstrap-vcpkg.sh not found. Initializing Git submodules..."
    if ! git submodule update --init --recursive; then
        echo "Git submodule update returned an error."
        exit 1
    fi
fi

vcpkg_commit=$(git rev-parse HEAD:./vcpkg 2>/dev/null)
bootstrap_commit_path="./vcpkg/bootstrap_commit.txt"

needs_bootstrap=false
if [ ! -f ./vcpkg/vcpkg ]; then
    needs_bootstrap=true
elif [ -n "$vcpkg_commit" ]; then
    if [ ! -f "$bootstrap_commit_path" ]; then
        needs_bootstrap=true
    else
        bootstrapped_commit=$(cat "$bootstrap_commit_path")
        if [ "$vcpkg_commit" != "$bootstrapped_commit" ]; then
            echo "vcpkg submodule has changed since last bootstrap. Re-bootstrapping..."
            needs_bootstrap=true
        fi
    fi
fi

if [ "$needs_bootstrap" == true ]; then
    echo "Bootstrapping vcpkg..."
    chmod +x ./vcpkg/bootstrap-vcpkg.sh
    if ! sh ./vcpkg/bootstrap-vcpkg.sh; then
        echo "vcpkg bootstrapping returned an error."
        exit 1
    fi
    if [ -n "$vcpkg_commit" ]; then
        echo "$vcpkg_commit" > "$bootstrap_commit_path"
    fi
    echo "vcpkg bootstrapped successfully"
fi

if ! check_manifests_for_changes; then
    echo "vcpkg manifest checksums could not be verified."
    exit 1
fi

install_failed=false

if [ "$is_engine_dependency_install_required" == true ]; then
    if ! install_dependencies_for_project_and_linkage ./Engine dynamic; then
        install_failed=true
    fi
    if ! install_dependencies_for_project_and_linkage ./Engine static; then
        install_failed=true
    fi
else
    echo "Engine dependencies are already installed, skipping"
fi

if [ "$is_editor_dependency_install_required" == true ]; then
    if ! install_dependencies_for_project_and_linkage ./Editor dynamic; then
        install_failed=true
    fi
else
    echo "Editor dependencies are already installed, skipping"
fi

if [ "$is_engine_tests_dependency_install_required" == true ]; then
    if ! install_dependencies_for_project_and_linkage ./EngineTests dynamic; then
        install_failed=true
    fi
    if ! install_dependencies_for_project_and_linkage ./EngineTests static; then
        install_failed=true
    fi
else
    echo "EngineTests dependencies are already installed, skipping"
fi

if [ "$is_editor_tests_dependency_install_required" == true ]; then
    if ! install_dependencies_for_project_and_linkage ./EditorTests dynamic; then
        install_failed=true
    fi
else
    echo "EditorTests dependencies are already installed, skipping"
fi

if [ "$install_failed" == true ]; then
    echo "One or more vcpkg install steps failed."
    exit 1
fi

echo "Installation steps completed without issue. Writing manifest checksums to file..."

echo "$engine_manifest_checksum" >"$engine_checksum_path"
engineChecksumFileContents=$(cat $engine_checksum_path)
if [ "$engineChecksumFileContents" = "$engine_checksum_path" ]; then
    echo
    echo "Engine manifest checksum could not be written to disk. Next build may redundantly reinstall dependencies."
fi

echo "$editor_manifest_checksum" >"$editor_checksum_path"
editorChecksumFileContents=$(cat $editor_checksum_path)
if [ "$editorChecksumFileContents" = "$editor_checksum_path" ]; then
    echo "Editor manifest checksum could not be written to disk. Next build may redundantly reinstall dependencies."
fi

echo "$engine_tests_manifest_checksum" >"$engine_tests_checksum_path"
engineTestsChecksumFileContents=$(cat $engine_tests_checksum_path)
if [ "$engineTestsChecksumFileContents" = "$engine_tests_checksum_path" ]; then
    echo
    echo "EngineTests manifest checksum could not be written to disk. Next build may redundantly reinstall dependencies."
fi

echo "$editor_tests_manifest_checksum" >"$editor_tests_checksum_path"
editorTestsChecksumFileContents=$(cat $editor_tests_checksum_path)
if [ "$editorTestsChecksumFileContents" = "$editor_tests_checksum_path" ]; then
    echo "EditorTests manifest checksum could not be written to disk. Next build may redundantly reinstall dependencies."
fi

echo "Successfully completed vcpkg install step"
exit 0
