#include "VulkanApplication.h"

#include "Camera/Camera.h"
#include "Game/GameObject.h"
#include "systems/RenderSystem.h"
#include "systems/PointLightSystem.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"
#include "Buffer/Buffer.h"
#include <memory>
#include <src/Descriptor/Descriptors.h>
#include <src/Presentation/SwapChain.h>

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
        globalPool = VgeDescriptorPool::Builder(vgeDevice)
            .setMaxSets(VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VgeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        loadGameObjects();
    }

    VulkanApplication::~VulkanApplication(){}

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
        Input cameraController{vgeWindow.getGLFWwindow()};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!vgeWindow.shouldClose()){
            glfwPollEvents();

            if(cameraController.isEscapePressed(vgeWindow.getGLFWwindow())){
                glfwSetWindowShouldClose(vgeWindow.getGLFWwindow(), GLFW_TRUE);
                continue;
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // Update camera
            cameraController.moveInPlaneXZ(vgeWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

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

                // render
                vgeRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                vgeRenderer.endSwapChainRenderPass(commandBuffer);
                vgeRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadGameObjects(){
        std::shared_ptr<Model> vgeModel =
            Model::createModelFromFile(vgeDevice, "/home/po/Projects/VoxelEngine/VoxelEngine/src/3dModels/smooth_vase.obj");
        auto smoothVase = GameObject::createGameObject();
        smoothVase.model = vgeModel;
        smoothVase.transform.translation = {.0f, .0f, 0.f};
        smoothVase.transform.scale = glm::vec3{3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        vgeModel = Model::createModelFromFile(vgeDevice, "/home/po/Projects/VoxelEngine/VoxelEngine/src/3dModels/quad.obj");
        auto quad = GameObject::createGameObject();
        quad.model = vgeModel;
        quad.transform.translation = {.0f, .0f, 0.f};
        quad.transform.scale = glm::vec3{3.f};
        gameObjects.emplace(quad.getId(), std::move(quad));

        vgeModel = Model::createModelFromFile(vgeDevice, "/home/po/Projects/VoxelEngine/VoxelEngine/src/3dModels/Lowpoly_tree.obj");
        auto tree = GameObject::createGameObject();
        tree.model = vgeModel;
        tree.transform.translation = {2.5f, .0f, 0.f};
        tree.transform.scale = glm::vec3{.2f};
        tree.transform.rotation = glm::vec3{glm::radians(180.0f), 0.f, 0.f};
        gameObjects.emplace(tree.getId(), std::move(tree));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}
        };

        for(int i = 0; i < lightColors.size(); i++){
            auto pointLight = GameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
} // namespace
