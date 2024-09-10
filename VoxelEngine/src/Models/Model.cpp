#include "Model.h"
#include <vulkan/vulkan_core.h>

// std
#include <cassert>
#include <cstdint>

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
    }
}
