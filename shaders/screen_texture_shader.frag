#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler2D inputTexture;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(inputTexture, fragTexCoord);
}