#include "VulkanApplication.h"

#include "Camera/Camera.h"
#include "Game/GameObject.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Renderer.h"
#include "Input/Input.h"

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

    VulkanApplication::VulkanApplication(){ loadGameObjects(); }

    VulkanApplication::~VulkanApplication(){}

    void VulkanApplication::run(){
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

            if (glfwGetKey(vgeWindow.getGLFWwindow(), GLFW_KEY_TAB) == GLFW_PRESS) {
                static bool cursorCaptured = true;
                cursorCaptured = !cursorCaptured;
                glfwSetInputMode(vgeWindow.getGLFWwindow(), GLFW_CURSOR,
                                 cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
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
                vgeRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                vgeRenderer.endSwapChainRenderPass(commandBuffer);
                vgeRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadGameObjects(){
        std::shared_ptr<Model> vgeModel = Model::createModelFromFile(vgeDevice, "/home/po/Projects/VoxelEngine/VoxelEngine/src/3dModels/Lowpoly_tree_sample.obj");

        auto cube = GameObject::createGameObject();
        cube.model = vgeModel;
        cube.transform.translation = {.0f, .0f, 2.5f};
        cube.transform.rotation = {.0f, .0f, 3.0f};
        cube.transform.scale = {.5, .5f, .5f};
        gameObjects.push_back(std::move(cube));
    }
} // namespace
