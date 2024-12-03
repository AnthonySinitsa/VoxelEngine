#pragma once

#include "../Window.h"
#include "../Device/Device.h"
#include "../Presentation/SwapChain.h"

// std
#include <memory>
#include <vector>
#include <cassert>
#include <vulkan/vulkan_core.h>
#include <array>

namespace vge {
    class Renderer {

        public:
            Renderer(Window &window, VgeDevice &device);
            ~Renderer();

            Renderer(const Renderer &) = delete;
            Renderer &operator=(const Renderer &) = delete;

            VkRenderPass getSwapChainRenderPass() const { return vgeSwapChain->getRenderPass(); }
            float getAspectRatio() const { return vgeSwapChain->extentAspectRatio(); }
            bool isFrameInProgress() const { return isFrameStarted; }

            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
                return commandBuffers[currentFrameIndex];
            }

            int getFrameIndex() const {
                assert(isFrameStarted && "Cannot get frame index when frame not in progress.");
                return currentFrameIndex;
            }

            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

            void setBackgroundColor(float r, float g, float b, float a) {
                backgroundColor = {r, g, b, a};
            }

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

            Window& vgeWindow;
            VgeDevice& vgeDevice;
            std::unique_ptr<VgeSwapChain> vgeSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted{false};

            std::array<float, 4> backgroundColor{0.01f, 0.01f, 0.01f, 1.0f};
    };
} // namespace
