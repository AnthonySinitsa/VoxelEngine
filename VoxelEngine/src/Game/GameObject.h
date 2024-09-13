#pragma once

#include "../Models/Model.h"

// std
#include <memory>

namespace vge{

    struct Transform2dComponent{
        glm::vec2 translation{}; // position offset

        glm::mat2 mat2() { return glm::mat2{1.f}; }
    };

    class GameObject{
        public:
        using id_t = unsigned int;

        static GameObject createGameObject(){
            static id_t currentId = 0;
            return GameObject{currentId++};
        }

        GameObject(const GameObject &) = delete;
        GameObject &operator=(const GameObject &) = delete;
        GameObject(GameObject&&) = default;
        GameObject &operator=(GameObject&&) = default;

        const id_t getId() { return id; }

        std::shared_ptr<Model> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

        private:
        GameObject(id_t objId) : id{objId} {}

        id_t id;
    };
} // namespace
