#include "Input.h"

// std
#include <chrono>
#include <glm/gtc/constants.hpp>
#include <limits>
#include <GLFW/glfw3.h>

namespace vge{

    void Input::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject & gameObject){
        glm::vec3 rotate{0};
        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS)rotate.y += 1.f;
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)rotate.y -= 1.f;
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS)rotate.x += 1.f;
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS)rotate.x -= 1.f;

        // Apply mouse rotation
        rotate.x += mouseDelta.y;
        rotate.y += mouseDelta.x;

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

        // Reset mouse delta
        mouseDelta = glm::vec2{0.0f};
    }


    void Input::mousePosCallback(GLFWwindow* window, double xpos, double ypos){
        auto input = reinterpret_cast<Input*>(glfwGetWindowUserPointer(window));
        input->mouseCallback(xpos, ypos);
    }


    void Input::mouseCallback(double xpos, double ypos){
        auto now = std::chrono::steady_clock::now();

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            lastMouseMoveTime = now;
            firstMouse = false;
            return;
        }

        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top

        // Calculate time since last mousem ovement
        auto timeDelta =
            std::chrono::duration_cast<std::chrono::duration<float>>(now - lastMouseMoveTime).count();

        // Calculate mouse speed
        float mouseSpeed =
            static_cast<float>(glm::length(glm::vec2(xoffset, yoffset))) / (timeDelta + 0.0001f); // Avoid division by zero

        // Apply mouse sensitivity as a multiplier
        float adjustedSensitivity = mouseSensitivity * mouseSpeed;

        mouseDelta.x += static_cast<float>(xoffset) * adjustedSensitivity;
        mouseDelta.y += static_cast<float>(xoffset) * adjustedSensitivity;

        lastX = xpos;
        lastY = ypos;
        lastMouseMoveTime = now;
    }
} // namespace
