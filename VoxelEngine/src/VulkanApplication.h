#pragma once

#include "Device/Device.h"
#include "Game/GameObject.h"
#include "Rendering/Renderer.h"
#include "Window.h"
#include "Descriptor/Descriptors.h"
#include "ImGui/ImGuiManager.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vge {
    class VulkanApplication {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            VulkanApplication();
            ~VulkanApplication();

            VulkanApplication(const VulkanApplication &) = delete;
            VulkanApplication &operator=(const VulkanApplication &) = delete;

            void run();

        private:
            void loadGameObjects();

            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
            VgeDevice vgeDevice{vgeWindow};
            Renderer vgeRenderer{vgeWindow, vgeDevice};

            // note: order of desclaration matters
            std::unique_ptr<VgeDescriptorPool> globalPool{};
            GameObject::Map gameObjects;

            // ImGui manager
            std::unique_ptr<VgeImgui> vgeImgui{};
    };
} // namespace
