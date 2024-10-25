#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in float inSize;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    float time;
} push;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
} ubo;

void main() {
    // Calculate initial angle based on star index
    float baseAngle = (inPosition.x + 4.5) / 9.0 * 2.0 * 3.14159;

    // Apply rotation based on time
    float rotationSpeed = 1.0; // One full rotation per second
    float angle = baseAngle + push.time * rotationSpeed;

    // Set radius of circle
    float radius = 2.0;

    // Calculate rotated position
    vec3 rotatedPosition = vec3(
            radius * cos(angle), // X coordinate
            radius * sin(angle), // Y coordinate
            0.0 // Z coordinate
        );

    vec4 worldPosition = push.modelMatrix * vec4(rotatedPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * worldPosition;
    gl_PointSize = inSize;

    // Color based on angle
    fragColor = vec3(
            cos(angle) * 0.5 + 0.5, // Red
            sin(angle) * 0.5 + 0.5, // Green
            sin(angle + 3.14159) * 0.5 + 0.5 // Blue
        );
}
