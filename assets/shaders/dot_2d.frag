#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uvCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 localPos = uvCoord * 2.0 - 1.0;
    float dist = dot(localPos, localPos);
    if (dist > 1.0) {
        discard;
    }
    outColor = vec4(fragColor, 1.0);
}
