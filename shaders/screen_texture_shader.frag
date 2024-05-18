#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler2D inputTexture;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	vec2 screen_extent;
} push;

layout(binding = 3) buffer Particles {
	int num_particles;
	vec2 particle_positions[];
};

const float particle_radius_sqr = 4.0 * 4.0;
const vec4 particle_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 background_color = vec4(0.0, 0.0, 0.0, 1.0);

void main() {
	// Draw all particles
	outColor = background_color;
	for (int i = 0; i < num_particles; i++) {
		vec2 particle_position = particle_positions[i];
		vec2 diff = fragTexCoord - particle_position;
		float distance_sqr = dot(diff, diff);
		if (distance_sqr < particle_radius_sqr) {
			if (i == 18)
			{
				outColor = vec4(1.0, 0.0, 0.0, 1.0);
				return;
			}
			outColor = particle_color;
			return;
		}
	}
}