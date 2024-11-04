#pragma once

#include "../Device/Device.h"
#include "../Graphics/Pipeline.h"
#include "../FrameInfo.h"
#include "../Buffer/Buffer.h"
#include "../Descriptor/Descriptors.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vge {

    struct Star {
        glm::vec3 position;
    };

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    class GalaxySystem {
    public:
        static constexpr int NUM_STARS = 10;
        static constexpr int WORKGROUP_SIZE = 256;

        GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~GalaxySystem();

        GalaxySystem(const GalaxySystem&) = delete;
        GalaxySystem& operator=(const GalaxySystem&) = delete;

        void render(FrameInfo& frameInfo);
        // void update(FrameInfo& frameInfo);
        void computeStars(FrameInfo& frameInfo);

    private:

        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        void createComputePipelineLayout();
        void createComputePipeline();
        void createComputeDescriptorSetLayout();
        void createComputeDescriptorSets();
        void createStarBuffer();
        void initStars();

        // Helper functions
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        VgeDevice& vgeDevice;

        // Graphics pipeline related
        std::unique_ptr<Pipeline> graphicsPipeline;
        VkPipelineLayout graphicsPipelineLayout;
        VkDescriptorSetLayout globalSetLayout;

        // Compute pipeline related
        std::unique_ptr<Pipeline> computePipeline;
        VkPipelineLayout computePipelineLayout;
        std::unique_ptr<VgeDescriptorSetLayout> computeDescriptorSetLayout;

        // Two descriptor sets for double buffering
        VkDescriptorSet computeDescriptorSetA;
        VkDescriptorSet computeDescriptorSetB;

        // Star data and buffers
        std::vector<Star> stars; // DELETE ME?
        std::unique_ptr<VgeBuffer> starBufferA;
        std::unique_ptr<VgeBuffer> starBufferB;
        bool useBufferA = true;

        // Descriptor pool for compute descriptor
        std::unique_ptr<VgeDescriptorPool> computeDescriptorPool;
    };
} // namespace vge
