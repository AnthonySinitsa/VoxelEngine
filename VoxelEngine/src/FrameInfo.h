#pragma once

#include "Camera/Camera.h"
#include "Game/GameObject.h"

// lib
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace vge {

    #define MAX_LIGHTS 10

    struct PointLight {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{}; // w is intensity
    };

    // Can pass as many fields into this struct
    struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
} // namespace
