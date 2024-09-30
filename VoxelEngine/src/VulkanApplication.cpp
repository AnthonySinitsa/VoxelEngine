#include "VulkanApplication.h"

#include "Camera/Camera.h"
#include "Game/GameObject.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"
#include "Buffer/Buffer.h"
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

    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

    VulkanApplication::VulkanApplication(){ loadGameObjects(); }

    VulkanApplication::~VulkanApplication(){}

    void VulkanApplication::run(){
        VgeBuffer globalUboBuffer{
            vgeDevice,
            sizeof(GlobalUbo),
            VgeSwapChain::MAX_FRAMES_IN_FLIGHT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            vgeDevice.properties.limits.minUniformBufferOffsetAlignment,
        };
        globalUboBuffer.map();

        RenderSystem renderSystem{vgeDevice, vgeRenderer.getSwapChainRenderPass()};
        Camera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = GameObject::createGameObject();
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
                    camera
                };

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                globalUboBuffer.writeToIndex(&ubo, frameIndex);
                globalUboBuffer.flushIndex(frameIndex);

                // render
                vgeRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo, gameObjects);
                vgeRenderer.endSwapChainRenderPass(commandBuffer);
                vgeRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadGameObjects(){
        std::shared_ptr<Model> vgeModel =
            Model::createModelFromFile(vgeDevice, "/home/po/Projects/VoxelEngine/VoxelEngine/src/3dModels/viking_room.obj");

        auto gameObj = GameObject::createGameObject();
        gameObj.model = vgeModel;
        gameObj.transform.translation = {.0f, .0f, 2.5f};
        gameObj.transform.scale = glm::vec3{1.f};
        gameObj.transform.rotation = glm::vec3{glm::radians(180.0f), 0.f, 0.f};
        gameObjects.push_back(std::move(gameObj));
    }
} // namespace
