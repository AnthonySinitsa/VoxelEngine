#include "VulkanApplication.h"

#include "Rendering/RenderSystem.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace vge{

    VulkanApplication::VulkanApplication(){
        loadGameObjects();
    }

    VulkanApplication::~VulkanApplication(){}

    void VulkanApplication::run(){
        RenderSystem renderSystem{vgeDevice, vgeRenderer.getSwapChainRenderPass()};

        while(!vgeWindow.shouldClose()){
            glfwPollEvents();

            if(auto commandBuffer = vgeRenderer.beginFrame()){
                vgeRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects);
                vgeRenderer.endSwapChainRenderPass(commandBuffer);
            }
            vgeRenderer.endFrame();
        }

        vkDeviceWaitIdle(vgeDevice.device());
    }


    void VulkanApplication::loadGameObjects(){
        std::vector<Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        };
        auto vgeModel = std::make_shared<Model>(vgeDevice, vertices);

        auto triangle = GameObject::createGameObject();
        triangle.model = vgeModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
} // namespace
