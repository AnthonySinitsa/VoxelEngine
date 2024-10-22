#pragma once

#include "../Device/Device.h"
#include "../Graphics/Pipeline.h"
#include "../FrameInfo.h"
#include "../Buffer/Buffer.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vge {

    struct Star {
        glm::vec2 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    class GalaxySystem {
    public:
        GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~GalaxySystem();

        GalaxySystem(const GalaxySystem&) = delete;
        GalaxySystem& operator=(const GalaxySystem&) = delete;

        void update(FrameInfo& frameInfo);
        void computeStars(FrameInfo& frameInfo);
        void render(FrameInfo& frameInfo);

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    private:
        VkDescriptorSetLayout globalSetLayout;

        void createPipelineLayout();
        void createPipelines(VkRenderPass renderPass);
        void createStarBuffer();

        void createGraphicsDescriptorSetLayout();
        void createComputeDescriptorSetLayout();
        void createGraphicsDescriptorSet();
        void createComputeDescriptorSet();

        VgeDevice& vgeDevice;

        // Graphics and compute pipelines
        std::unique_ptr<Pipeline> graphicsPipeline;
        std::unique_ptr<Pipeline> computePipeline;

        // Separate pipeline layouts for graphics and compute
        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;

        // Descriptor set layouts for graphics and compute
        VkDescriptorSetLayout graphicsDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout computeDescriptorSetLayout = VK_NULL_HANDLE;

        // Descriptor set and pool
        VkDescriptorPool graphicsDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorPool computeDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet graphicsDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet computeDescriptorSet = VK_NULL_HANDLE;

        std::vector<Star> stars;
        std::unique_ptr<VgeBuffer> starBuffer;
    };

} // namespace
