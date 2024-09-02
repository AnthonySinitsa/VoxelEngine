#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vge {
    class Window{

        public:
            Window(int w, int h, std::string name);
            ~Window();

        private:

            void initWindow();

            const int width;
            const int height;

            std::string windowName;
            GLFWwindow *window;
    };
} // namespace
