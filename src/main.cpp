#include <cstdlib>
#include <exception>
#include <iostream>

#include "VulkanApplication.h"

int main() {
    vge::VulkanApplication app{};

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
