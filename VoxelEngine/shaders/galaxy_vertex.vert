#version 450

layout(location = 0) in vec3 inPosition;

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
    gl_Position = ubo.projection * ubo.view * worldPosition;

    // Calculate size based on time and position
    float basePulseSize = 20.0;
    float pulseIntensity = sin(push.time * 2.0) * 0.3 + 1.0; // Pulsing effect
    gl_PointSize = basePulseSize * pulseIntensity;
}
