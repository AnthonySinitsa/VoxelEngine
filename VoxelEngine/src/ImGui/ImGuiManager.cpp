#include "ImGuiManager.h"

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
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <fstream>

namespace vge {

    VgeImgui::VgeImgui(
        Window &window, VgeDevice &device, Renderer &renderer, VkRenderPass renderPass, uint32_t imageCount
    ) : vgeDevice{device}, vgeRenderer{renderer}{
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


    void VgeImgui::runHierarchy() {
        if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        {
            ImGui::Begin("Hierarchy");

            // Demo and color controls section
            ImGui::Text("Global Controls");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Demo Window", &show_demo_window);
            if(ImGui::ColorEdit3("clear color", (float *)&clear_color)) {
                vgeRenderer.setBackgroundColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Ellipse Controls in a TreeNode
            if (ImGui::TreeNode("Galaxy Parameters")) {
                ImGui::TextWrapped("Adjust the shape and orientation of the galaxy's elliptical paths");
                ImGui::Spacing();

                // Inner Ellipse Controls
                if (ImGui::TreeNode("Inner Ellipse")) {
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Parameters for the inner ring of stars");
                    }

                    ImGui::DragFloat("Major Axis##Inner", &Ellipse::innerEllipse.majorAxis, 0.01f, 0.1f, 5.0f);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Controls the length of the ellipse");
                    }

                    ImGui::DragFloat("Minor Axis##Inner", &Ellipse::innerEllipse.minorAxis, 0.01f, 0.1f, 5.0f);

                    float innerDegrees = glm::degrees(Ellipse::innerEllipse.tiltAngle);
                    if (ImGui::DragFloat("Tilt Angle##Inner", &innerDegrees, 1.0f, 0.0f, 360.0f)) {
                        Ellipse::innerEllipse.tiltAngle = glm::radians(innerDegrees);
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Reset Inner Ellipse")) {
                        Ellipse::innerEllipse = {1.0f, 0.8f, M_PI / 6.0f};
                    }

                    ImGui::TreePop();
                }

                ImGui::Spacing();

                // Outer Ellipse Controls
                if (ImGui::TreeNode("Outer Ellipse")) {
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Parameters for the outer ring of stars");
                    }

                    ImGui::DragFloat("Major Axis##Outer", &Ellipse::outerEllipse.majorAxis, 0.01f, 0.1f, 5.0f);
                    ImGui::DragFloat("Minor Axis##Outer", &Ellipse::outerEllipse.minorAxis, 0.01f, 0.1f, 5.0f);

                    float outerDegrees = glm::degrees(Ellipse::outerEllipse.tiltAngle);
                    if (ImGui::DragFloat("Tilt Angle##Outer", &outerDegrees, 1.0f, 0.0f, 360.0f)) {
                        Ellipse::outerEllipse.tiltAngle = glm::radians(outerDegrees);
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Reset Outer Ellipse")) {
                        Ellipse::outerEllipse = {2.0f, 1.6f, M_PI / 3.0f};
                    }

                    ImGui::TreePop();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Reset All Ellipses")) {
                    Ellipse::innerEllipse = {1.0f, 0.8f, M_PI / 6.0f};
                    Ellipse::outerEllipse = {2.0f, 1.6f, M_PI / 3.0f};
                }

                ImGui::TreePop();
            }

            ImGui::Spacing();
            ImGui::Separator();

            // Performance metrics
            ImGui::Text("Performance");
            ImGui::Text("%.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate
            );

            ImGui::End();
        }
    }
} // namespace
