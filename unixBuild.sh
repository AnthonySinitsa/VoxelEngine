#!/bin/bash

mkdir -p build
cd build || exit

# Run cmake
echo "Running CMake..."
if ! cmake -S ../ -B . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; then
    echo "CMake configuration failed"
    exit 1
fi

# Build project
echo "Building project (including shaders)..."
if ! make; then
    echo "Build failed"
    exit 1
fi

# Run the executable from the project root
cd ..
echo "Running VgeEngine..."
./build/VgeEngine
