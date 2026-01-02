#pragma once

#include "../Device/Device.h"
#include "../FrameInfo.h"
#include "../Graphics/Pipeline.h"

// std
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vge {
class RenderSystem {
   public:
    RenderSystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void renderGameObjects(FrameInfo& frameInfo);

   private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    VgeDevice& vgeDevice;

    std::unique_ptr<Pipeline> vgePipeline;
    VkPipelineLayout pipelineLayout;
};
}  // namespace vge
