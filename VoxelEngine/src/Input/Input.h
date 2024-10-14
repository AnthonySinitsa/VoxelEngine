#pragma once

#include "../Game/GameObject.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace vge{

    class Input{
        public:
            struct KeyMappings{
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_SPACE;
                int moveDown = GLFW_KEY_LEFT_SHIFT;
                int lookLeft = GLFW_KEY_LEFT;
                int lookRight = GLFW_KEY_RIGHT;
                int lookUp = GLFW_KEY_UP;
                int lookDown = GLFW_KEY_DOWN;
                int unlockMouse = GLFW_KEY_E;
            };

            bool isEscapePressed(GLFWwindow* window){
                return glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
            }

            void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);
            void mouseMove(GLFWwindow* window, GameObject& gameObject);

            // Function to toggle mouse lock state
            void toggleMouseLock(GLFWwindow* window);
            void update(GLFWwindow* window);
            bool isMouseLocked() const { return mouseLocked; }

            KeyMappings keys{};
            float moveSpeed{3.f};
            float lookSpeed{1.5f};
            float mouseSensitivity{0.1f};

        private:
            // Variables to track previous mouse position
            double lastMouseX = 0.0;
            double lastMouseY = 0.0;
            bool firstMouseMove = true; // Helps initialize last position
            bool mouseLocked = true; // Start with mouse locked
            bool unlockKeyPressed = false;
    };
} // namespace
