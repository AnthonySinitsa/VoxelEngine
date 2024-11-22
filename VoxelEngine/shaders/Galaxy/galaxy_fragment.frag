#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

// Gaussian function for smooth falloff
float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main() {
    // Create a circular point with soft edges
    vec2 centerDist = gl_PointCoord - vec2(0.5);
    float dist = length(centerDist);

    // Soft circle edge with gaussian falloff
    float sigma = 0.15; // Controls the softness of the edge
    float alpha = gaussian(dist, sigma);

    // Base color - slightly bluish white
    vec3 baseColor = vec3(0.9, 0.95, 1.0);

    // Add subtle color variation based on position
    float colorVar = gaussian(dist, 0.3);
    vec3 tintColor = mix(vec3(0.7, 0.8, 1.0), vec3(1.0, 0.95, 0.9), colorVar);

    // Combine colors and add glow
    vec3 finalColor = mix(baseColor, tintColor, 0.3);
    finalColor += vec3(0.2, 0.3, 0.4) * (1.0 - dist); // Add blue-ish glow

    // Adjust alpha for better blending
    float finalAlpha = alpha * 0.7; // Reduce overall opacity for better blending

    // Early discard for performance
    if (finalAlpha < 0.01) {
        discard;
    }

    outColor = vec4(finalColor, finalAlpha);
}
