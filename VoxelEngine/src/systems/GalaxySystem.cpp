#include "GalaxySystem.h"
#include "../Buffer/Buffer.h"

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
                    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4)
                    .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                    .build();

                createComputeDescriptorSetLayout();
                createStarBuffer();
                createComputeDescriptorSets();
                createComputePipelineLayout();
                createComputePipeline();
                createPipelineLayout();
                createPipeline(renderPass);
                initStars();

            } catch (const std::exception& e) {
                assert("Error during GalaxySystem initialization!!!");
                throw;
            }
    }

    GalaxySystem::~GalaxySystem() {
        // Wait for device to be idle before cleanup
        vkDeviceWaitIdle(vgeDevice.device());

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
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{computeDescriptorSetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
                .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
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


    void GalaxySystem::createComputeDescriptorSets() {
        // Create descriptor sets with error checking
        auto bufferInfoA = starBufferA->descriptorInfo();
        auto bufferInfoB = starBufferB->descriptorInfo();

        // Try to create first descriptor set
        if (!VgeDescriptorWriter(*computeDescriptorSetLayout, *computeDescriptorPool)
            .writeBuffer(0, &bufferInfoA)
            .writeBuffer(1, &bufferInfoB)
            .build(computeDescriptorSetA)) {
            throw std::runtime_error("Failed to allocate descriptor set A");
        }

        // Try to create second descriptor set
        if (!VgeDescriptorWriter(*computeDescriptorSetLayout, *computeDescriptorPool)
            .writeBuffer(0, &bufferInfoB)
            .writeBuffer(1, &bufferInfoA)
            .build(computeDescriptorSetB)) {
            throw std::runtime_error("Failed to allocate descriptor set B");
        }
    }


    void GalaxySystem::initStars() {
        std::vector<Star> initialStars(NUM_STARS);
        float lineLength = 4.0f;
        float spacing = lineLength / (NUM_STARS - 1);

        std::cout << "Initializing " << NUM_STARS << " stars with spacing " << spacing << std::endl;
        for (int i = 0; i < NUM_STARS; i++) {
            float xPos = (i * spacing) - (lineLength / 2.0f);
            initialStars[i].position = glm::vec3(xPos, 0.0f, 0.0f);

            std::cout << "Initial Star " << i << " Position: "
                      << initialStars[i].position.x << ", "
                      << initialStars[i].position.y << ", "
                      << initialStars[i].position.z << std::endl;
        }

        // Write to buffers
        starBufferA->map();
        starBufferA->writeToBuffer(initialStars.data());
        starBufferA->unmap();

        starBufferB->map();
        starBufferB->writeToBuffer(initialStars.data());
        starBufferB->unmap();

        void* dataA = nullptr;
        if (vkMapMemory(vgeDevice.device(), starBufferA->getMemory(), 0, VK_WHOLE_SIZE, 0, &dataA) == VK_SUCCESS) {
            Star* starDataA = static_cast<Star*>(dataA);
            std::cout << "\nVerifying buffer A after initialization:" << std::endl;
            for (int i = 0; i < NUM_STARS; i++) {
                std::cout << "Buffer A Star " << i << " Position: "
                          << starDataA[i].position.x << ", "
                          << starDataA[i].position.y << ", "
                          << starDataA[i].position.z << std::endl;
            }
            vkUnmapMemory(vgeDevice.device(), starBufferA->getMemory());
        }
    }


    void GalaxySystem::update(FrameInfo& frameInfo) {
        // totalTime += frameInfo.frameTime;

        // std::cout << "Frame Time: " << frameInfo.frameTime << " seconds, Total Time: " << totalTime << " seconds" << std::endl;
    }


    void GalaxySystem::computeStars(FrameInfo& frameInfo) {
        static int frameCounter = 0;
        bool shouldDebug = (frameCounter % 1 == 0);

        // Determine the read and write buffers based on `useBufferA`
        VgeBuffer* readBuffer = useBufferA ? starBufferA.get() : starBufferB.get();
        VgeBuffer* writeBuffer = useBufferA ? starBufferB.get() : starBufferA.get();

        if (shouldDebug) {
            std::cout << "\nFrame " << frameCounter << " - COMPUTE PHASE" << std::endl;
            std::cout << "Reading from buffer " << (useBufferA ? "A" : "B") << std::endl;
            std::cout << "Writing to buffer " << (useBufferA ? "B" : "A") << std::endl;
        }

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

        // Debug: Check buffer contents after compute shader writes
        if (shouldDebug) {
            void* writeData = nullptr;
            if (vkMapMemory(vgeDevice.device(), writeBuffer->getMemory(), 0, VK_WHOLE_SIZE, 0, &writeData) == VK_SUCCESS) {
                Star* writeStars = static_cast<Star*>(writeData);
                std::cout << "Post-compute buffer state (first 10 stars):" << std::endl;
                for (int i = 0; i < 10; i++) {
                    std::cout << "Star " << i << " Position: "
                                << writeStars[i].position.x << ", "
                                << writeStars[i].position.y << ", "
                                << writeStars[i].position.z << std::endl;
                }
                vkUnmapMemory(vgeDevice.device(), writeBuffer->getMemory());
            }
        }

        frameCounter++;
    }


    void GalaxySystem::render(FrameInfo& frameInfo) {
        static int renderFrameCounter = 0;
        bool shouldDebug = (renderFrameCounter % 1 == 0);

        VgeBuffer* currentBuffer = useBufferA ? starBufferB.get() : starBufferA.get();

        if (shouldDebug) {
            std::cout << "\nRENDER PHASE" << std::endl;
            std::cout << "Rendering from buffer " << (!useBufferA ? "A" : "B") << std::endl;

            // Debug render buffer state
            void* renderData = nullptr;
            if (vkMapMemory(vgeDevice.device(), currentBuffer->getMemory(), 0, VK_WHOLE_SIZE, 0, &renderData) == VK_SUCCESS) {
                Star* renderStars = static_cast<Star*>(renderData);
                std::cout << "Pre-render buffer state (first 10 stars):" << std::endl;
                for (int i = 0; i < 10; i++) {
                    std::cout << "Star " << i << " Position: "
                              << renderStars[i].position.x << ", "
                              << renderStars[i].position.y << ", "
                              << renderStars[i].position.z << std::endl;
                }
                vkUnmapMemory(vgeDevice.device(), currentBuffer->getMemory());
            }
        }

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

        renderFrameCounter++;
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
