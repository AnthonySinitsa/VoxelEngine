#pragma once

#include "../Graphics/Pipeline.h"
#include "../Device/Device.h"
#include "../FrameInfo.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vge {
    class PointLight {

        public:
            PointLight(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~PointLight();

            PointLight(const PointLight &) = delete;
            PointLight &operator=(const PointLight &) = delete;

            void render(FrameInfo &frameInfo);

        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            VgeDevice& vgeDevice;

            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
    };
} // namespace
