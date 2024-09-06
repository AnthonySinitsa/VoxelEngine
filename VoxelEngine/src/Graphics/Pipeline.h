#pragma once

#include "../Device/Device.h"

// std
#include <cstdint>
#include <string>
#include <vector>

namespace vge{

    struct PipelineConfigInfo{

    };

    class Pipeline{
        public:
        Pipeline(
            VgeDevice &device,
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo
        );
        ~Pipeline() {}

        Pipeline(const Pipeline&) = delete;
        void operator=(const Pipeline&) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeline(
            const std::string& vertFilepath,
            const std::string& fragFilepath, const PipelineConfigInfo& configInfo
        );

        void createShaderModule(const std::vector<char>& code, VkShaderModule *shaderModule);

        VgeDevice& vgeDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
} // namespace
