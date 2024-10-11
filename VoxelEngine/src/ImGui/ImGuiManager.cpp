#include "ImGuiManager.h"

#include "../Device/Device.h"
#include "../Window.h"

// libs
#include <cstdint>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// std
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace vge {

    VgeImgui::VgeImgui(
        Window &window, VgeDevice &device, VkRenderPass renderPass, uint32_t imageCount
    ) : vgeDevice{device}{
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
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

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
        // todo, should prob get around to integrate memory allocator library such
        // as vulkan memory allocator(VMA)
        init_info.Allocator = nullptr;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        if (!ImGui_ImplVulkan_Init(&init_info)) {
            throw std::runtime_error("Failed to initialize ImGui Vulkan implementation!!!");
        }

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily by using some existin helper functions on vge device object
        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture();
        device.endSingleTimeCommands(commandBuffer);
        // AAAAAAAAAAAAAAAAAAAAAAaa
        ImGui_ImplVulkan_DestroyFontsTexture();
    }


    VgeImgui::~VgeImgui(){
        vkDestroyDescriptorPool(vgeDevice.device(), descriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }


    void VgeImgui::newFrame(){
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }


    // this tells imgui that we're done setting up the current frame,
    // then gets the draw data from imgui and uses it to record to the provided
    // command buffer the necessary draw commands
    void VgeImgui::render(VkCommandBuffer commandBuffer) {
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }


    void VgeImgui::runExample(){
        // 1. Show big demo window(most of the sample code is in ImGui::ShowDemoWindow()!
        // Can browse its code to learn more about Dear ImGui!).
        if(show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show simple window that we create ourselves. we use  a begin/end pair to created
        // a named window
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("World, James World");// create a window called "" and append into it.

            ImGui::Text(
                "This is some useful text. AI poo"
            ); // display some text(you  can use format string too)

            ImGui::Checkbox(
                "demo WIIINDOW",
                &show_demo_window
            ); // edit bools storing our window open/close state
            ImGui::Checkbox("Another Window HERHERHERH", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // edit 1 float using slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // edit 3 floats representing a color

            if(ImGui::Button("Button")) // Buttons return  true when clicked(most widgets return  tru when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text(
                "Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate
            );
            ImGui::End();
        }


        // 3. Show another simple window
        if(show_another_window){
            ImGui::Begin(
                "Another Window YARHHRH",
                &show_another_window // Pass a pointer to our bool variable(the window will have
                                      // closing button that will clear the bool when clicked)
            );

            ImGui::Text("Hello from another window!!!");
            if(ImGui::Button("Close Me")) show_another_window = false;
            ImGui::End();
        }
    }
} // namespace
