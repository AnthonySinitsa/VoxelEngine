#include "GalaxySystem.h"
#include "../Buffer/Buffer.h"
#include "../Utils/ellipse.h"
#include "../Utils/hashFunction.h"

#include <glm/ext/quaternion_geometric.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <iostream>

namespace vge {

    GalaxySystem::GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : vgeDevice{device}, globalSetLayout{globalSetLayout} {

            try {
                computeDescriptorPool = VgeDescriptorPool::Builder(device)
                    .setMaxSets(2)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6)
                    .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                    .build();

                createComputeDescriptorSetLayout();
                createStarBuffer();
                createEllipseBuffer();
                updateEllipseBuffer();
                createComputeDescriptorSets();
                createComputePipelineLayout();
                createComputePipeline();
                createPipelineLayout();
                createPipeline(renderPass);
                initStars();

            } catch (const std::exception& e) {
                std::cerr << "Error during GalaxySystem initialization: " << e.what() << std::endl;
                assert("Error during GalaxySystem initialization!!!");
                throw;
            }
    }

    GalaxySystem::~GalaxySystem() {
        // Wait for device to be idle before cleanup
        vkDeviceWaitIdle(vgeDevice.device());

        if (ellipseBuffer) {
            ellipseBuffer->unmap();
        }

        if (computeDescriptorSetA != VK_NULL_HANDLE) {
            std::vector<VkDescriptorSet> sets = {computeDescriptorSetA};
            computeDescriptorPool->freeDescriptors(sets);
        }
        if (computeDescriptorSetB != VK_NULL_HANDLE) {
            std::vector<VkDescriptorSet> sets = {computeDescriptorSetB};
            computeDescriptorPool->freeDescriptors(sets);
        }

        vkDestroyPipelineLayout(vgeDevice.device(), graphicsPipelineLayout, nullptr);
        vkDestroyPipelineLayout(vgeDevice.device(), computePipelineLayout, nullptr);
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

        if (vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void GalaxySystem::createComputePipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ComputePushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{computeDescriptorSetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(vgeDevice.device(), &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline layout!");
        }
    }


    void GalaxySystem::createPipeline(VkRenderPass renderPass) {
        assert(graphicsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);

        // Configure for point rendering
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = graphicsPipelineLayout;
        pipelineConfig.bindingDescriptions = getBindingDescriptions();
        pipelineConfig.attributeDescriptions = getAttributeDescriptions();

        graphicsPipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_vertex.vert.spv",
            "shaders/galaxy_fragment.frag.spv",
            pipelineConfig
        );
    }

    void GalaxySystem::createComputePipeline() {
        assert(computePipelineLayout != nullptr && "Cannot create compute pipeline before pipeline layout");

        PipelineConfigInfo computePipelineConfig{};
        computePipelineConfig.pipelineLayout = computePipelineLayout;

        computePipeline = std::make_unique<Pipeline>(
            vgeDevice,
            "shaders/galaxy_compute.comp.spv",
            computePipelineConfig
        );
    }


    void GalaxySystem::createComputeDescriptorSetLayout() {
        computeDescriptorSetLayout = VgeDescriptorSetLayout::Builder(vgeDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
                .build();
    }


    void GalaxySystem::createStarBuffer() {
        VkDeviceSize bufferSize = sizeof(Star) * NUM_STARS;

        // Create two buffers for double buffering
        starBufferA = std::make_unique<VgeBuffer>(
            vgeDevice,
            sizeof(Star),
            NUM_STARS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        starBufferB = std::make_unique<VgeBuffer>(
            vgeDevice,
            sizeof(Star),
            NUM_STARS,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }

    void GalaxySystem::createEllipseBuffer() {
        ellipseBuffer = std::make_unique<VgeBuffer>(
            vgeDevice,
            sizeof(Ellipse::EllipseParams),
            MAX_ELLIPSES,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // Map the buffer and write the ellipse data
        ellipseBuffer->map();
    }


    void GalaxySystem::createComputeDescriptorSets() {
        // Create descriptor set A (buffer A -> buffer B)
        {
            auto bufferInfoA = starBufferA->descriptorInfo();
            auto bufferInfoB = starBufferB->descriptorInfo();
            auto ellipseBufferInfo = ellipseBuffer->descriptorInfo();

            if (!VgeDescriptorWriter(*computeDescriptorSetLayout, *computeDescriptorPool)
                .writeBuffer(0, &bufferInfoA)  // input buffer (binding 0)
                .writeBuffer(1, &bufferInfoB)  // output buffer (binding 1)
                .writeBuffer(2, &ellipseBufferInfo)
                .build(computeDescriptorSetA)) {
                throw std::runtime_error("Failed to create compute descriptor set A");
            }
        }

        // Create descriptor set B (buffer B -> buffer A)
        {
            auto bufferInfoA = starBufferA->descriptorInfo();
            auto bufferInfoB = starBufferB->descriptorInfo();
            auto ellipseBufferInfo = ellipseBuffer->descriptorInfo();

            if (!VgeDescriptorWriter(*computeDescriptorSetLayout, *computeDescriptorPool)
                .writeBuffer(0, &bufferInfoB)  // input buffer (binding 0)
                .writeBuffer(1, &bufferInfoA)  // output buffer (binding 1)
                .writeBuffer(2, &ellipseBufferInfo)
                .build(computeDescriptorSetB)) {
                throw std::runtime_error("Failed to create compute descriptor set B");
            }
        }
    }


    void GalaxySystem::updateEllipseBuffer() {
        // Generate new ellipse parameters
        Ellipse::generateEllipseParams(MAX_ELLIPSES);

        // Write to the buffer
        ellipseBuffer->writeToBuffer(Ellipse::ellipseParams.data(),
            sizeof(Ellipse::EllipseParams) * Ellipse::ellipseParams.size());
    }

    void GalaxySystem::updateGalaxyParameters() {
        updateEllipseBuffer();
    }


    void GalaxySystem::initStars() {
        std::vector<Star> initialStars(NUM_STARS);

        int starsPerEllipse = NUM_STARS / MAX_ELLIPSES;

        for (int ellipseIndex = 0; ellipseIndex < MAX_ELLIPSES; ellipseIndex++) {
            int startIndex = ellipseIndex * starsPerEllipse;
            int endIndex = (ellipseIndex == MAX_ELLIPSES - 1) ? NUM_STARS : startIndex + starsPerEllipse;
            int starsInThisEllipse = endIndex - startIndex;

            float angleStep = (2.0f * M_PI) / starsInThisEllipse;

            for (int i = startIndex; i < endIndex; i++) {
                float t = (i - startIndex) * angleStep;
                glm::vec3 point = Ellipse::calculateEllipsePoint(t, Ellipse::ellipseParams[ellipseIndex], i);

                // Generate random offsets using our hash function
                float randRadius = hash(float(i) * 12.345f) * 1.0f; // modify 1.0 for more radius
                float randAngle = hash(float(i) * 67.890f) * 2.0f * M_PI;

                // Calculate random offset in polar coordinates
                float offsetX = randRadius * cos(randAngle);
                float offsetZ = randRadius * sin(randAngle);

                // Apply the random offset to the original position
                initialStars[i].position = point + glm::vec3(offsetX, 0.0f, offsetZ);

                // Store the original angle and ellipse index for the compute shader
                initialStars[i].velocity = glm::vec3(t, offsetX, offsetZ);
            }
        }

        // Write to buffers
        starBufferA->map();
        starBufferA->writeToBuffer(initialStars.data());
        starBufferA->unmap();

        starBufferB->map();
        starBufferB->writeToBuffer(initialStars.data());
        starBufferB->unmap();
    }


    void GalaxySystem::update(FrameInfo& frameInfo) {
        // totalTime += frameInfo.frameTime;

        // std::cout << "Frame Time: " << frameInfo.frameTime << " seconds, Total Time: " << totalTime << " seconds" << std::endl;
    }


    void GalaxySystem::computeStars(FrameInfo& frameInfo) {
        // Bind the compute pipeline and descriptor set
        VkDescriptorSet currentDescriptorSet = useBufferA ? computeDescriptorSetA : computeDescriptorSetB;
        computePipeline->bind(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            0, 1,
            &currentDescriptorSet,
            0, nullptr
        );

        ComputePushConstants push{};
        push.numStars = NUM_STARS;
        push.numEllipses = MAX_ELLIPSES;
        push.deltaTime = frameInfo.frameTime;
        vkCmdPushConstants(
            frameInfo.commandBuffer,
            computePipelineLayout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(ComputePushConstants),
            &push
        );

        // Dispatch the compute shader
        vkCmdDispatch(
            frameInfo.commandBuffer,
            (NUM_STARS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE,
            1,
            1
        );

        // Memory barrier to ensure compute writes are visible to the vertex shader
        VkMemoryBarrier memoryBarrier{};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(
            frameInfo.commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );

        // Toggle buffer usage
        useBufferA = !useBufferA;
    }


    void GalaxySystem::render(FrameInfo& frameInfo) {
        VgeBuffer* currentBuffer = useBufferA ? starBufferB.get() : starBufferA.get();

        graphicsPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0, nullptr
        );

        SimplePushConstantData push{};
        push.modelMatrix = glm::mat4(1.0f);
        push.normalMatrix = glm::mat4(1.0f);

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            graphicsPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push
        );

        VkBuffer vertexBuffer = currentBuffer->getBuffer();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, &vertexBuffer, &offset);
        vkCmdDraw(frameInfo.commandBuffer, NUM_STARS, 1, 0, 0);
    }

    std::vector<VkVertexInputBindingDescription> GalaxySystem::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Star);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GalaxySystem::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Star, position);
        return attributeDescriptions;
    }
} // namespace
