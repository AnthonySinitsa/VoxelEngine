#pragma once

#include "../Device/Device.h"
#include "../Graphics/Pipeline.h"
#include "../FrameInfo.h"
#include "../Buffer/Buffer.h"

#include <src/Descriptor/Descriptors.h>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vge {

    struct Star {
        glm::vec3 position;  // 3D position
        glm::vec3 velocity;
    };

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        float time;
    };

    struct ComputePushConstantData {
        float deltaTime;
        float totalTime;
        int numStars;
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
        void update(FrameInfo& frameInfo); // for compute shader dispatch
        void computeStars(FrameInfo& frameInfo); // dispatch compute work

    private:

        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        void createComputePipelineLayout();
        void createComputePipeline();
        void createComputeDescriptorSetLayout();
        void createComputeDescriptorSets();
        void createStarBuffer();
        void initStars(); // initialize star data

        // Helper functions
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        float totalTime = 0.0f;

        VgeDevice& vgeDevice;

        // Graphics pipeline related
        std::unique_ptr<Pipeline> graphicsPipeline;
        VkPipelineLayout graphicsPipelineLayout;
        VkDescriptorSetLayout globalSetLayout;

        // Compute pipeline related
        std::unique_ptr<Pipeline> computePipeline;
        VkPipelineLayout computePipelineLayout;
        std::unique_ptr<VgeDescriptorSetLayout> computeDescriptorSetLayout;
        VkDescriptorSet computeDescriptorSet;

        // Star data and buffers
        std::vector<Star> stars;
        std::unique_ptr<VgeBuffer> starBufferA;
        std::unique_ptr<VgeBuffer> starBufferB;
        bool useBufferA = true; // track which buffer is current

        // Descripto pool for compute descriptor
        std::unique_ptr<VgeDescriptorPool> computeDescriptorPool;
    };
} // namespace vge
