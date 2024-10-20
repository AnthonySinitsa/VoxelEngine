#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

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
    vec4 positionWorld = push.modelMatrix * vec4(inPosition.x, inPosition.y, 0.0, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
    gl_PointSize = 10.0; // Increased point size for visibility

    fragColor = inColor;
}
