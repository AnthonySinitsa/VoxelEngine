#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vge {
class Window {
public:
    Window(int w, int h, std::string name, bool fullscreen = false);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() {
        return glfwWindowShouldClose(window);
    }
    VkExtent2D getExtent() {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
    bool wasWindowResized() {
        return frameBufferResized;
    }
    void resetWindowResizedFlag() {
        frameBufferResized = false;
    }
    GLFWwindow* getGLFWwindow() const {
        return window;
    }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    void toggleFullscreen();

private:
    static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

    void initWindow();

    int width;
    int height;
    bool frameBufferResized = false;
    bool isFullscreen = false;

    std::string windowName;
    GLFWwindow* window;
};
}  // namespace vge
