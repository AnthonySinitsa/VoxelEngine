#pragma once

#include "../Device/Device.h"

// std
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>
#include <vector>

namespace vge {

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    // Mainly for graphics pipeline
    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;

    // Common for both compute and graphics pipelines
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;  // used for graphics, ingored in compute
    uint32_t subpass = 0;
};

class Pipeline {
   public:
    Pipeline(VgeDevice& device, const std::string& vertFilepath, const std::string& fragFilepath,
             const PipelineConfigInfo& configInfo);

    // Constructor for compute pipeline
    Pipeline(VgeDevice& device, const std::string& compFilepath,
             const PipelineConfigInfo& configInfo);

    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer,
              VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

   private:
    static std::vector<char> readFile(const std::string& filepath);

    // Functions for creating graphics and compute pipelines
    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath,
                                const PipelineConfigInfo& configInfo);

    void createComputePipeline(const std::string& compFilepath,
                               const PipelineConfigInfo& configInfo);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    VgeDevice& vgeDevice;
    VkPipeline graphicsPipeline;
    VkPipeline computePipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    VkShaderModule compShaderModule;
};
}  // namespace vge
