#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    float time;
} push;

void main() {
    // Create a circular point with soft edges
    vec2 centerDist = gl_PointCoord - vec2(0.5);
    float dist = length(centerDist);

    // Soft circle edge
    float alpha = 1.0 - smoothstep(0.45, 0.5, dist);

    // Calculate color based on time and position
    vec3 baseColor = vec3(
            sin(push.time) * 0.5 + 0.5,
            cos(push.time * 0.7) * 0.5 + 0.5,
            sin(push.time * 1.3) * 0.5 + 0.5
        );

    // Add shimmer effect
    float shimmer = sin(push.time * 10.0 + dist * 20.0) * 0.1 + 0.9;

    // Add glow effect based on distance from center
    vec3 glowColor = mix(baseColor * 1.5, baseColor * shimmer, dist * 2.0);

    // Discard pixels outside the circle
    if (dist > 0.5) {
        discard;
    }

    outColor = vec4(glowColor, alpha);
}
