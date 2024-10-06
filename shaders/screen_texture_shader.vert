#version 450

layout(location = 0) out vec2 fragTexCoord;

const vec2 screen_corners[6] = vec2[](
	vec2( 1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0)
);

layout(push_constant) uniform Push {
	vec2 screen_extent;
} push;

void main() {
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.5, 1.0);
	fragTexCoord = 0.5 * (vec2(1.0) + pos) * push.screen_extent;
}