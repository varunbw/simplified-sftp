#!/bin/bash

# Check if the user provided an argument (source file)
if [ -z "$1" ]; then
    echo "Usage: $0 <source_file>"
    exit 1
fi

SOURCE_FILE="$1.cpp"
OUTPUT_FILE="$1"

# Base compile command
COMPILE_CMD="g++ -g $SOURCE_FILE -o $OUTPUT_FILE -std=c++23 -fsanitize=address -lssl -lcrypto"

# Compile the source file
echo "Compiling with command: $COMPILE_CMD"
$COMPILE_CMD

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

echo "Compilation successful"
echo "--------------------------------------------------------------------------"
./$OUTPUT_FILE "$@"
