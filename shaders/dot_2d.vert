#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in float size;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = vec4(position, 1.0);
    gl_PointSize = size;
    fragColor = color;
}
