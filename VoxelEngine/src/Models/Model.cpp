#include "Model.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

// std
#include <cassert>
#include <cstdint>
#include <cstring>

namespace vge{
    Model::Model(VgeDevice& device, const std::vector<Vertex> &vertices) : vgeDevice{device}{
        createVertexBuffer(vertices);
    }

    Model::~Model(){
        vkDestroyBuffer(vgeDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(vgeDevice.device(), vertexBufferMemory, nullptr);
    }


    void Model::createVertexBuffer(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        vgeDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );
        void *data;
        vkMapMemory(vgeDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(vgeDevice.device(), vertexBufferMemory);

    }
}
