#pragma once

#include "../Game/GameObject.h"
#include "../Window.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <chrono>

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
            };

            Input(GLFWwindow* window){
                glfwSetWindowUserPointer(window, this);
                glfwSetCursorPosCallback(window, mousePosCallback);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            bool isEscapePressed(GLFWwindow* window){
                return glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
            }

            void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);

            KeyMappings keys{};
            float moveSpeed{3.f};
            float lookSpeed{0.002f};
            float mouseSensitivity{1.0f}; // Acts as a multiplier

        private:
            static void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
            void mouseCallback(double xpos, double ypos);

            glm::vec2 mouseDelta{0.f};
            bool firstMouse = true;
            float lastX = 0.f;
            float lastY = 0.f;
            std::chrono::steady_clock::time_point lastMouseMoveTime;
    };
} // namespace
