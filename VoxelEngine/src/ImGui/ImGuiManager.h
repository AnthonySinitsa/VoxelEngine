#pragma once

#include "../Device/Device.h"
#include "../Window.h"
#include "../Rendering/Renderer.h"
#include "../systems/Galaxy/GalaxySystem.h"
#include "../Input/Input.h"

// libs
#include "external/ImGuiDocking/imgui.h"

namespace vge {

    class GalaxySystem;
    class Input;

    static void check_vk_result(VkResult err) {
        if(err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if(err < 0) abort();
    }

    class VgeImgui {
        public:
            VgeImgui(
                Window &window,
                VgeDevice &device,
                Renderer &renderer,
                VkRenderPass renderPass,
                uint32_t imageCount,
                GalaxySystem* galaxySystem,
                Input* input);
            ~VgeImgui();

            void newFrame();

            void render(VkCommandBuffer commandBuffer);

            void runHierarchy();
            void RenderUI();
            void ShowExampleAppDockSpace();
            void controlBackgroundColor();

            void saveSettings();
            void loadSettings();

            void beginDockspace();
            void endDockspace();

            void updatePerformanceMetrics();

            // UI Section Renderers
            void renderGlobalControls();
            void renderCameraControls();
            void renderGalaxyParameters();
            void renderPerformanceMetrics();

            // Galaxy Parameter Sub-sections
            void renderGalaxyShapeParameters(bool& parametersChanged);
            void renderHeightDistributionParameters(bool& parametersChanged);

            // Helper functions
            void handleGalaxyParameterChanges(bool parametersChanged);
            void restoreDefaultGalaxyParameters();

        private:
            Renderer &vgeRenderer;
            VgeDevice &vgeDevice;
            VkDescriptorPool descriptorPool;

            bool show_demo_window = false;
            bool show_another_window = false;
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            const char* settingsFilePath = "imgui_settings.ini";

            bool opt_fullscreen = true;
            bool opt_padding = true;
            ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            static const int MAX_FRAME_HISTORY = 150;
            std::vector<float> frameTimeHistory;
            std::vector<float> fpsHistory;
            float avgFrameTime = 0.0f;
            float avgFps = 0.0f;
            float timeSinceLastUpdate = 0.0f;
            const float UPDATE_INTERVAL = 1.0f;

            GalaxySystem* galaxySystem;
            Input* input;
    };
} // namespace
