#pragma once

#include "../Device/Device.h"
#include "../Window.h"

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
            VgeImgui(Window &window, VgeDevice &device, VkRenderPass renderPass, uint32_t imageCount);
            ~VgeImgui();

            void newFrame();

            void render(VkCommandBuffer commandBuffer);

            bool show_demo_window = true;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            void runExample();
            void RenderUI();
            void ShowExampleAppDockSpace();

        private:
            VgeDevice &vgeDevice;

            VkDescriptorPool descriptorPool;
    };
} // namespace
