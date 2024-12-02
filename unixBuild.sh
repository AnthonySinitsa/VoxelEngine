#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit

# Run cmake
echo "Running CMake..."
if ! cmake -S ../ -B .; then
    echo "CMake configuration failed"
    exit 1
fi

# Build project and shaders
echo "Building project..."
if ! make; then
    echo "Build failed"
    exit 1
fi

echo "Building shaders..."
if ! make Shaders; then
    echo "Shader compilation failed"
    exit 1
fi

# Return to project root before running
cd ..

# Run the executable from the VoxelEngine directory
cd VoxelEngine || exit
echo "Running VgeEngine..."
../build/VgeEngine

# Return to original directory
cd ..
