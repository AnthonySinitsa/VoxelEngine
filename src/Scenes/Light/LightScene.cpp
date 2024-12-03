#include "LightScene.h"
#include "imgui.h"

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

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
        std::shared_ptr<Model> vgeModel =
            Model::createModelFromFile(device, ENGINE_DIR "models/Lowpoly_tree_sample.obj");

        // Create vase object
        auto tree = GameObject::createGameObject();
        tree.model = vgeModel;
        tree.transform.translation = {0.0f, 0.0f, 0.0f};
        tree.transform.scale = glm::vec3{1.0f};
        tree.transform.rotation = glm::vec3{glm::radians(180.0f), 0.f, 0.f};
        gameObjects.emplace(tree.getId(), std::move(tree));

        // Create point lights with different colors and positions
        // Central light
        auto centerLight = GameObject::makePointLight(20.0f, 0.1f, glm::vec3{1.0f, 1.0f, 1.0f});
        centerLight.transform.translation = {6.6f, -12.6f, 12.0f};
        gameObjects.emplace(centerLight.getId(), std::move(centerLight));

        // Red light
        auto redLight = GameObject::makePointLight(15.0f, 0.1f, glm::vec3{1.0f, 0.0f, 0.0f});
        redLight.transform.translation = {-0.2f, -16.0f, 4.8f};
        gameObjects.emplace(redLight.getId(), std::move(redLight));

        // Blue light
        auto blueLight = GameObject::makePointLight(10.0f, 0.1f, glm::vec3{0.0f, 0.0f, 1.0f});
        blueLight.transform.translation = {0.0f, -5.6f, 2.5f};
        gameObjects.emplace(blueLight.getId(), std::move(blueLight));

        auto greenLight = GameObject::makePointLight(20.0f, 0.1f, glm::vec3{0.0f, 1.0f, 0.0f});
        greenLight.transform.translation = {0.0f, -30.0f, 0.0f};
        gameObjects.emplace(greenLight.getId(), std::move(greenLight));
    }

    void LightScene::updateUbo(GlobalUbo& ubo, FrameInfo& frameInfo) {
        // Update point lights in UBO
        pointLightSystem->update(frameInfo, ubo, false);
    }

    void LightScene::update(FrameInfo& frameInfo) {
        // Get the existing UBO from frameInfo and update it with light information
        GlobalUbo ubo{};
        ubo.projection = frameInfo.camera.getProjection();
        ubo.view = frameInfo.camera.getView();
        ubo.inverseView = frameInfo.camera.getInverseView();

        // Set a low ambient light
        ubo.ambientLightColor = {0.02f, 0.02f, 0.02f, 0.02f};  // Last component is intensity
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
