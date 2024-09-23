#include "ImGuiManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

void ImGuiManager::Init(
    GLFWwindow* window,
    VkInstance instance,
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkRenderPass renderPass
) {
    // Initialize ImGui for Vulkan
    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = device.getPhysicalDevice();  // Adjust based on how you retrieve it
    init_info.Device = device.device();  // Your Vulkan logical device
    init_info.Queue = device.getGraphicsQueue();  // Adjust this to fit your device class
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = device.getDescriptorPool();  // You may need to add this if it's managed by VgeDevice
    init_info.MinImageCount = 2;
    init_info.ImageCount = device.getSwapChainImageCount();  // Adjust based on how you manage swapchain images
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    // Record a single-time command buffer to upload fonts
    VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture();
    device.endSingleTimeCommands(commandBuffer);

    ImGui_ImplVulkan_DestroyFontsTexture();
}

void ImGuiManager::BeginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render() {
    // Create a simple test window
    ImGui::Begin("Test Window");
    if (ImGui::Button("Press me")) {
        // Do something
    }
    ImGui::End();

    // Render ImGui
    ImGui::Render();
}

void ImGuiManager::Shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
