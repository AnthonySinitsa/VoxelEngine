#pragma once

#include "../../Graphics/Pipeline.h"
#include "../../Device/Device.h"
#include "../../FrameInfo.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vge {
    class PointLightSystem {

        public:
            PointLightSystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~PointLightSystem();

            PointLightSystem(const PointLightSystem &) = delete;
            PointLightSystem &operator=(const PointLightSystem &) = delete;

            void update(FrameInfo &frameInfo, GlobalUbo &ubo, bool rotateLight = true);
            void render(FrameInfo &frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            VgeDevice& vgeDevice;

            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
    };
} // namespace
