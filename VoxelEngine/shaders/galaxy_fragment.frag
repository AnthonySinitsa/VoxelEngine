#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    // Create a circular point with soft edges
    vec2 centerDist = gl_PointCoord - vec2(0.5);
    float dist = length(centerDist);

    // Soft circle edge
    float alpha = 1.0 - smoothstep(0.45, 0.5, dist);

    // Add some internal glow effect based on distance from center
    vec3 glowColor = mix(fragColor * 1.5, fragColor, dist * 2.0);

    // Discard pixels outside the circle
    if (dist > 0.5) {
        discard;
    }

    outColor = vec4(glowColor, alpha);
}
