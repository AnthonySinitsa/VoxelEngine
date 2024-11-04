#version 450

layout(location = 0) in vec3 inPosition;

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
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0);
    vec4 viewPosition = ubo.view * worldPosition;
    gl_Position = ubo.projection * viewPosition;

    float distanceToCamera = length(viewPosition.xyz);
    float baseSize = 20.0;
    gl_PointSize = baseSize * (1.0 / distanceToCamera);
}
