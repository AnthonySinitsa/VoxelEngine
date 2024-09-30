#pragma once

#include "Camera/Camera.h"

// lib
#include <vulkan/vulkan.h>

namespace vge {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
    };
} // namespace
