#pragma once

#include "Scene.h"
#include "../systems/RenderSystem.h"
#include "../systems/PointLight/PointLightSystem.h"

namespace vge {

    class LightScene : public Scene {
    public:
        LightScene(VgeDevice& device, Renderer& renderer, VkDescriptorSetLayout globalSetLayout);
        ~LightScene() override = default;

        void init() override;
        void update(FrameInfo& frameInfo) override;
        void render(FrameInfo& frameInfo) override;
        void renderUI() override;
        const char* getName() const override { return "Light Scene"; }

    private:
        void loadGameObjects();

        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<PointLightSystem> pointLightSystem;
    };

} // namespace
