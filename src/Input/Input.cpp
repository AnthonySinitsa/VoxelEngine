#include "Input.h"

// std
#include <glm/gtc/constants.hpp>
#include <GLFW/glfw3.h>

namespace vge{

    void Input::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject & gameObject){
        // Only process input if window is focused
        if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)) return;

        glm::vec3 rotate{0};
        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS)rotate.y += 1.f;
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)rotate.y -= 1.f;
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS)rotate.x += 1.f;
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS)rotate.x -= 1.f;

        if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()){
            gameObject.transform.rotation  += lookSpeed * dt * glm::normalize(rotate);
        }

        // this limits pitch values between +/- 85 degress
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS)moveDir += forwardDir;
        if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)moveDir -= forwardDir;
        if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS)moveDir += rightDir;
        if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)moveDir -= rightDir;
        if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS)moveDir += upDir;
        if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS)moveDir -= upDir;

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
            gameObject.transform.translation  += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

    void Input::toggleMouseLock(GLFWwindow* window) {
        mouseLocked = !mouseLocked;
        if (mouseLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // Reset first mouse move flag when re-locking mouse
            firstMouseMove = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void Input::update(GLFWwindow* window) {
        // Check if window has focus
        bool windowFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED);

        // If window loses focus, automatically unlock mouse
        if (!windowFocused && mouseLocked) {
            toggleMouseLock(window);
            return;
        }

        // Only process unlock key if window is focused
        if (windowFocused) {
            int state = glfwGetKey(window, keys.unlockMouse);
            if(state == GLFW_PRESS){
                if(!unlockKeyPressed){
                    toggleMouseLock(window);
                    unlockKeyPressed = true;
                }
            } else if (state == GLFW_RELEASE){
                unlockKeyPressed = false;
            }
        }
    }

    void Input::mouseMove(GLFWwindow* window, GameObject& gameObject) {
        // Check both mouse lock and window focus
        if (!mouseLocked || !glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
            return;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (firstMouseMove) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouseMove = false;
            return;  // Skip the first frame to avoid sudden jumps
        }

        float deltaX = static_cast<float>(mouseX - lastMouseX);
        float deltaY = static_cast<float>(mouseY - lastMouseY);

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        const float mouseSensitivity = 0.001f;
        gameObject.transform.rotation.y += deltaX * mouseSensitivity;
        gameObject.transform.rotation.x -= deltaY * mouseSensitivity;

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    }
} // namespace
