#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit

# Run cmake with the compatibility flag for older dependencies
echo "Running CMake..."
if ! cmake -S ../ -B . -DCMAKE_POLICY_VERSION_MINIMUM=3.5; then
    echo "CMake configuration failed"
    exit 1
fi

# Build project and shaders
echo "Building project..."
if ! make; then
    echo "Build failed"
    exit 1
fi

# No need for "make Shaders" if added add_dependencies in CMakeLists.txt
echo "Building shaders..."
if ! make Shaders; then
    echo "Shader compilation failed"
    exit 1
fi

# Run the executable from the project root
cd ..
echo "Running VgeEngine..."
./build/VgeEngine
