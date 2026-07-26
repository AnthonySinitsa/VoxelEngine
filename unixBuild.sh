#!/bin/bash

# --- Dependency Checker ---
install_dependencies() {
    echo "Checking for Vulkan dependencies..."

    # We check for vulkan.h (headers) and glslangValidator (shader compiler)
    if [ ! -f "/usr/include/vulkan/vulkan.h" ] || ! command -v glslangValidator &>/dev/null; then
        echo "Missing Vulkan development packages or Shader Compiler!"
        read -p "Would you like to install them now? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            if command -v pacman &>/dev/null; then
                # Arch Linux / Manjaro
                sudo pacman -S --needed vulkan-headers vulkan-icd-loader vulkan-validation-layers glslang
            elif command -v apt-get &>/dev/null; then
                # Ubuntu / Debian / Pop!_OS
                sudo apt-get update
                sudo apt-get install -y libvulkan-dev vulkan-validationlayers glslang-tools
            elif command -v dnf &>/dev/null; then
                # Fedora
                sudo dnf install -y vulkan-headers vulkan-loader-devel vulkan-validation-layers glslang
            else
                echo "Could not detect package manager. Please install Vulkan manually."
                exit 1
            fi
        else
            echo "Cannot build engine without Vulkan. Exiting."
            exit 1
        fi
    else
        echo "All Vulkan dependencies found!"
    fi
}

# Run the dependency check
install_dependencies

# --- Original Build Script ---
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
