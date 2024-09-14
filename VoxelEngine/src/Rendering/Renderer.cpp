#include "Renderer.h"

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace vge{
    Renderer::Renderer(Window &window, VgeDevice& device) : vgeWindow{window}, vgeDevice{device}{
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer(){ freeCommandBuffers(); }


    void Renderer::recreateSwapChain(){
        auto extent = vgeWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = vgeWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vgeDevice.device());

        if(vgeSwapChain == nullptr){
            vgeSwapChain = std::make_unique<VgeSwapChain>(vgeDevice, extent);
        } else {
            std::shared_ptr<VgeSwapChain> oldSwapChain = std::move(vgeSwapChain);
            vgeSwapChain = std::make_unique<VgeSwapChain>(vgeDevice, extent, oldSwapChain);

            if(!oldSwapChain->compareSwapFormats(*vgeSwapChain.get())){
                throw std::runtime_error("Swap chain image(or depth) format has changed!!!");
            }
        }
    }


    void Renderer::createCommandBuffers(){
        commandBuffers.resize(VgeSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vgeDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if(vkAllocateCommandBuffers(vgeDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate command buffers!!!");
        }
    }


    void Renderer::freeCommandBuffers(){
        vkFreeCommandBuffers(
            vgeDevice.device(),
            vgeDevice.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data()
        );
        commandBuffers.clear();
    }


    VkCommandBuffer Renderer::beginFrame(){
        assert(!isFrameStarted && "Can't call beginFrame while already in progress.");

        auto result = vgeSwapChain->acquireNextImage(&currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapChain();
            return nullptr;
        }
        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("failed to acquire swap chain image!!!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("failed to begin recording command buffer!!!");
        }
        return commandBuffer;
    }


    void Renderer::endFrame(){
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress.");
        auto commandBuffer = getCurrentCommandBuffer();
        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
            throw std::runtime_error("failed to record command buffer!!!");
        }

        auto result = vgeSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vgeWindow.wasWindowResized()){
            vgeWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if(result != VK_SUCCESS){
            throw std::runtime_error("failed to acquire swap chain image!!!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % VgeSwapChain::MAX_FRAMES_IN_FLIGHT;
    }


    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress.");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame.");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vgeSwapChain->getRenderPass();
        renderPassInfo.framebuffer = vgeSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vgeSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vgeSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(vgeSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, vgeSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }


    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress.");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame.");

        vkCmdEndRenderPass(commandBuffer);
    }
} // namespace
