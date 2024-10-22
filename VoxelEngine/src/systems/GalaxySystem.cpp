#include "GalaxySystem.h"

#include <cassert>
#include <stdexcept>
#include <array>
#include <vulkan/vulkan_core.h>

namespace vge {

    GalaxySystem::GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : vgeDevice{device}, globalSetLayout{globalSetLayout}, descriptorPool{VK_NULL_HANDLE}, descriptorSet{VK_NULL_HANDLE} {
        createDescriptorSetLayout();
        createPipelineLayout();
        createPipelines(renderPass);
        createStarBuffer();
        createDescriptorSet();
    }

    GalaxySystem::~GalaxySystem() {
        if (graphicsPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vgeDevice.device(), graphicsPipelineLayout, nullptr);
        }

        if (computePipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vgeDevice.device(), computePipelineLayout, nullptr);
        }

        if (graphicsDescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(vgeDevice.device(), graphicsDescriptorSetLayout, nullptr);
        }

        if (computeDescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(vgeDevice.device(), computeDescriptorSetLayout, nullptr);
        }

        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(vgeDevice.device(), descriptorPool, nullptr);
        }
    }


    std::vector<VkVertexInputBindingDescription> GalaxySystem::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Star);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GalaxySystem::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Star, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Star, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Star, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Star, uv);

        return attributeDescriptions;
    }


    void GalaxySystem::createDescriptorSetLayout() {
        // For the compute pipeline
        VkDescriptorSetLayoutBinding layoutBindingCompute{};
        layoutBindingCompute.binding = 0;
        layoutBindingCompute.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindingCompute.descriptorCount = 1;
        layoutBindingCompute.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;  // Only compute stage
        layoutBindingCompute.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfoCompute{};
        layoutInfoCompute.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfoCompute.bindingCount = 1;
        layoutInfoCompute.pBindings = &layoutBindingCompute;

        if (vkCreateDescriptorSetLayout(vgeDevice.device(), &layoutInfoCompute, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout for compute!");
        }

        // For graphics stages (vertex, fragment)
        VkDescriptorSetLayoutBinding layoutBindingGraphics{};
        layoutBindingGraphics.binding = 0;
        layoutBindingGraphics.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindingGraphics.descriptorCount = 1;
        layoutBindingGraphics.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfoGraphics{};
        layoutInfoGraphics.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfoGraphics.bindingCount = 1;
        layoutInfoGraphics.pBindings = &layoutBindingGraphics;

        if (vkCreateDescriptorSetLayout(vgeDevice.device(), &layoutInfoGraphics, nullptr, &graphicsDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout for graphics!");
        }
    }

    void GalaxySystem::createDescriptorSet() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(vgeDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &computeDescriptorSetLayout;

        if (vkAllocateDescriptorSets(vgeDevice.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = starBuffer->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Star) * stars.size();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vgeDevice.device(), 1, &descriptorWrite, 0, nullptr);
    }


    void GalaxySystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        // Use two different layouts for graphics and compute pipelines
        std::array<VkDescriptorSetLayout, 2> graphicsSetLayouts = {globalSetLayout, graphicsDescriptorSetLayout};
        std::array<VkDescriptorSetLayout, 2> computeSetLayouts = {globalSetLayout, computeDescriptorSetLayout};

        // For graphics pipeline layout
        VkPipelineLayoutCreateInfo graphicsPipelineLayoutInfo{};
        graphicsPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        graphicsPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(graphicsSetLayouts.size());
        graphicsPipelineLayoutInfo.pSetLayouts = graphicsSetLayouts.data();
        graphicsPipelineLayoutInfo.pushConstantRangeCount = 1;
        graphicsPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vgeDevice.device(), &graphicsPipelineLayoutInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout for graphics!");
        }

        // For compute pipeline layout
        VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};
        computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        computePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(computeSetLayouts.size());
        computePipelineLayoutInfo.pSetLayouts = computeSetLayouts.data();
        computePipelineLayoutInfo.pushConstantRangeCount = 1;
        computePipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vgeDevice.device(), &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout for compute!");
        }
    }

    void GalaxySystem::createPipelines(VkRenderPass renderPass) {
        assert(graphicsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = graphicsPipelineLayout;

        pipelineConfig.bindingDescriptions = getBindingDescriptions();
        pipelineConfig.attributeDescriptions = getAttributeDescriptions();

        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        graphicsPipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_vertex.vert.spv",
            "shaders/galaxy_fragment.frag.spv",
            pipelineConfig
        );

        PipelineConfigInfo computePipelineConfig{};
        computePipelineConfig.pipelineLayout = computePipelineLayout;
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

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            1,
            1,
            &descriptorSet,
            0,
            nullptr
        );

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

    void GalaxySystem::render(FrameInfo& frameInfo) {
        graphicsPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipelineLayout,
            1,
            1,
            &descriptorSet,
            0,
            nullptr
        );

        SimplePushConstantData push{};
        push.modelMatrix = glm::mat4(1.0f);
        push.normalMatrix = glm::mat4(1.0f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            graphicsPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push
        );

        VkBuffer buffers[] = {starBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(frameInfo.commandBuffer, static_cast<uint32_t>(stars.size()), 1, 0, 0);
    }

} // namespace
