#version 450

layout(location = 0) in float inSize;
layout(location = 0) out vec4 outColor;

void main() {
    vec2 centerDist = gl_PointCoord - vec2(0.5);
    if (length(centerDist) > 0.5) {
        discard;
    }
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
