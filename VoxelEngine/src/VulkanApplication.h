#pragma once

#include "Window.h"

namespace vge {
    class VulkanApplication {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();

        private:
            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
    };
} // namespace
