#pragma once

#include "Device/Device.h"
#include "Game/GameObject.h"
#include "Rendering/Renderer.h"
#include "Window.h"
#include "Descriptor/Descriptors.h"
#include "ImGui/ImGuiManager.h"
#include "Scenes/Scene.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vge {
    class VulkanApplication {

        public:
            static constexpr int WIDTH = 1800;
            static constexpr int HEIGHT = 900;

            VulkanApplication();
            ~VulkanApplication();

            VulkanApplication(const VulkanApplication &) = delete;
            VulkanApplication &operator=(const VulkanApplication &) = delete;

            void run();

        private:
            void loadGameObjects();
            void createDescriptorSetLayout();

            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA", false};
            VgeDevice vgeDevice{vgeWindow};
            Renderer vgeRenderer{vgeWindow, vgeDevice};

            // note: order of desclaration matters
            std::unique_ptr<VgeDescriptorPool> globalPool{};
            std::unique_ptr<VgeDescriptorSetLayout> globalSetLayout{};
            GameObject::Map gameObjects;

            std::unique_ptr<VgeImgui> vgeImgui{};
            std::unique_ptr<Scene> currentScene;
            Input input{};
    };
} // namespace
