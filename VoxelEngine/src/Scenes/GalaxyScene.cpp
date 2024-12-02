#include "GalaxyScene.h"
#include "external/ImGuiDocking/imgui.h"

namespace vge {

    GalaxyScene::GalaxyScene(VgeDevice& device, Renderer& renderer, VkDescriptorSetLayout globalSetLayout)
        : Scene(device, renderer, globalSetLayout) {
        init();
    }

    void GalaxyScene::init() {
        // Create galaxy system
        galaxySystem = std::make_unique<GalaxySystem>(
            device,
            renderer.getSwapChainRenderPass(),
            globalSetLayout
        );
    }

    void GalaxyScene::updateUbo(GlobalUbo& ubo, FrameInfo& frameInfo) {}

    void GalaxyScene::update(FrameInfo& frameInfo) {
        galaxySystem->update(frameInfo);
        galaxySystem->computeStars(frameInfo);
    }

    void GalaxyScene::render(FrameInfo& frameInfo) {
        galaxySystem->render(frameInfo);
    }


    void GalaxyScene::renderUI() {
        if (!ImGui::TreeNode("Galaxy Parameters")) return;

        bool parametersChanged = false;
        renderGalaxyShapeParameters(parametersChanged);

        ImGui::Spacing();
        ImGui::Text("Height Distribution Parameters");
        ImGui::Separator();

        renderHeightDistributionParameters(parametersChanged);

        ImGui::Spacing();
        if (ImGui::Button("Restore Defaults")) {
            restoreDefaultGalaxyParameters();
        }

        handleGalaxyParameterChanges(parametersChanged);
        ImGui::TreePop();
    }

    void GalaxyScene::renderGalaxyShapeParameters(bool& parametersChanged) {
        if (ImGui::DragFloat("Base Radius", &Ellipse::baseRadius, 0.01f, 1.0f, 5.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Starting radius for the first ellipse");
        }

        if (ImGui::DragFloat("Radius Increment", &Ellipse::radiusIncrement, 0.01f, 0.1f, 2.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("How much larger each successive ellipse becomes");
        }

        float baseTiltDegrees = glm::degrees(Ellipse::baseTilt);
        if (ImGui::DragFloat("Base Tilt", &baseTiltDegrees, 1.0f, -180.0f, 180.0f, "%.1f°")) {
            Ellipse::baseTilt = glm::radians(baseTiltDegrees);
            parametersChanged = true;
        }

        float tiltIncrementDegrees = glm::degrees(Ellipse::tiltIncrement);
        if (ImGui::DragFloat("Tilt Increment", &tiltIncrementDegrees, 0.1f, 0.0f, 45.0f, "%.1f°")) {
            Ellipse::tiltIncrement = glm::radians(tiltIncrementDegrees);
            parametersChanged = true;
        }

        if (ImGui::DragFloat("Eccentricity", &Ellipse::eccentricity, 0.01f, 0.1f, 1.0f, "%.2f")) {
            parametersChanged = true;
        }
    }

    void GalaxyScene::renderHeightDistributionParameters(bool& parametersChanged) {
        if (ImGui::DragFloat("Central Intensity (I_0)", &Ellipse::centralIntensity, 0.1f, 0.1f, 50.0f, "%.1f")) {
            parametersChanged = true;
        }

        if (ImGui::DragFloat("Base Radius2", &Ellipse::baseRadius2, 0.01f, 0.1f, 5.0f, "%.2f")) {
            parametersChanged = true;
        }

        if (ImGui::DragFloat("Distribution Constant (b)", &Ellipse::constant, 0.1f, 0.1f, 10.0f, "%.1f")) {
            parametersChanged = true;
        }

        if (ImGui::DragFloat("Effective Radius (Re)", &Ellipse::effectiveRadiusScale, 0.1f, 0.1f, 10.0f, "%.1f")) {
            parametersChanged = true;
        }

        if (ImGui::DragFloat("Max Height", &Ellipse::maxHeight, 0.01f, 0.1f, 2.0f, "%.2f")) {
            parametersChanged = true;
        }
    }

    void GalaxyScene::handleGalaxyParameterChanges(bool parametersChanged) {
        if (parametersChanged) {
            Ellipse::generateEllipseParams(Ellipse::MAX_ELLIPSES);
            galaxySystem->updateGalaxyParameters();
        }
    }

    void GalaxyScene::restoreDefaultGalaxyParameters() {
        Ellipse::baseRadius = 1.83f;
        Ellipse::radiusIncrement = 0.5f;
        Ellipse::baseTilt = 0.0f;
        Ellipse::tiltIncrement = 0.16f;
        Ellipse::eccentricity = 0.8f;

        Ellipse::centralIntensity = 10.0f;
        Ellipse::baseRadius2 = 1.83f;
        Ellipse::constant = 1.4f;
        Ellipse::effectiveRadiusScale = 2.0f;
        Ellipse::maxHeight = 0.5f;

        Ellipse::generateEllipseParams(Ellipse::MAX_ELLIPSES);
        galaxySystem->updateGalaxyParameters();
    }

} // namespace
