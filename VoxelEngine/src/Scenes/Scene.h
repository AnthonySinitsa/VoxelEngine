#pragma once

#include "../Device/Device.h"
#include "../FrameInfo.h"
#include "../Rendering/Renderer.h"
#include "../Game/GameObject.h"

namespace vge {

    class Scene {
    public:
        Scene(VgeDevice& device, Renderer& renderer, VkDescriptorSetLayout globalSetLayout)
            : device{device}, renderer{renderer}, globalSetLayout{globalSetLayout}, shouldDestroy{false} {}
        virtual ~Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        virtual void init() = 0;
        virtual void update(FrameInfo& frameInfo) = 0;
        virtual void render(FrameInfo& frameInfo) = 0;
        virtual void renderUI() = 0;
        virtual const char* getName() const = 0;

        bool shouldDestroy{false};
        GameObject::Map& getGameObjects() { return gameObjects; }

    protected:
        VgeDevice& device;
        Renderer& renderer;
        VkDescriptorSetLayout globalSetLayout;
        GameObject::Map gameObjects;
    };

} // namespace
