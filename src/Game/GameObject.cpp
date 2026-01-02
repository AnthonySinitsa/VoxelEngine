#include "GameObject.h"

#include <memory>

namespace vge {

glm::mat4 TransformComponent::mat4() {
    const float cosYaw = glm::cos(rotation.y);
    const float sinYaw = glm::sin(rotation.y);
    const float cosPitch = glm::cos(rotation.x);
    const float sinPitch = glm::sin(rotation.x);
    const float cosRoll = glm::cos(rotation.z);
    const float sinRoll = glm::sin(rotation.z);

    return glm::mat4{{
                         scale.x * (cosYaw * cosRoll + sinYaw * sinPitch * sinRoll),
                         scale.x * (cosPitch * sinRoll),
                         scale.x * (cosYaw * sinPitch * sinRoll - cosRoll * sinYaw),
                         0.0f,
                     },
                     {
                         scale.y * (cosRoll * sinYaw * sinPitch - cosYaw * sinRoll),
                         scale.y * (cosPitch * cosRoll),
                         scale.y * (cosYaw * cosRoll * sinPitch + sinYaw * sinRoll),
                         0.0f,
                     },
                     {
                         scale.z * (cosPitch * sinYaw),
                         scale.z * (-sinPitch),
                         scale.z * (cosYaw * cosPitch),
                         0.0f,
                     },
                     {translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat3 TransformComponent::normalMatrix() {
    const float cosYaw = glm::cos(rotation.y);
    const float sinYaw = glm::sin(rotation.y);
    const float cosPitch = glm::cos(rotation.x);
    const float sinPitch = glm::sin(rotation.x);
    const float cosRoll = glm::cos(rotation.z);
    const float sinRoll = glm::sin(rotation.z);
    const glm::vec3 invScale = 1.0f / scale;

    return glm::mat3{{
                         invScale.x * (cosYaw * cosRoll + sinYaw * sinPitch * sinRoll),
                         invScale.x * (cosPitch * sinRoll),
                         scale.x * (cosYaw * sinPitch * sinRoll - cosRoll * sinYaw),
                     },
                     {
                         invScale.y * (cosRoll * sinYaw * sinPitch - cosYaw * sinRoll),
                         invScale.y * (cosPitch * cosRoll),
                         invScale.y * (cosYaw * cosRoll * sinPitch + sinYaw * sinRoll),
                     },
                     {
                         invScale.z * (cosPitch * sinYaw),
                         invScale.z * (-sinPitch),
                         invScale.z * (cosYaw * cosPitch),
                     }};
}

GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
    GameObject gameObj = GameObject::createGameObject();
    gameObj.color = color;
    gameObj.transform.scale.x = radius;
    gameObj.pointLight = std::make_unique<PointLightComponent>();
    gameObj.pointLight->lightIntensity = intensity;
    return gameObj;
}
}  // namespace vge
