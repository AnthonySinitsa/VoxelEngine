#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;

    GLFWwindow* getHandle() const { return m_window; }

private:
    GLFWwindow* m_window;
};