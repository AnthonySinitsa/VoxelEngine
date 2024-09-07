#include "VulkanApplication.h"
#include <vulkan/vulkan_core.h>


// std
#include <stdexcept>

namespace vge{

    VulkanApplication::VulkanApplication(){
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    VulkanApplication::~VulkanApplication(){
        vkDestroyPipelineLayout(vgeDevice.device(), pipelineLayout, nullptr);
    }

    void VulkanApplication::run(){
        while(!vgeWindow.shouldClose()){
            glfwPollEvents();
        }
    }

    void VulkanApplication::createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayoutInfo) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout!!!");
        }
    }

    void VulkanApplication::createPipeline(){
        auto pipelineConfig =
            Pipeline::defaultPipelineConfigInfo(vgeSwapChain.width(), vgeSwapChain.height());
        pipelineConfig.renderPass = vgeSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        vgePipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/vertex_shader.vert.spv",
            "shaders/fragment_shader.frag.spv",
            pipelineConfig
        );
    }

    void VulkanApplication::createCommandBuffers(){}
    void VulkanApplication::drawFrame(){}
} // namespace
