#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace vge {

class StarColor {
public:
    // Convert temperature to RGB color using blackbody radiation
    static glm::vec3 temperatureToColor(float temperature) {
        // Lowered temperature bounds
        const float MIN_TEMP = 1000.0f;  // Coolest stars (deep red)
        const float MAX_TEMP = 12000.0f;  // Hottest stars (blue-white) - lowered from 40000

        // Clamp temperature to valid range
        temperature = std::clamp(temperature, MIN_TEMP, MAX_TEMP);

        float r, g, b;

        // Modified temperature-color mapping for more pronounced colors
        if (temperature <= 4000.0f) {
            r = 1.0f;
            g = 0.2f + 0.6f * (temperature - 1000.0f) / 3000.0f;
            b = 0.1f * (temperature - 1000.0f) / 3000.0f;
        } else if (temperature <= 7000.0f) {
            r = 1.0f;
            g = 0.8f + 0.2f * (temperature - 4000.0f) / 3000.0f;
            b = 0.3f + 0.4f * (temperature - 4000.0f) / 3000.0f;
        } else {
            r = 1.0f - 0.4f * (temperature - 7000.0f) / 5000.0f;
            g = 1.0f - 0.2f * (temperature - 7000.0f) / 5000.0f;
            b = 0.7f + 0.3f * (temperature - 7000.0f) / 5000.0f;
        }

        // Clamp values
        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        // Adjust intensity but keep it higher for visibility
        float intensity = std::clamp(0.7f + 0.3f * temperature / MAX_TEMP, 0.7f, 1.0f);
        return glm::vec3(r, g, b) * intensity;
    }

    // Generate a random temperature with more emphasis on cooler stars
    static float generateRandomTemperature(float seed) {
        float rand = std::fmod(std::abs(std::sin(seed * 12345.6789f)), 1.0f);

        // Lower temperature ranges
        const float temp_ranges[] = {
            2000.0f,   // Deep red
            3000.0f,   // Red
            4000.0f,   // Orange-Red
            5000.0f,   // Yellow-Orange
            6000.0f,   // Yellow (Sun-like)
            8000.0f,   // White-Yellow
            12000.0f   // Blue-White
        };

        // Make distribution heavily favor cooler stars
        rand = std::pow(rand, 3.0f);  // Increased power for more bias towards lower temps

        const int num_ranges = sizeof(temp_ranges) / sizeof(float);
        float index = rand * (num_ranges - 1);
        int lower_idx = static_cast<int>(index);
        int upper_idx = std::min(lower_idx + 1, num_ranges - 1);
        float fraction = index - lower_idx;

        return temp_ranges[lower_idx] + fraction * (temp_ranges[upper_idx] - temp_ranges[lower_idx]);
    }
};

} // namespace vge
