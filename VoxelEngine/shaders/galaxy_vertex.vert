#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in float inSize;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
} ubo;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
    gl_PointSize = inSize;

    // Calculate color based on position
    float t = (inPosition.x + 4.5) / 9.0; // Normalize x position to 0-1 range
    fragColor = vec3(
            1.0 - t, // Red decreases from left to right
            sin(t * 3.14159), // Green peaks in middle
            t // Blue increases from left to right
        );
}
