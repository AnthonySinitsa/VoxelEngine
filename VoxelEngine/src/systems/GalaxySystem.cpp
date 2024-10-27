#include "GalaxySystem.h"

#include <stdexcept>
#include <random>

namespace vge {

    GalaxySystem::GalaxySystem(VgeDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : vgeDevice{device}, globalSetLayout{globalSetLayout} {

            try {
                computeDescriptorPool = VgeDescriptorPool::Builder(device)
                    .setMaxSets(1)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
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
        pushConstantRange.size = sizeof(ComputePushConstantData);

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
            .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)  // Input buffer
            .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)  // Output buffer
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

        starBufferA->map();
        starBufferB->map();
    }


    void GalaxySystem::createComputeDescriptorSets() {
        auto bufferInfoA = starBufferA->descriptorInfo();
        auto bufferInfoB = starBufferB->descriptorInfo();

        VgeDescriptorWriter(*computeDescriptorSetLayout, *computeDescriptorPool)
            .writeBuffer(0, &bufferInfoA)
            .writeBuffer(1, &bufferInfoB)
            .build(computeDescriptorSet);
    }


    void GalaxySystem::initStars() {
        std::vector<Star> initialStars(NUM_STARS);

        // Place stars in a circle
        float radius = 2.0f;  // Fixed radius for the circle
        for (int i = 0; i < NUM_STARS; i++) {
            float angle = (float)i / NUM_STARS * 2.0f * glm::pi<float>();

            // Position stars in a perfect circle on the XZ plane
            initialStars[i].position = glm::vec3(
                radius * std::cos(angle),  // X coordinate
                0.0f,                      // Y coordinate (flat on XZ plane)
                radius * std::sin(angle)   // Z coordinate
            );

            // Calculate orbital velocity for stable circular orbit
            float orbitalSpeed = 1.0f;  // Fixed orbital speed
            initialStars[i].velocity = glm::vec3(
                -std::sin(angle) * orbitalSpeed,  // Perpendicular to position
                0.0f,
                std::cos(angle) * orbitalSpeed
            );
        }

        // Write initial data to both buffers
        starBufferA->writeToBuffer(initialStars.data());
        starBufferB->writeToBuffer(initialStars.data());
    }


    void GalaxySystem::update(FrameInfo& frameInfo) {
        totalTime += frameInfo.frameTime;
    }


    void GalaxySystem::computeStars(FrameInfo& frameInfo) {
        computePipeline->bind(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            computePipelineLayout,
            0, 1,
            &computeDescriptorSet,
            0, nullptr
        );

        ComputePushConstantData push{};
        push.deltaTime = frameInfo.frameTime;
        push.totalTime = totalTime;
        push.numStars = NUM_STARS;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            computePipelineLayout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            sizeof(ComputePushConstantData),
            &push
        );

        vkCmdDispatch(
            frameInfo.commandBuffer,
            (NUM_STARS + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE,
            1,
            1
        );

        // Add memory barrier to ensure compute shader completes before rendering
        VkMemoryBarrier memoryBarrier{};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

        vkCmdPipelineBarrier(
            frameInfo.commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );

        // Swap buffers for next frame
        useBufferA = !useBufferA;
    }


    void GalaxySystem::render(FrameInfo& frameInfo) {
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
        push.time = totalTime;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            graphicsPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push
        );

        // Bind the current star buffer
        VkBuffer currentBuffer = useBufferA ? starBufferA->getBuffer() : starBufferB->getBuffer();
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(frameInfo.commandBuffer, 0, 1, &currentBuffer, &offset);

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
} // namespace vge
