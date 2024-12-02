#include "LightScene.h"
#include "imgui.h"

namespace vge {

    LightScene::LightScene(VgeDevice& device, Renderer& renderer, VkDescriptorSetLayout globalSetLayout)
        : Scene(device, renderer, globalSetLayout) {
        init();
    }

    void LightScene::init() {
        renderSystem = std::make_unique<RenderSystem>(
            device,
            renderer.getSwapChainRenderPass(),
            globalSetLayout
        );

        pointLightSystem = std::make_unique<PointLightSystem>(
            device,
            renderer.getSwapChainRenderPass(),
            globalSetLayout
        );

        loadGameObjects();
    }

    void LightScene::loadGameObjects() {
        // Load model
        std::shared_ptr<Model> vgeModel = Model::createModelFromFile(device, "src/3dModels/smooth_vase.obj");

        // Create vase object
        auto vase = GameObject::createGameObject();
        vase.model = vgeModel;
        vase.transform.translation = {0.0f, 0.0f, 0.0f};
        vase.transform.scale = glm::vec3{3.0f};
        gameObjects.emplace(vase.getId(), std::move(vase));

        // Create point lights with different colors and positions
        // Central light
        auto centerLight = GameObject::makePointLight(1.0f, 0.1f, glm::vec3{1.0f, 1.0f, 1.0f});
        centerLight.transform.translation = {0.0f, 1.0f, 2.5f};  // Above the vase
        gameObjects.emplace(centerLight.getId(), std::move(centerLight));

        // Red light
        auto redLight = GameObject::makePointLight(1.0f, 0.1f, glm::vec3{1.0f, 0.0f, 0.0f});
        redLight.transform.translation = {-1.0f, 0.0f, 2.5f};  // Left of the vase
        gameObjects.emplace(redLight.getId(), std::move(redLight));

        // Blue light
        auto blueLight = GameObject::makePointLight(1.0f, 0.1f, glm::vec3{0.0f, 0.0f, 1.0f});
        blueLight.transform.translation = {1.0f, 0.0f, 2.5f};  // Right of the vase
        gameObjects.emplace(blueLight.getId(), std::move(blueLight));
    }

    void LightScene::updateUbo(GlobalUbo& ubo, FrameInfo& frameInfo) {
        // Update point lights in UBO
        pointLightSystem->update(frameInfo, ubo);
    }

    void LightScene::update(FrameInfo& frameInfo) {
        // Get the existing UBO from frameInfo and update it with light information
        GlobalUbo ubo{};
        ubo.projection = frameInfo.camera.getProjection();
        ubo.view = frameInfo.camera.getView();
        ubo.inverseView = frameInfo.camera.getInverseView();

        // Set a low ambient light
        ubo.ambientLightColor = {0.02f, 0.02f, 0.02f, 0.02f};  // Last component is intensity

        // Update point lights in UBO
        pointLightSystem->update(frameInfo, ubo);
    }

    void LightScene::render(FrameInfo& frameInfo) {
        // First render all regular objects
        renderSystem->renderGameObjects(frameInfo);

        // Then render point lights
        pointLightSystem->render(frameInfo);
    }

    void LightScene::renderUI() {
        if (!ImGui::TreeNode("Light Controls")) return;

        // Light control UI
        for (auto& kv : gameObjects) {
            auto& obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            // Create a unique label for each light
            std::string label = "Light " + std::to_string(kv.first);
            if (ImGui::TreeNode(label.c_str())) {
                // Light position control
                glm::vec3& pos = obj.transform.translation;
                ImGui::DragFloat3("Position", &pos.x, 0.1f);

                // Light color control
                ImGui::ColorEdit3("Color", &obj.color.x);

                // Light intensity control
                float& intensity = obj.pointLight->lightIntensity;
                ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 20.0f);

                // Light radius control
                float& radius = obj.transform.scale.x;
                ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 0.5f);

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

} // namespace
