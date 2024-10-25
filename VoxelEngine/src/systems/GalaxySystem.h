#pragma once

#include "../Device/Device.h"
#include "../Graphics/Pipeline.h"
#include "../FrameInfo.h"
#include "../Buffer/Buffer.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace vge {

    struct Star {
        glm::vec3 position;  // 3D position
        glm::vec3 color;     // RGB color
        float size;          // Point size
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

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        void createStarBuffer();

        // Helper functions
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        VgeDevice& vgeDevice;
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout globalSetLayout;

        std::vector<Star> stars;
        std::unique_ptr<VgeBuffer> starBuffer;
    };
} // namespace vge
