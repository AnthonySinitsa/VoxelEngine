#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in float inSize;

layout(location = 0) out float outSize;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

void main() {
    gl_Position = push.modelMatrix * vec4(inPosition, 0.0, 1.0);
    gl_PointSize = inSize;
    outSize = inSize;
}
