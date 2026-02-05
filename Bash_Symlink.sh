#!/bin/bash

# Default to empty string if no argument provided
SINGLE_FOLDER="${1:-}"

SML_PATH="/home/$USER/SF_Modding/SatisfactoryModLoader/Mods"
MOD_DIR="/home/$USER/SF_Modding/SatisfactoryMods"

# Function to create symbolic link
new_sym_link() {
    local link_name="$1"
    local link_path="$SML_PATH/$link_name"
    local target_path="$MOD_DIR/$link_name"

    # Remove existing symlink or directory if it exists
    if [ -e "$link_path" ] || [ -L "$link_path" ]; then
        rm -rf "$link_path"
    fi

    # Create new symbolic link
    ln -sf "$target_path" "$link_path"
}

# Check if a specific folder was specified
if [ -n "$SINGLE_FOLDER" ]; then
    # Check if the specified folder exists and is a directory
    if [ -d "$MOD_DIR/$SINGLE_FOLDER" ]; then
        echo "Processing folder: $SINGLE_FOLDER"
        new_sym_link "$SINGLE_FOLDER"
    else
        echo "Folder '$SINGLE_FOLDER' not found."
    fi
else
    # Process all directories in MOD_DIR
    if [ -d "$MOD_DIR" ]; then
        for dir in "$MOD_DIR"/*; do
            if [ -d "$dir" ]; then
                dir_name=$(basename "$dir")
                echo "Processing folder: $dir_name"
                new_sym_link "$dir_name"
            fi
        done
    else
        echo "Mod directory not found: $MOD_DIR"
    fi
fi
