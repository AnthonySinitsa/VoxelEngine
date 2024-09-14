#pragma once

#include "../Graphics/Pipeline.h"
#include "../Device/Device.h"
#include "../Game/GameObject.h"

// std
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vge {
    class RenderSystem {

        public:
            RenderSystem(VgeDevice& device, VkRenderPass renderPass);
            ~RenderSystem();

            RenderSystem(const RenderSystem &) = delete;
            RenderSystem &operator=(const RenderSystem &) = delete;

            void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects);

        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

            VgeDevice& vgeDevice;

            std::unique_ptr<Pipeline> vgePipeline;
            VkPipelineLayout pipelineLayout;
    };
} // namespace
