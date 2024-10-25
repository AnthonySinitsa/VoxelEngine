// GalaxySystem.cpp
#include "GalaxySystem.h"

#include <stdexcept>

namespace vge {

    GalaxySystem::GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : vgeDevice{device}, globalSetLayout{globalSetLayout} {
        createPipelineLayout();
        createPipeline(renderPass);
        createStarBuffer();
    }

    GalaxySystem::~GalaxySystem() {
        vkDestroyPipelineLayout(vgeDevice.device(), pipelineLayout, nullptr);
    }

    void GalaxySystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void GalaxySystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);

        // Configure for point rendering
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.bindingDescriptions = getBindingDescriptions();
        pipelineConfig.attributeDescriptions = getAttributeDescriptions();

        pipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_vertex.vert.spv",
            "shaders/galaxy_fragment.frag.spv",
            pipelineConfig
        );
    }

    void GalaxySystem::createStarBuffer() {
        stars.resize(10);  // Start with 10 stars

        // Initialize stars in a simple line formation
        for (int i = 0; i < 10; i++) {
            stars[i].position = glm::vec3(
                i * 1.0f - 5.0f,  // X position: -5 to 5
                0.0f,             // Y position: all in same plane
                0.0f              // Z position: all at same depth
            );
            stars[i].color = glm::vec3(1.0f);  // White color
            stars[i].size = 10.0f;             // Uniform size
        }

        starBuffer = std::make_unique<VgeBuffer>(
            vgeDevice,
            sizeof(Star),
            stars.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        starBuffer->map();
        starBuffer->writeToBuffer(stars.data());
    }

    void GalaxySystem::render(FrameInfo& frameInfo) {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        SimplePushConstantData push{};
        push.modelMatrix = glm::mat4(1.0f);
        push.normalMatrix = glm::mat4(1.0f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push
        );

        VkBuffer buffers[] = {starBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(frameInfo.commandBuffer, static_cast<uint32_t>(stars.size()), 1, 0, 0);
    }

    std::vector<VkVertexInputBindingDescription> GalaxySystem::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Star);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GalaxySystem::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Star, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Star, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Star, size);

        return attributeDescriptions;
    }
} // namespace vge
