#include "VulkanApplication.h"

#include "Camera/Camera.h"
#include "Game/GameObject.h"
#include "systems/RenderSystem.h"
#include "systems/PointLightSystem.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"
#include "Buffer/Buffer.h"
#include "external/ImGuiDocking/imgui_internal.h"
#include <memory>
#include <src/Descriptor/Descriptors.h>
#include <src/Presentation/SwapChain.h>
#include <src/systems/GalaxySystem.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <chrono>
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace vge{

    VulkanApplication::VulkanApplication(){
        // Initialize global descriptor pool
        globalPool = VgeDescriptorPool::Builder(vgeDevice)
            .setMaxSets(VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        createDescriptorSetLayout();
        createGalaxySystem();

        // Load game objects
        // loadGameObjects();

        // Initialize ImGui
        vgeImgui = std::make_unique<VgeImgui>(
            vgeWindow,
            vgeDevice,
            vgeRenderer,
            vgeRenderer.getSwapChainRenderPass(),
            VgeSwapChain::MAX_FRAMES_IN_FLIGHT
        );
    }

    VulkanApplication::~VulkanApplication(){}


    void VulkanApplication::createDescriptorSetLayout() {
        globalSetLayout = VgeDescriptorSetLayout::Builder(vgeDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
    }

    void VulkanApplication::createGalaxySystem() {
        galaxySystem = std::make_unique<GalaxySystem>(
            vgeDevice,
            vgeRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        );
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

        // MAYBE DELETE ME
        auto globalSetLayout = VgeDescriptorSetLayout::Builder(vgeDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

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
        Input input{};

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
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f); // FYI: 10.f is the clipping plane


            // Render the rest of the game objects using Vulkan
            if(auto commandBuffer = vgeRenderer.beginFrame()){
                int frameIndex = vgeRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                galaxySystem->update(frameInfo);
                galaxySystem->computeStars(frameInfo);

                vgeRenderer.beginSwapChainRenderPass(commandBuffer); // Begin swapchain render pass

                renderSystem.renderGameObjects(frameInfo); // Render game objects
                pointLightSystem.render(frameInfo);
                galaxySystem->render(frameInfo);

                vgeImgui->newFrame(); // Start new ImGui frame
                vgeImgui->beginDockspace();
                vgeImgui->runHierarchy();
                vgeImgui->endDockspace();
                vgeImgui->render(commandBuffer); // Render ImGui

                vgeRenderer.endSwapChainRenderPass(commandBuffer); // End swapchain render pass
                vgeRenderer.endFrame(); // End frame
            }
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    // void VulkanApplication::loadGameObjects(){
    //     std::shared_ptr<Model> vgeModel =
    //         Model::createModelFromFile(vgeDevice, "src/3dModels/Lowpoly_tree.obj");
    //     auto tree = GameObject::createGameObject();
    //     tree.model = vgeModel;
    //     tree.transform.translation = {.0f, .0f, 0.f};
    //     tree.transform.scale = glm::vec3{.2f};
    //     tree.transform.rotation = glm::vec3{glm::radians(180.0f), 0.f, 0.f};
    //     gameObjects.emplace(tree.getId(), std::move(tree));
    // }
} // namespace
