#!/bin/bash

# Check if the user provided at least one argument (source file)
if [ $# -lt 1 ]; then
    echo "Usage: $0 <source_file1> [<source_file2> ...]"
    exit 1
fi

# Collect all source files
SOURCE_FILES=""
for arg in "$@"; do
    SOURCE_FILES="$SOURCE_FILES $arg.cpp"
done

OUTPUT_FILE="output"

# Base compile command
COMPILE_CMD="g++ -g $SOURCE_FILES -o $OUTPUT_FILE -std=c++23 -fsanitize=address -lssl -lcrypto"

# Compile the source files
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
