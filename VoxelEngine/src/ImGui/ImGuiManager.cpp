#include "ImGuiManager.h"

#include "external/ImGuiDocking/imgui_internal.h"
#include "../Device/Device.h"
#include "../Window.h"
#include "../Utils/ellipse.h"

// libs
#include <cstdint>
#include "external/ImGuiDocking/imgui.h"
#include "external/ImGuiDocking/backends/imgui_impl_glfw.h"
#include "external/ImGuiDocking/backends/imgui_impl_vulkan.h"
#include "src/Rendering/Renderer.h"

// std
#include <src/systems/GalaxySystem.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <fstream>

namespace vge {

    VgeImgui::VgeImgui(
        Window &window,
        VgeDevice &device,
        Renderer &renderer,
        VkRenderPass renderPass,
        uint32_t imageCount,
        GalaxySystem* galaxySystem,
        Input* input
    ) : vgeDevice{device},
        vgeRenderer{renderer},
        galaxySystem{galaxySystem},
        input{input} {
        // Set up descriptor pool stored on this instance
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if(vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up descriptor pool for imgui!!!");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);  // Semi-transparent windows
        style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.2f, 0.6f, 0.8f, 0.7f);
        style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);  // Transparent docking background
        style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.device();

        // Get queue family indices
        QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
        init_info.QueueFamily = indices.graphicsFamily;
        init_info.Queue = device.graphicsQueue();

        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptorPool;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.RenderPass = renderPass;

        if (!ImGui_ImplVulkan_Init(&init_info)) {
            throw std::runtime_error("Failed to initialize ImGui Vulkan implementation!!!");
        }

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily by using some existin helper functions on vge device object
        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontsTexture();

        loadSettings();
    }


    VgeImgui::~VgeImgui(){
        saveSettings();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        vkDestroyDescriptorPool(vgeDevice.device(), descriptorPool, nullptr);
        ImGui::DestroyContext();
    }


    void VgeImgui::newFrame(){
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }


    void VgeImgui::render(VkCommandBuffer commandBuffer) {
        // this tells imgui that we're done setting up the current frame,
        // then gets the draw data from imgui and uses it to record to the provided
        // command buffer the necessary draw commands
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }


    void VgeImgui::saveSettings() {
        std::ofstream outFile(settingsFilePath);
        if (outFile.is_open()) {
            outFile << clear_color.x << " " << clear_color.y << " " << clear_color.z << " " << clear_color.w;
            outFile.close();
        }
    }

    void VgeImgui::loadSettings() {
        std::ifstream inFile(settingsFilePath);
        if (inFile.is_open()) {
            inFile >> clear_color.x >> clear_color.y >> clear_color.z >> clear_color.w;
            inFile.close();
            vgeRenderer.setBackgroundColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        }
    }


    void VgeImgui::beginDockspace() {
        // Configure flags
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

        if (opt_fullscreen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // Push transparent styles
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // Begin dockspace window
        ImGui::Begin("DockSpace", nullptr, window_flags);

        if (opt_padding)
            ImGui::PopStyleVar();
        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Pop transparent styles
        ImGui::PopStyleColor(3);

        // Create the actual dockspace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            // Set up default layout if not already done
            static bool first_time = true;
            if (first_time) {
                first_time = false;
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

                auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
                ImGui::DockBuilderDockWindow("Hierarchy", dock_id_right);
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }
    }

    void VgeImgui::endDockspace() {
        ImGui::End();
    }


    void VgeImgui::updatePerformanceMetrics() {
        float currentFrameTime = 1000.0f / ImGui::GetIO().Framerate;
        float currentFps = ImGui::GetIO().Framerate;
        float deltaTime = ImGui::GetIO().DeltaTime;

        // Initialize vectors if needed
        if (frameTimeHistory.empty()) {
            frameTimeHistory.resize(MAX_FRAME_HISTORY, currentFrameTime);
            fpsHistory.resize(MAX_FRAME_HISTORY, currentFps);
        }

        // Add new values to history
        for (int i = 0; i < MAX_FRAME_HISTORY - 1; i++) {
            frameTimeHistory[i] = frameTimeHistory[i + 1];
            fpsHistory[i] = fpsHistory[i + 1];
        }
        frameTimeHistory[MAX_FRAME_HISTORY - 1] = currentFrameTime;
        fpsHistory[MAX_FRAME_HISTORY - 1] = currentFps;

        // Update running averages every second
        timeSinceLastUpdate += deltaTime;
        if (timeSinceLastUpdate >= UPDATE_INTERVAL) {
            // Calculate new averages
            float newAvgFrameTime = 0.0f;
            float newAvgFps = 0.0f;
            for (int i = 0; i < MAX_FRAME_HISTORY; i++) {
                newAvgFrameTime += frameTimeHistory[i];
                newAvgFps += fpsHistory[i];
            }
            avgFrameTime = newAvgFrameTime / MAX_FRAME_HISTORY;
            avgFps = newAvgFps / MAX_FRAME_HISTORY;

            // Reset timer
            timeSinceLastUpdate = 0.0f;
        }
    }

/*------------------------MAIN------------------------------ */


    void VgeImgui::runHierarchy() {
        if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        renderGlobalControls();
        renderCameraControls();
        renderGalaxyParameters();
        renderPerformanceMetrics();

        ImGui::End();
    }


/*---------------------------------------------------------- */



    void VgeImgui::renderGlobalControls() {
        ImGui::Text("Global Controls");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Demo Window", &show_demo_window);
        if(ImGui::ColorEdit3("Sky Color", (float*)&clear_color, ImGuiColorEditFlags_NoInputs)) {
            vgeRenderer.setBackgroundColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        }

        ImGui::Spacing();
        ImGui::Separator();
    }

/*---------------------------------------------------------- */

    void VgeImgui::renderCameraControls() {
        ImGui::Text("Camera Controls");

        float currentSpeed = input->moveSpeed;
        if (ImGui::SliderFloat("Movement Speed", &currentSpeed, 1.0f, 20.0f, "%.1f")) {
            input->moveSpeed = currentSpeed;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

/*---------------------------------------------------------- */

    void VgeImgui::renderGalaxyParameters() {
        if (!ImGui::TreeNode("Galaxy Parameters")) return;

        bool parametersChanged = false;
        renderGalaxyShapeParameters(parametersChanged);

        ImGui::Spacing();
        ImGui::Text("Height Distribution Parameters");
        ImGui::Separator();

        renderHeightDistributionParameters(parametersChanged);

        ImGui::Spacing();
        if (ImGui::Button("Restore Defaults")) {
            restoreDefaultGalaxyParameters();
        }

        handleGalaxyParameterChanges(parametersChanged);
        ImGui::TreePop();
    }

/*---------------------------------------------------------- */

    void VgeImgui::renderGalaxyShapeParameters(bool& parametersChanged) {
        // Base Radius Control
        if (ImGui::DragFloat("Base Radius", &Ellipse::baseRadius, 0.01f, 0.1f, 5.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Starting radius for the first ellipse");
        }

        // Radius Increment Control
        if (ImGui::DragFloat("Radius Increment", &Ellipse::radiusIncrement, 0.01f, 0.1f, 2.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("How much larger each successive ellipse becomes");
        }

        // Base Tilt Control
        float baseTiltDegrees = glm::degrees(Ellipse::baseTilt);
        if (ImGui::DragFloat("Base Tilt", &baseTiltDegrees, 1.0f, -180.0f, 180.0f, "%.1f°")) {
            Ellipse::baseTilt = glm::radians(baseTiltDegrees);
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Starting tilt angle for the first ellipse");
        }

        // Tilt Increment Control
        float tiltIncrementDegrees = glm::degrees(Ellipse::tiltIncrement);
        if (ImGui::DragFloat("Tilt Increment", &tiltIncrementDegrees, 0.1f, 0.0f, 45.0f, "%.1f°")) {
            Ellipse::tiltIncrement = glm::radians(tiltIncrementDegrees);
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("How much additional tilt each successive ellipse gets");
        }

        // Eccentricity Control
        if (ImGui::DragFloat("Eccentricity", &Ellipse::eccentricity, 0.01f, 0.1f, 1.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Controls how elliptical the shapes are (1.0 = circular)");
        }
    }

/*---------------------------------------------------------- */

    void VgeImgui::renderHeightDistributionParameters(bool& parametersChanged) {
        // Central Intensity Control
        if (ImGui::DragFloat("Central Intensity (I_0)", &Ellipse::centralIntensity, 0.1f, 0.1f, 50.0f, "%.1f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Controls the intensity at the center of the galaxy (I0)");
        }

        // Constant (b) Control
        if (ImGui::DragFloat("Distribution Constant (b)", &Ellipse::constant, 0.1f, 0.1f, 10.0f, "%.1f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Controls how quickly the height decreases with radius (b)");
        }

        // Effective Radius Scale Control
        if (ImGui::DragFloat("Effective Radius (Re)", &Ellipse::effectiveRadiusScale, 0.1f, 0.1f, 10.0f, "%.1f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Multiplier for the effective radius (Re = baseRadius * scale)");
        }

        // Max Height Control
        if (ImGui::DragFloat("Max Height", &Ellipse::maxHeight, 0.01f, 0.1f, 2.0f, "%.2f")) {
            parametersChanged = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum possible height for any star");
        }
    }

/*---------------------------------------------------------- */

    void VgeImgui::renderPerformanceMetrics() {
        ImGui::Separator();

        updatePerformanceMetrics();

        ImGui::Text("Average:");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", avgFrameTime, avgFps);

        ImGui::Spacing();
        ImGui::Text("Current:");
        ImGui::Text("%.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
    }

/*---------------------------------------------------------- */

    void VgeImgui::handleGalaxyParameterChanges(bool parametersChanged) {
        if (parametersChanged) {
            Ellipse::generateEllipseParams(Ellipse::MAX_ELLIPSES);
            galaxySystem->updateGalaxyParameters();
        }
    }

/*---------------------------------------------------------- */

    void VgeImgui::restoreDefaultGalaxyParameters() {
        Ellipse::baseRadius = 1.83f;
        Ellipse::radiusIncrement = 0.5f;
        Ellipse::baseTilt = 0.0f;
        Ellipse::tiltIncrement = 0.16f;
        Ellipse::eccentricity = 0.8f;

        Ellipse::centralIntensity = 10.0f;
        Ellipse::constant = 1.4f;
        Ellipse::effectiveRadiusScale = 2.0f;
        Ellipse::maxHeight = 0.5f;

        Ellipse::generateEllipseParams(Ellipse::MAX_ELLIPSES);
        galaxySystem->updateGalaxyParameters();
    }
} // namespace
