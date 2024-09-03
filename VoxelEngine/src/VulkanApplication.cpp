#include "VulkanApplication.h"

namespace vge{
    void VulkanApplication::run(){
        while(!vgeWindow.shouldClose()){
            glfwPollEvents();
        }
    }
} // namespace
