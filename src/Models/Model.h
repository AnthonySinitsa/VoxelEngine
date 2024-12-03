#pragma once

#include "../Buffer/Buffer.h"
#include "../Device/Device.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace vge{

    struct Material {
        glm::vec3 diffuseColor{1.0f};   // Kd
        glm::vec3 ambientColor{0.0f};   // Ka
        glm::vec3 specularColor{0.0f};  // Ks
        std::string name;
    };

    class Model{
        public:

        struct Vertex{
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};
            int materialId{0};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return  position == other.position &&
                        color == other.color &&
                        normal == other.normal &&
                        uv == other.uv &&
                        materialId == other.materialId;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            std::vector<Material> materials{};

            void loadModel(const std::string &filepath);
        };

        const std::vector<Material>& getMaterials() const { return materials; }

        Model(VgeDevice &device, const Model::Builder &builder);
        ~Model();

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        static std::unique_ptr<Model> createModelFromFile(
            VgeDevice &device, const std::string &filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        private:
        void createVertexBuffer(const std::vector<Vertex> &vertices);
        void createIndexBuffer(const std::vector<uint32_t> &indices);

        VgeDevice& vgeDevice;

        std::unique_ptr<VgeBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<VgeBuffer> indexBuffer;
        uint32_t indexCount;

        std::vector<Material> materials{};
    };
} // namespace
