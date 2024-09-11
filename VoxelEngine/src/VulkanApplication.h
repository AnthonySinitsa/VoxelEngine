#pragma once

#include "Window.h"
#include "Graphics/Pipeline.h"
#include "Device/Device.h"
#include "Presentation/SwapChain.h"
#include "Models/Model.h"

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
            void loadModels();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);

            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
            VgeDevice vgeDevice{vgeWindow};
            std::unique_ptr<VgeSwapChain> vgeSwapChain;
            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::unique_ptr<Model> vgeModel;
    };
} // namespace
