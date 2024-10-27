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
    // Calculate initial angle based on vertex position
    float baseAngle = atan(inPosition.y, inPosition.x);
    float radius = length(inPosition.xy);

    // Apply rotation based on time
    float rotationSpeed = 1.0;
    float angle = baseAngle + push.time * rotationSpeed;

    // Calculate rotated position
    vec3 rotatedPosition = vec3(
            radius * cos(angle),
            0.0,
            radius * sin(angle)
        );

    vec4 worldPosition = push.modelMatrix * vec4(rotatedPosition, 1.0);
    vec4 viewPosition = ubo.view * worldPosition;
    gl_Position = ubo.projection * viewPosition;

    // Calculaet perspective-correct point size
    float baseSize = 200.0f;
    // Static point size
    gl_PointSize = baseSize / viewPosition.z;
}
