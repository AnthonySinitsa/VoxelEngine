#pragma once

#include <glm/glm.hpp>
#include <cmath>

namespace vge {

class Ellipse {
public:
    // Structure to hold ellipse parameters
    struct EllipseParams {
        float majorAxis;      // a value (2 for outer ellipse, 1 for inner)
        float minorAxis;      // b value (1.6 for outer ellipse, 0.8 for inner)
        float tiltAngle;      // phi value (pi/3 for outer ellipse, pi/6 for inner)
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
        // Inner ellipse parameters
        EllipseParams inner{
            1.0f,           // majorAxis
            0.8f,          // minorAxis
            M_PI / 6.0f    // tiltAngle (pi/6)
        };

        // Outer ellipse parameters
        EllipseParams outer{
            2.0f,           // majorAxis
            1.6f,          // minorAxis
            M_PI / 3.0f    // tiltAngle (pi/3)
        };

        return {
            calculateEllipsePoint(t, inner),
            calculateEllipsePoint(t, outer)
        };
    }
};

} // namespace
