#pragma once

#include "../Device/Device.h"
#include "../Graphics/Pipeline.h"
#include "../FrameInfo.h"
#include "../Buffer/Buffer.h"
#include "../Descriptor/Descriptors.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

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

    private:
        VkDescriptorSetLayout globalSetLayout;

        void createPipelineLayout();
        void createPipelines(VkRenderPass renderPass);
        void createStarBuffer();
        void createDescriptorSetLayout();
        void createDescriptorSet();

        VgeDevice& vgeDevice;
        std::unique_ptr<Pipeline> graphicsPipeline;
        std::unique_ptr<Pipeline> computePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<Star> stars;
        std::unique_ptr<VgeBuffer> starBuffer;

        std::unique_ptr<VgeDescriptorSetLayout> descriptorSetLayout;
        VkDescriptorSet descriptorSet;
    };

} // namespace
