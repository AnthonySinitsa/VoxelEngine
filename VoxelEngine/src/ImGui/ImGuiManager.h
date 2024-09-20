#pragma once

#include "../Device/Device.h"
#include "../Rendering/Renderer.h"
#include "../Window.h"
#include <vulkan/vulkan.h>

namespace vge {

class ImGuiManager {
public:
    ImGuiManager(Window& window, VgeDevice& device, Renderer& renderer);
    ~ImGuiManager();

    void init();
    void newFrame();
    void render(VkCommandBuffer commandBuffer);
    void cleanup();

private:
    Window& window;
    VgeDevice& device;
    Renderer& renderer;
    VkDescriptorPool imguiPool = VK_NULL_HANDLE;

    void createDescriptorPool();
};

} // namespace
