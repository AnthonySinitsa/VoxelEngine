#pragma once

#include "../Window.h"
#include "../Device/Device.h"
#include "../Presentation/SwapChain.h"

// std
#include <cstdint>
#include <memory>
#include <vector>
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace vge {
    class Renderer {

        public:
            Renderer(Window &window, VgeDevice &device);
            ~Renderer();

            Renderer(const Renderer &) = delete;
            Renderer &operator=(const Renderer &) = delete;

            VkRenderPass getSwapChainRenderPass() const { return vgeSwapChain->getRenderPass(); }
            bool isFrameInProgress() const { return isFrameStarted; }

            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
                return commandBuffers[currentImageIndex];
            }

            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

            Window& vgeWindow;
            VgeDevice& vgeDevice;
            std::unique_ptr<VgeSwapChain> vgeSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            bool isFrameStarted;
    };
} // namespace
