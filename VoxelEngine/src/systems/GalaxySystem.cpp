#include "GalaxySystem.h"

#include <cassert>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace vge {

    GalaxySystem::GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : vgeDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipelines(renderPass);
        createStarBuffer();
    }

    GalaxySystem::~GalaxySystem() {
        vkDestroyPipelineLayout(vgeDevice.device(), pipelineLayout, nullptr);
    }

    void GalaxySystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void GalaxySystem::createPipelines(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipelineConfig.bindingDescriptions = Model::Vertex::getBindingDescriptions();
        pipelineConfig.attributeDescriptions = Model::Vertex::getAttributeDescriptions();

        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        graphicsPipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_vertex.vert.spv",
            "shaders/galaxy_fragment.frag.spv",
            pipelineConfig
        );

        PipelineConfigInfo computePipelineConfig{};
        computePipelineConfig.pipelineLayout = pipelineLayout;
        computePipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_compute.comp.spv",
            computePipelineConfig
        );
    }

    void GalaxySystem::createStarBuffer() {
        stars.resize(10);
        for (int i = 0; i < 10; i++) {
            stars[i].position = glm::vec2(i * 0.2f - 0.9f, 0.0f);
            // stars[i].size = 0.05f;
            stars[i].color = {1.0f, 1.0f, 1.0f};
            stars[i].normal = {0.0f, 0.0f, 1.0f};
            stars[i].uv = {0.0f, 0.0f};
        }

        VkDeviceSize bufferSize = sizeof(Star) * stars.size();

        starBuffer = std::make_unique<VgeBuffer>(
            vgeDevice,
            sizeof(Star),
            stars.size(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        starBuffer->map();
        starBuffer->writeToBuffer(stars.data());
    }

    void GalaxySystem::update(FrameInfo &frameInfo) {
        // Update star positions here for later
    }

    void GalaxySystem::computeStars(FrameInfo& frameInfo) {
        // Bind the compute pipeline and dispatch
        computePipeline->bind(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
        vkCmdDispatch(frameInfo.commandBuffer, 1, 1, 1);

        // Add a memory barrier to ensure compute shader writes are visible
        VkBufferMemoryBarrier bufferBarrier{};
        bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufferBarrier.buffer = starBuffer->getBuffer();
        bufferBarrier.offset = 0;
        bufferBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            frameInfo.commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            0, nullptr,
            1, &bufferBarrier,
            0, nullptr
        );
    }

    void GalaxySystem::render(FrameInfo &frameInfo) {
        // Now bind the graphics pipeline and draw
        graphicsPipeline->bind(frameInfo.commandBuffer);

        VkBuffer buffers[] = {starBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(frameInfo.commandBuffer, static_cast<uint32_t>(stars.size()), 1, 0, 0);
    }

} // namespace
