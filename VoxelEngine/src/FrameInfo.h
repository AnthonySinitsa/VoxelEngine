#pragma once

#include "Camera/Camera.h"
#include "Game/GameObject.h"

// lib
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace vge {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
        GameObject::Map &gameObjects;
    };
} // namespace
