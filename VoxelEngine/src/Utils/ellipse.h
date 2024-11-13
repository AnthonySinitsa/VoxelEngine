#pragma once

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

            static inline const int MAX_ELLIPSES = 30; // Maximum number of ellipses we can support
            static inline std::vector<EllipseParams> ellipseParams;

            // Base parameters that will be scaled for each ellipse
            static inline float baseRadius = 1.0f;           // Starting radius
            static inline float radiusIncrement = 0.5f;      // How much each ellipse grows
            static inline float baseTilt = 0.0f;             // Starting tilt (0 radians)
            static inline float tiltIncrement = 0.16f;       // Small increment per ellipse (about 2.86 degrees)
            static inline float eccentricity = 0.8f;         // Relationship between major and minor axis

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
            static glm::vec2 calculateEllipsePoint(float t, const EllipseParams& params) {
                float x = params.majorAxis * std::cos(t) * std::cos(params.tiltAngle) -
                        params.minorAxis * std::sin(t) * std::sin(params.tiltAngle);

                float y = params.majorAxis * std::cos(t) * std::sin(params.tiltAngle) +
                        params.minorAxis * std::sin(t) * std::cos(params.tiltAngle);

                return glm::vec2(x, y);
            }

            // Get point for a specific ellipse index
            static glm::vec2 calculateEllipsePointForIndex(float t, int ellipseIndex) {
                if (ellipseIndex >= ellipseParams.size()) {
                    return glm::vec2(0.0f); // Return origin if invalid index
                }
                return calculateEllipsePoint(t, ellipseParams[ellipseIndex]);
            }
    };

} // namespace
