#include "VulkanApplication.h"

#include "Buffer/Buffer.h"
#include "Camera/Camera.h"
#include "Descriptor/Descriptors.h"
#include "Game/GameObject.h"
#include "Input/Input.h"
#include "Presentation/SwapChain.h"
#include "Rendering/Renderer.h"
#include "systems/RenderSystem.h"
#include "systems/PointLight/PointLightSystem.h"
#include "Scenes/Galaxy/GalaxyScene.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cassert>
#include <chrono>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vge{

    VulkanApplication::VulkanApplication(){
        // Initialize global descriptor pool
        globalPool = VgeDescriptorPool::Builder(vgeDevice)
            .setMaxSets(VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        createDescriptorSetLayout();

        currentScene = std::unique_ptr<Scene>(new GalaxyScene(
            vgeDevice,
            vgeRenderer,
            globalSetLayout->getDescriptorSetLayout()
        ));

        // Initialize ImGui
        vgeImgui = std::make_unique<VgeImgui>(
            vgeWindow,
            vgeDevice,
            vgeRenderer,
            VgeSwapChain::MAX_FRAMES_IN_FLIGHT,
            &currentScene,
            globalSetLayout->getDescriptorSetLayout(),
            &input
        );
    }

    VulkanApplication::~VulkanApplication(){}


    void VulkanApplication::createDescriptorSetLayout() {
        globalSetLayout = VgeDescriptorSetLayout::Builder(vgeDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
    }

    void VulkanApplication::run(){
        std::vector<std::unique_ptr<VgeBuffer>> uboBuffers(VgeSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < uboBuffers.size(); i++){
            uboBuffers[i] = std::make_unique<VgeBuffer>(
                vgeDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        std::vector<VkDescriptorSet> globalDescriptorSets(VgeSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VgeDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{
            vgeDevice,
            vgeRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{
            vgeDevice,
            vgeRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto viewerObject = GameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        // Input input{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!vgeWindow.shouldClose()){
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            input.update(vgeWindow.getGLFWwindow()); // Toggle mouse lock/unlock
            input.moveInPlaneXZ(vgeWindow.getGLFWwindow(), frameTime, viewerObject); // Update camera
            input.mouseMove(vgeWindow.getGLFWwindow(), viewerObject);

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation); // Update camera view matrix

            // Set camera perspective projection
            float aspect = vgeRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f); // FYI: 1000.f is the clipping plane


            // Render the rest of the game objects using Vulkan
            if(auto commandBuffer = vgeRenderer.beginFrame()){
                int frameIndex = vgeRenderer.getFrameIndex();

                GameObject::Map& sceneObjects = currentScene ? currentScene->getGameObjects() : gameObjects;

                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    sceneObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();

                // Wait for GPU to finish before destroying scene
                if (currentScene && currentScene->shouldDestroy) {
                    vkDeviceWaitIdle(vgeDevice.device());
                    currentScene.reset();
                }

                if (currentScene) {
                    currentScene->update(frameInfo);
                    currentScene->updateUbo(ubo, frameInfo);
                }

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                vgeRenderer.beginSwapChainRenderPass(commandBuffer);

                if (currentScene) {
                    currentScene->render(frameInfo);
                }

                vgeImgui->newFrame();
                vgeImgui->beginDockspace();
                vgeImgui->runHierarchy();
                vgeImgui->endDockspace();
                vgeImgui->render(commandBuffer);

                vgeRenderer.endSwapChainRenderPass(commandBuffer);
                vgeRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadGameObjects(){
        std::shared_ptr<Model> vgeModel =
            Model::createModelFromFile(vgeDevice, "models/smooth_vase.obj");
        auto tree = GameObject::createGameObject();
        tree.model = vgeModel;
        tree.transform.translation = {1.0f, .0f, 0.f};
        tree.transform.scale = glm::vec3{2.2f};
        tree.transform.rotation = glm::vec3{glm::radians(0.0f), 0.f, 0.f};
        gameObjects.emplace(tree.getId(), std::move(tree));

        // Create a point light
        auto pointLight = GameObject::makePointLight(
            1.0f,     // intensity
            0.1f,      // radius
            glm::vec3{1.0f, 0.5f, 1.0f}  // color (white)
        );
        // Set position of the light
        pointLight.transform.translation = {0.0f, -1.0f, 0.0f};  // example position

        // Add to game objects map
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
} // namespace
