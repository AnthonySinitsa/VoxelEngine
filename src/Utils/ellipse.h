#pragma once

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
            static inline float baseRadius = 1.83f;
            static inline float radiusIncrement = 0.5f;
            static inline float baseTilt = 0.0f;
            static inline float tiltIncrement = 0.16f;
            static inline float eccentricity = 0.8f;

            // Galaxy shape using Vaucouleurs Law
            static inline float constant = 1.4f;
            static inline float baseRadius2 = 1.83f;
            static inline float centralIntensity = 10.0f;
            static inline float effectiveRadiusScale = 2.0f;
            static inline float maxHeight = 0.5f;

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

            static float calculateVaucouleursHeight(float x, float z, float maxHeight) {
                float radius = std::sqrt(x * x + z * z) + 0.0001f;
                float effectiveRadius = baseRadius2 * effectiveRadiusScale;
                float heightFactor = centralIntensity * std::exp(-constant * std::pow(radius/effectiveRadius, 0.25f));
                return maxHeight * heightFactor;
            }

            // Update the calculateEllipsePoint to use storedHeight parameter
            static glm::vec3 calculateEllipsePoint(float t, const EllipseParams& params, float storedHeight) {
                float x = params.majorAxis * std::cos(t) * std::cos(params.tiltAngle) -
                        params.minorAxis * std::sin(t) * std::sin(params.tiltAngle);

                float z = params.majorAxis * std::cos(t) * std::sin(params.tiltAngle) +
                        params.minorAxis * std::sin(t) * std::cos(params.tiltAngle);

                return glm::vec3(x, storedHeight, z);
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
