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
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vgeDevice.device(), pipelineLayout, nullptr);
        }

        if (descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(vgeDevice.device(), descriptorSetLayout, nullptr);
        }

        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(vgeDevice.device(), descriptorPool, nullptr);
        }
    }


    void GalaxySystem::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &layoutBinding;

        if (vkCreateDescriptorSetLayout(vgeDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
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
        allocInfo.pSetLayouts = &descriptorSetLayout;

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

        std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {globalSetLayout, descriptorSetLayout};

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
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout,
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
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
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
            pipelineLayout,
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
