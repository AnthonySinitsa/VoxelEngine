#pragma once

#include "Window.h"
#include "Graphics/Pipeline.h"
#include "Device/Device.h"
#include "Game/GameObject.h"
#include "Rendering/Renderer.h"

// std
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vge {
    class VulkanApplication {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            VulkanApplication();
            ~VulkanApplication();

            VulkanApplication(const VulkanApplication &) = delete;
            VulkanApplication &operator=(const VulkanApplication &) = delete;

            void run();

        private:
            void loadGameObjects();
            void createPipelineLayout();
            void createPipeline();
            void renderGameObjects(VkCommandBuffer commandBuffer);

            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
            VgeDevice vgeDevice{vgeWindow};
            Renderer vgeRenderer{vgeWindow, vgeDevice};

            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<GameObject> gameObjects;
    };
} // namespace
