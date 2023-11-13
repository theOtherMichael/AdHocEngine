#!/bin/bash

cwd_folder_name=$(basename $PWD)

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

echo "Installing $cwd_folder_name dependencies..."
vcpkg="../vcpkg/vcpkg"

echo "Running vcpkg install for static triplets..."
"$vcpkg" install --no-print-usage --triplet=x64-osx --x-install-root=vcpkg_installed/x64-static
"$vcpkg" install --no-print-usage --triplet=arm64-osx --x-install-root=vcpkg_installed/arm64-static

echo "Merging into universal binaries..."
lipo_directory_recursive vcpkg_installed/x64-static/x64-osx  vcpkg_installed/arm64-static/arm64-osx  vcpkg_installed/uni-static

echo "Running vcpkg install for dynamic triplets..."
"$vcpkg" install --no-print-usage --triplet=x64-osx-dynamic   --x-install-root=vcpkg_installed/x64-dynamic
"$vcpkg" install --no-print-usage --triplet=arm64-osx-dynamic --x-install-root=vcpkg_installed/arm64-dynamic

echo "Merging into universal binaries..."
lipo_directory_recursive vcpkg_installed/x64-dynamic/x64-osx-dynamic vcpkg_installed/arm64-dynamic/arm64-osx-dynamic vcpkg_installed/uni-dynamic

touch ./vcpkg_installed/empty.txt

echo "$cwd_folder_name dependencies installed!"
