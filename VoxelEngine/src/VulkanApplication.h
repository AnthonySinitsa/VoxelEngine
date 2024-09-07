#pragma once

#include "Window.h"
#include "Graphics/Pipeline.h"
#include "Device/Device.h"
#include "Presentation/SwapChain.h"

// std
#include <memory>
#include <vector>

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
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();

            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
            VgeDevice vgeDevice{vgeWindow};
            VgeSwapChain vgeSwapChain{vgeDevice, vgeWindow.getExtent()};
            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
    };
} // namespace
