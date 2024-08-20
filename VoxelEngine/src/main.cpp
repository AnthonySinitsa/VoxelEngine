#include "window.h"

#include <iostream>

int main() {
    try {
        Window window(800, 600, "AAAAAAAAAAAA");

        while (!window.shouldClose()) {
            // Render your Voxel Engine here

            window.swapBuffers();
            window.pollEvents();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}