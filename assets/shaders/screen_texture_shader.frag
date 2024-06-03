#version 450

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler2D inputTexture;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	vec2 screenExtent;
} push;

layout(binding = 3) buffer Particles {
	uint numParticles;
	float smoothRadius;
	float targetDensity;
	vec2 data[];
};

// struct ParticleData
// {
// 	unsigned int numParticles;
// 	std::vector<glm::vec2> positions;
// 	std::vector<glm::vec2> velocities;
// };

const float particleRadiusSqr = 4.0 * 4.0;

const vec4 particleColorSlow = vec4(0.078, 0.282, 0.627, 1.0);
const vec4 particleColorMedium = vec4(0.322, 0.984, 0.576, 1.0);
const vec4 particleColorMedHigh = vec4(0.980, 0.925, 0.027, 1.0);
const vec4 particleColorFast = vec4(0.941, 0.290, 0.047, 1.0);

const float maxDisplayVelocityMagSqr = 100.0 * 100.0;
const float medHighVelocityMagSqr = maxDisplayVelocityMagSqr / 4.0 * 3.0;
const float medVelocityMagSqr = maxDisplayVelocityMagSqr / 2.0;


const vec4 bgColorLowDensity = vec4(0.0, 0.0, 1.0, 1.0);
const vec4 bgColorTargetDensity = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 bgColorHighDensity = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 bgColorDefault = vec4(0.0, 0.0, 0.0, 1.0);

float smoothKernel(float radius, float dist) {
	float radiusSqr = radius * radius;
	float radius8 = radiusSqr * radiusSqr * radiusSqr * radiusSqr;
	float volume = M_PI * radius8 / 4.0;
	float result = max(0, radiusSqr - dist * dist);
	return result * result * result / volume;
}

float calculateDensity(vec2 samplePointPos) {
	float density = 0.0;
	const float mass = 1.0;

	for (int i = 0; i < numParticles; i++) {
		vec2 particlePos = data[i];
		float dist = distance(samplePointPos, particlePos);
		float influence = smoothKernel(smoothRadius, dist);
		density += mass * influence;
	}

	return density;
}

void main() {
	// Draw all particles
	for (int i = 0; i < numParticles; i++) {
		vec2 particlePosition = data[i];
		vec2 diff = fragTexCoord - particlePosition;
		float distanceSqr = dot(diff, diff);

		if (distanceSqr < particleRadiusSqr) { // If the pixel is inside the particle
			vec2 particleVelocity = data[i + numParticles];
			float velocityMagSqr = dot(particleVelocity, particleVelocity);
			velocityMagSqr = min(velocityMagSqr, maxDisplayVelocityMagSqr);
			// find the color based on the velocity
			vec4 particleColor;
			if (velocityMagSqr < medVelocityMagSqr) {
				particleColor = mix(particleColorSlow, particleColorMedium, velocityMagSqr / medVelocityMagSqr);
			} else if (velocityMagSqr < medHighVelocityMagSqr) {
				particleColor = mix(particleColorMedium, particleColorMedHigh, (velocityMagSqr - medVelocityMagSqr) / (medHighVelocityMagSqr - medVelocityMagSqr));
			} else {
				particleColor = mix(particleColorMedHigh, particleColorFast, (velocityMagSqr - medHighVelocityMagSqr) / (maxDisplayVelocityMagSqr - medHighVelocityMagSqr));
			}
			outColor = particleColor;
			return;
		}
	}

#ifdef SHOW_DENSITY
	// calculate density
	float density = calculateDensity(fragTexCoord);
	float maxDisplayDensity = targetDensity * 2.0;
	density = min(density, maxDisplayDensity);
	// outColor = mix(bgColorLowDensity, bgColorHighDensity, density / maxDisplayDensity);
	if (density < targetDensity) {
		outColor = mix(bgColorLowDensity, bgColorTargetDensity, density / targetDensity);
	} else {
		outColor = mix(bgColorTargetDensity, bgColorHighDensity, (density - targetDensity) / targetDensity);
	}
#else
	outColor = bgColorDefault;
#endif
}