#!/bin/bash
# This script is triggered as an Xcode scheme pre-build action.
# The working directory is the workspace folder (repository root).

engine_manifest_path="./Engine/vcpkg.json"
editor_manifest_path="./Editor/vcpkg.json"
engine_checksum_path="./Engine/vcpkg_installed/manifest_checksum.txt"
editor_checksum_path="./Editor/vcpkg_installed/manifest_checksum.txt"

engine_manifest_checksum=""
editor_manifest_checksum=""
is_engine_dependency_install_required=false
is_editor_dependency_install_required=false

check_manifests_for_changes() {
    if [ ! -f "$engine_manifest_path" ]; then
        echo "Unable to locate engine manifest at \"$engine_manifest_path\"!"
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
        fi
    fi

    if [ ! -f "$editor_manifest_path" ]; then
        echo "Unable to locate editor manifest at \"$editor_manifest_path\"!"
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
        fi
    fi
}

write_checksums_to_file() {
    if ! echo "$engine_manifest_checksum" >"$engine_checksum_path"; then
        return 1
    fi
    if ! echo "$editor_manifest_checksum" >"$editor_checksum_path"; then
        return 1
    fi
}

lipo_directory_recursive() {
    local source_dir="$1"
    local target_dir="$2"
    local output_dir="$3"

    echo "Searching \"$source_dir\" for binaries to merge..."

    local failure_detected=false

    for source_entry in "$source_dir"/*; do
        if [ -d "$source_entry" ]; then
            subdirectory_name=$(basename "$source_entry")
            if ! lipo_directory_recursive "$source_entry" "$target_dir/$subdirectory_name" "$output_dir/$subdirectory_name"; then
                echo "Failure merging binaries in \"$source_entry\"!"
                failure_detected=true
            fi

        elif [ -f "$source_entry" ]; then
            source_file_relative_path="${source_entry#$source_dir}"
            target_entry="$target_dir$source_file_relative_path"

            if [ -f "$target_entry" ]; then
                file_name=$(basename "$source_entry")
                file_extension="${file_name##*.}"

                mkdir -p "$output_dir"
                output_file="$output_dir/$(basename "$source_entry")"

                if [ "$file_extension" = "a" ] || [ "$file_extension" = "dylib" ]; then
                    echo "Creating \"$output_file\" from x64 and arm64 binaries..."
                    if ! lipo -create "$source_entry" "$target_entry" -output "$output_file"; then
                        echo "Failed to merge $file_name!"
                        failure_detected=true
                    fi
                else
                    echo "Copying non-binary file \"$source_entry\"..."
                    if ! cp "$source_entry" "$output_file"; then
                        echo "Failed to copy \"$file_name\"!"
                        failure_detected=true
                    fi
                fi
            fi
        fi
    done

    if [ "$failure_detected" == true ]; then
        echo "One or more errors occurred during lipo step"
        return 1
    fi
}

install_dependencies_for_project_and_linkage() {
    local project_dir=$1
    local linkage_flag=$2

    echo "Installing dependencies for $project_dir ($linkage_flag linkage)..."

    vcpkg="./vcpkg/vcpkg"

    echo "Installing for x64..."
    if ! "$vcpkg" install \
        --no-print-usage \
        --overlay-triplets="./triplets" \
        --triplet=x64-osx-11_5-$linkage_flag \
        --x-manifest-root="$project_dir" \
        --x-install-root="$project_dir/vcpkg_installed/x64-$linkage_flag"; then
        echo "Error running vcpkg on triplet x64-osx-11_5-$linkage_flag!"
        return 1
    fi

    echo "Installing for arm64..."
    if ! "$vcpkg" install \
        --no-print-usage \
        --overlay-triplets="./triplets" \
        --triplet=arm64-osx-11_5-$linkage_flag \
        --x-manifest-root="$project_dir" \
        --x-install-root="$project_dir/vcpkg_installed/arm64-$linkage_flag"; then
        echo "Error running vcpkg on triplet arm64-osx-11_5-$linkage_flag!"
        return 1
    fi

    echo "Merging into universal binaries using lipo tool..."
    if ! lipo_directory_recursive \
        "$project_dir/vcpkg_installed/x64-$linkage_flag/x64-osx-11_5-$linkage_flag" \
        "$project_dir/vcpkg_installed/arm64-$linkage_flag/arm64-osx-11_5-$linkage_flag" \
        "$project_dir/vcpkg_installed/uni-$linkage_flag"; then
        echo "Error during dependency merging!"
        return 1
    fi
}

# -----------------------------------------------------------------------------

echo "Running vcpkg install step..."

if [ ! -f ./vcpkg/bootstrap-vcpkg.sh ]; then
    echo "bootstrap-vcpkg.sh not found! Initializing Git submodules..."
    if ! git submodule update --init --recursive; then
        echo "Failure during git submodule update!"
        exit 1
    fi
fi

if [ ! -f ./vcpkg/vcpkg ]; then
    echo "vcpkg not found! Bootstrapping vcpkg..."
    chmod +x ./vcpkg/bootstrap-vcpkg.sh
    if ! sh ./vcpkg/bootstrap-vcpkg.sh; then
        echo "vcpkg bootstrapping failed!"
        exit 1
    fi
fi

if ! check_manifests_for_changes; then
    echo "Failure while checking vcpkg manifest checksums!"
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

if [ "$install_failed" == true ]; then
    echo "One or more vcpkg install steps failed"
    exit 1
fi

echo "Installation steps completed without issue. Writing manifest checksums to file..."
if ! write_checksums_to_file; then
    echo "Failed to write manifest checksums to disk! Next build may redundantly reinstall dependencies"
fi

echo "Successfully completed vcpkg install step!"
exit 0
