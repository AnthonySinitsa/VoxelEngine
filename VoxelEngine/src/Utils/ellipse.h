#pragma once

#include <glm/glm.hpp>
#include <cmath>

namespace vge {

class Ellipse {
    public:
        // Structure to hold ellipse parameters
        struct EllipseParams {
            float majorAxis;
            float minorAxis;
            float tiltAngle;
        };

        static inline EllipseParams innerEllipse = {
            1.0f,           // majorAxis
            0.8f,          // minorAxis
            M_PI / 6.0f    // tiltAngle
        };

        static inline EllipseParams outerEllipse = {
            2.0f,           // majorAxis
            1.6f,          // minorAxis
            M_PI / 3.0f    // tiltAngle
        };

        // Calculate point on ellipse given parameter t
        static glm::vec2 calculateEllipsePoint(float t, const EllipseParams& params) {
            float x = params.majorAxis * std::cos(t) * std::cos(params.tiltAngle) -
                    params.minorAxis * std::sin(t) * std::sin(params.tiltAngle);

            float y = params.majorAxis * std::cos(t) * std::sin(params.tiltAngle) +
                    params.minorAxis * std::sin(t) * std::cos(params.tiltAngle);

            return glm::vec2(x, y);
        }

        // Get points for both ellipses at once
        static std::pair<glm::vec2, glm::vec2> calculateBothEllipses(float t) {
            return {
                calculateEllipsePoint(t, innerEllipse),
                calculateEllipsePoint(t, outerEllipse)
            };
        }
    };

} // namespace
