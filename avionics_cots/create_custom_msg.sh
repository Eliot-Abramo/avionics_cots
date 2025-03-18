#!/bin/bash

# Set the source file path.
SOURCE="lib/Packets/generate_structs.cpp"

# Set the output executable name.
EXE="generate_structs"

# Compile the source file with C++17.
echo "Compiling $SOURCE..."
g++ -std=c++17 -o "$EXE" "$SOURCE"
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Define input folder and output file for the executable.
INPUT_FOLDER="msg/not_hd/"
OUTPUT_FILE="all_structs.generated.hpp"

# Execute the compiled program.
echo "Running $EXE with input folder '$INPUT_FOLDER' and output file '$OUTPUT_FILE'..."
./"$EXE" "$INPUT_FOLDER" "$OUTPUT_FILE"
