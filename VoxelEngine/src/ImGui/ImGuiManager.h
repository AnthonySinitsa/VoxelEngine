#pragma once

#include "../Device/Device.h"
#include "../Window.h"
#include "../Rendering/Renderer.h"

// libs
#include "external/ImGuiDocking/imgui.h"

namespace vge {

    static void check_vk_result(VkResult err) {
        if(err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if(err < 0) abort();
    }

    class VgeImgui {
        public:
            VgeImgui(Window &window, VgeDevice &device, Renderer &renderer, VkRenderPass renderPass, uint32_t imageCount);
            ~VgeImgui();

            void newFrame();

            void render(VkCommandBuffer commandBuffer);

            void runHierarchy();
            void RenderUI();
            void ShowExampleAppDockSpace();
            void controlBackgroundColor();

            void saveSettings();
            void loadSettings();

        private:
            Renderer &vgeRenderer;
            VgeDevice &vgeDevice;
            VkDescriptorPool descriptorPool;

            bool show_demo_window = false;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            const char* settingsFilePath = "imgui_settings.ini";
    };
} // namespace
