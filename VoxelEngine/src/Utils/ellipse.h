#pragma once

#include "hashFunction.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>

namespace vge {

    class Ellipse {
        public:
            struct EllipseParams {
                float majorAxis;
                float minorAxis;
                float tiltAngle;
            };

            static inline const int MAX_ELLIPSES = 30;
            static inline std::vector<EllipseParams> ellipseParams;

            // Base parameters that will be scaled for each ellipse
            static inline float baseRadius = 1.0f;
            static inline float radiusIncrement = 0.5f;
            static inline float baseTilt = 0.0f;
            static inline float tiltIncrement = 0.16f;
            static inline float eccentricity = 0.8f;

            static void generateEllipseParams(int numEllipses) {
                ellipseParams.clear();

                for (int i = 0; i < numEllipses; i++) {
                    EllipseParams params;

                    // Calculate radius for this ellipse
                    float currentRadius = baseRadius + (i * radiusIncrement);

                    params.majorAxis = currentRadius;
                    params.minorAxis = currentRadius * eccentricity;

                    // Add a small tilt for each successive ellipse
                    params.tiltAngle = baseTilt + (i * tiltIncrement);

                    ellipseParams.push_back(params);
                }
            }

            // Calculate point on ellipse given parameter t
            static glm::vec3 calculateEllipsePoint(float t, const EllipseParams& params, uint32_t seed) {
                float x = params.majorAxis * std::cos(t) * std::cos(params.tiltAngle) -
                        params.minorAxis * std::sin(t) * std::sin(params.tiltAngle);

                float z = params.majorAxis * std::cos(t) * std::sin(params.tiltAngle) +
                        params.minorAxis * std::sin(t) * std::cos(params.tiltAngle);

                float heightFactor = std::sin(t * 2.0f + hash(seed) * 6.28318f);
                float heightScale = params.majorAxis * 0.5f;
                float y = heightFactor * heightScale;

                return glm::vec3(x, y, z);
            }

            // Get point for a specific ellipse index
            static glm::vec3 calculateEllipsePointForIndex(float t, int ellipseIndex, uint32_t seed) {
                if (ellipseIndex >= ellipseParams.size()) {
                    return glm::vec3(0.0f); // Return origin if invalid index
                }
                return calculateEllipsePoint(t, ellipseParams[ellipseIndex], seed);
            }
    };

} // namespace
