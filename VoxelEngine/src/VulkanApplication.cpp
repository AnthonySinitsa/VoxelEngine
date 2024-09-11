#include "VulkanApplication.h"
#include "Device/Device.h"
#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>


// std
#include <stdexcept>
#include <array>

namespace vge{

    VulkanApplication::VulkanApplication(){
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    VulkanApplication::~VulkanApplication(){
        vkDestroyPipelineLayout(vgeDevice.device(), pipelineLayout, nullptr);
    }

    void VulkanApplication::run(){
        while(!vgeWindow.shouldClose()){
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadModels(){
        std::vector<Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        };
        vgeModel = std::make_unique<Model>(vgeDevice, vertices);
    }


    void VulkanApplication::createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout!!!");
        }
    }


    void VulkanApplication::createPipeline(){
        auto pipelineConfig =
            Pipeline::defaultPipelineConfigInfo(
                vgeSwapChain->width(),
                vgeSwapChain->height());
        pipelineConfig.renderPass = vgeSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        vgePipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/vertex_shader.vert.spv",
            "shaders/fragment_shader.frag.spv",
            pipelineConfig
        );
    }


    void VulkanApplication::recreateSwapChain(){
        auto extent = vgeWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = vgeWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(vgeDevice.device());
        vgeSwapChain = std::make_unique<VgeSwapChain>(vgeDevice, extent);
        createPipeline();
    }


    void VulkanApplication::createCommandBuffers(){
        commandBuffers.resize(vgeSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vgeDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if(vkAllocateCommandBuffers(vgeDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate command buffers!!!");
        }
    }


    void VulkanApplication::recordCommandBuffer(int imageIndex){
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("failed to begin recording command buffer!!!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vgeSwapChain->getRenderPass();
        renderPassInfo.framebuffer = vgeSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vgeSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vgePipeline->bind(commandBuffers[imageIndex]);
        vgeModel->bind(commandBuffers[imageIndex]);
        vgeModel->draw(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if(vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS){
            throw std::runtime_error("failed to record command buffer!!!");
        }
    }


    void VulkanApplication::drawFrame(){
        uint32_t imageIndex;
        auto result = vgeSwapChain->acquireNextImage(&imageIndex);

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("failed to acquire swap chain image!!!");
        }

        result = vgeSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if(result != VK_SUCCESS){
            throw std::runtime_error("failed to acquire swap chain image!!!");
        }
    }
} // namespace
