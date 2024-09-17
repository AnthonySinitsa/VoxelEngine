#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vge{

    class Camera{
        public:
        void setOrthographicProjection(
            float left, float right,  float top, float bottom, float near, float far);

        void setPerspectiveProjection(float foovy, float aspect, float near, float far);

        const glm::mat4 getProjection()  const { return projectionMatrix; }

        private:
        glm::mat4 projectionMatrix{1.f};
    };
} // namespace
