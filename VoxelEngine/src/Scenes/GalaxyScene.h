#pragma once

#include "Scene.h"
#include "../systems/Galaxy/GalaxySystem.h"
#include "../Device/Device.h"
#include "../Rendering/Renderer.h"


namespace vge {

    class GalaxyScene : public Scene {
    public:
        GalaxyScene(VgeDevice& device, Renderer& renderer, VkDescriptorSetLayout globalSetLayout);
        ~GalaxyScene() override = default;

        void init() override;
        void update(FrameInfo& frameInfo) override;
        void render(FrameInfo& frameInfo) override;
        void renderUI() override;
        void updateUbo(GlobalUbo& ubo, FrameInfo& frameInfo) override;
        const char* getName() const override { return "Galaxy Scene"; }

        // UI helper methods
        void renderGalaxyShapeParameters(bool& parametersChanged);
        void renderHeightDistributionParameters(bool& parametersChanged);
        void handleGalaxyParameterChanges(bool parametersChanged);
        void restoreDefaultGalaxyParameters();

    private:
        std::unique_ptr<GalaxySystem> galaxySystem;
    };

} // namespace
