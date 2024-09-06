#pragma once

#include "Window.h"
#include "Graphics/Pipeline.h"
#include "Device/Device.h"

namespace vge {
    class VulkanApplication {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();

        private:
            Window vgeWindow{WIDTH, HEIGHT, "AAAAAAAAAA"};
            VgeDevice vgeDevice{vgeWindow};
            Pipeline vgePipeline{
                vgeDevice,
                "shaders/vertex_shader.vert.spv",
                "shaders/fragment_shader.frag.spv",
                Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)
            };
    };
} // namespace
