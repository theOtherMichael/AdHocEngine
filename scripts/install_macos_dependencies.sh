#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments."
    echo "Usage: $0 (dynamic|static)"
    exit 1
fi

linkage_flag=$1
if [[ "$linkage_flag" != "dynamic" && "$linkage_flag" != "static" ]]; then
    echo "Invalid link type."
    echo "Usage: $0 (dynamic|static)"
    exit 1
fi

if [ ! -f ../vcpkg/bootstrap-vcpkg.sh ]; then
    echo "Initializing Git submodules..."
    git submodule update --init --recursive
fi

if [ ! -f ../vcpkg/vcpkg ]; then
    echo "Bootstrapping vcpkg..."
    chmod +x ../vcpkg/bootstrap-vcpkg.sh
    sh ../vcpkg/bootstrap-vcpkg.sh
fi

lipo_directory_recursive() {
    local source_dir="$1"
    local target_dir="$2"
    local output_dir="$3"

    for source_entry in "$source_dir"/*; do
        if [ -d "$source_entry" ]; then
            subdirectory_name=$(basename "$source_entry")
            lipo_directory_recursive "$source_entry" "$target_dir/$subdirectory_name" "$output_dir/$subdirectory_name"

        elif [ -f "$source_entry" ]; then
            source_file_relative_path="${source_entry#$source_dir}"
            target_entry="$target_dir$source_file_relative_path"

            if [ -f "$target_entry" ]; then
                file_name=$(basename "$source_entry")
                file_extension="${file_name##*.}"

                mkdir -p "$output_dir"
                output_file="$output_dir/$(basename "$source_entry")"

                if [ "$file_extension" == "a" ] || [ "$file_extension" == "dylib" ]; then
                    lipo -create "$source_entry" "$target_entry" -output "$output_file"
                else
                    cp "$source_entry" "$output_file"
                fi
            fi
        fi
    done
}

cwd_folder_name=$(basename $PWD)

echo "Installing $cwd_folder_name $linkage_flag dependencies..."

vcpkg="../vcpkg/vcpkg"
"$vcpkg" install \
    --no-print-usage \
    --overlay-triplets=../triplets \
    --triplet=x64-osx-11_5-$linkage_flag \
    --x-install-root=vcpkg_installed/x64-$linkage_flag
"$vcpkg" install \
    --no-print-usage \
    --overlay-triplets=../triplets \
    --triplet=arm64-osx-11_5-$linkage_flag \
    --x-install-root=vcpkg_installed/arm64-$linkage_flag

lipo_directory_recursive \
    vcpkg_installed/x64-$linkage_flag/x64-osx-11_5-$linkage_flag \
    vcpkg_installed/arm64-$linkage_flag/arm64-osx-11_5-$linkage_flag \
    vcpkg_installed/uni-$linkage_flag

touch ./vcpkg_installed/uni-$linkage_flag/empty.txt
