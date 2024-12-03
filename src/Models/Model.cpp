#include "Model.h"

#include "../Utils/utils.h"
#include "../Buffer/Buffer.h"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <stdexcept>
#include <cstddef>
#include <memory>
#include <iostream>

namespace std {
    template <>
    struct hash<vge::Model::Vertex> {
        size_t operator()(vge::Model::Vertex const &vertex) const {
            size_t seed = 0;
            vge::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
} // namespace std

namespace vge{
    Model::Model(VgeDevice& device, const Model::Builder &builder) : vgeDevice{device}{
        createVertexBuffer(builder.vertices);
        createIndexBuffer(builder.indices);
        materials = builder.materials;
    }

    Model::~Model(){}


    std::unique_ptr<Model> Model::createModelFromFile(
        VgeDevice &device, const std::string &filepath){
            Builder builder{};
            builder.loadModel(filepath);
            std::cout << "Vertex Count: " << builder.vertices.size() << "\n";
            return std::make_unique<Model>(device, builder);
        }


    void Model::createVertexBuffer(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        VgeBuffer stagingBuffer{
            vgeDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<VgeBuffer>(
            vgeDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vgeDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }


    void Model::createIndexBuffer(const std::vector<uint32_t> &indices){
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if(!hasIndexBuffer){
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        VgeBuffer stagingBuffer{
            vgeDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<VgeBuffer>(
            vgeDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vgeDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }


    void Model::bind(VkCommandBuffer commandBuffer){
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if(hasIndexBuffer){
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void Model::draw(VkCommandBuffer commandBuffer){
        if(hasIndexBuffer){
            // This command draws geometry based onan index buffer
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            // This command draws geometry directly from the vertex buffer, without using index buffer
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }


    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }


    void Model::Builder::loadModel(const std::string &filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> tinyMaterials;
        std::string warn, err;

        std::string baseDir = filepath.substr(0, filepath.find_last_of('/') + 1);

        if(!tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &warn, &err, filepath.c_str(), baseDir.c_str())) {
            throw std::runtime_error(warn + err);
        }

        materials.clear();
        // Convert tinyobj materials to our material format
        for (const auto& mat : tinyMaterials) {
            Material material{};
            material.name = mat.name;
            material.diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            material.ambientColor = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
            material.specularColor = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
            materials.push_back(material);
        }

        // If no materials were loaded, create a default white material
        if (materials.empty()) {
            Material defaultMaterial{};
            defaultMaterial.name = "default";
            defaultMaterial.diffuseColor = glm::vec3(1.0f);
            materials.push_back(defaultMaterial);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            // Inside the shape loop, before processing indices
            std::vector<int> faceMaterialIds;
            int currentFace = 0;
            for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {  // Since we have triangles
                int materialId = shape.mesh.material_ids.empty() ? 0 : shape.mesh.material_ids[i / 3];
                faceMaterialIds.push_back(materialId);
            }

            // Then in your vertex processing loop:
            for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
                const auto& index = shape.mesh.indices[i];
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                // Get material ID for this vertex based on which face it belongs to
                int materialId = shape.mesh.material_ids.empty() ? 0 : shape.mesh.material_ids[i / 3];
                vertex.materialId = materialId;

                // Always use material colors since we know we don't have vertex colors
                vertex.color = materials[materialId].diffuseColor;

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        std::cout << "Vertex Count: " << vertices.size() << std::endl;
    }


} // namespace
