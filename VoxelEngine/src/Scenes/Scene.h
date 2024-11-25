#pragma once

#include "../Device/Device.h"
#include "../FrameInfo.h"

namespace vge {

    class Scene {
    public:
        Scene(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
            : device{device}, renderPass{renderPass}, globalSetLayout{globalSetLayout} {}
        virtual ~Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        virtual void init() = 0;
        virtual void update(FrameInfo& frameInfo) = 0;
        virtual void render(FrameInfo& frameInfo) = 0;
        virtual void renderUI() = 0;
        virtual const char* getName() const = 0;

    protected:
        VgeDevice& device;
        VkRenderPass renderPass;
        VkDescriptorSetLayout globalSetLayout;
    };

} // namespace
