#version 450

#define M_PI 3.1415926535897932384626433832795

layout(push_constant) uniform Push {
	vec2 screenExtent;
} push;

layout(set = 0, binding = 0) uniform sampler2D inputTexture;

layout(binding = 1) buffer Particles {
	uint numParticles;
	float smoothRadius;
	float targetDensity;
	float dataScale;
	uint isNeighborViewActive;
	uint isDensityViewActive;
	vec2 data[];
};

layout(binding = 2) buffer Neighbors {
	int neighborIndex[];
};

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const float particleRadiusSqr = 4.0 * 4.0;

const vec4 colorLow = vec4(0.078, 0.282, 0.627, 1.0);
const vec4 colorMed = vec4(0.322, 0.984, 0.576, 1.0);
const vec4 colorMedHigh = vec4(0.980, 0.925, 0.027, 1.0);
const vec4 colorHigh = vec4(0.941, 0.290, 0.047, 1.0);

const vec4 upColor = vec4(0.996, 0.267, 0.412, 1.0);
const vec4 downColor = vec4(0.435, 0.525, 0.984, 1.0);
const vec4 leftColor = vec4(0.984, 0.851, 0.353, 1.0);
const vec4 rightColor = vec4(0.400, 0.851, 0.549, 1.0);

const float maxDisplayVelocityMag = 200.0;
const float maxDisplayVelocityMagSqr = maxDisplayVelocityMag * maxDisplayVelocityMag;
const float medHighVelocityMagSqr = maxDisplayVelocityMagSqr / 4.0 * 3.0;
const float medVelocityMagSqr = maxDisplayVelocityMagSqr / 2.0;

const vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

float kernelSpikyPow2_2D(float dist, float radius)
{
	if (dist >= radius)
		return 0.0;
	float v = radius - dist;
	return 6.f / (M_PI * pow(smoothRadius, 4.f)) * v * v;
}

float calculateDensity(vec2 samplePointPos) {
	float density = 0.0;
	samplePointPos *= dataScale;

	for (int i = 0; i < numParticles; i++) {
		vec2 particlePos = data[i];
		float dist = distance(samplePointPos, particlePos);
		float influence = kernelSpikyPow2_2D(dist, smoothRadius);
		density += influence;
	}

	return density;
}

vec4 fillParticleByVelocity(int particleIndex) {
	vec2 particleVelocity = data[particleIndex + numParticles] / dataScale;
	float velocityMagSqr = dot(particleVelocity, particleVelocity);
	vec2 velocityDir = normalize(particleVelocity);
	vec2 normalizedVelocity = particleVelocity;
	if (velocityMagSqr > maxDisplayVelocityMagSqr) {
		normalizedVelocity = velocityDir * maxDisplayVelocityMag;
	}

	// find the color based on the velocity magnitude and direction
	vec3 xColor, yColor;
	vec3 defaultColor = white.rgb * 0.01;

	float xIntensity = clamp(abs(normalizedVelocity.x) / maxDisplayVelocityMag, 0.0, 1.0);
	if (velocityDir.x > 0.0) {
		xColor = mix(defaultColor, leftColor.rgb, xIntensity);
	}
	else {
		xColor = mix(defaultColor, rightColor.rgb, xIntensity);
	}

	float yIntensity = clamp(abs(normalizedVelocity.y) / maxDisplayVelocityMag, 0.0, 1.0);
	if (velocityDir.y > 0.0) {
		yColor = mix(defaultColor, downColor.rgb, yIntensity);
	}
	else {
		yColor = mix(defaultColor, upColor.rgb, yIntensity);
	}

	float intensitySum = xIntensity + yIntensity;
	return vec4(clamp(xColor * xIntensity / intensitySum + yColor * yIntensity / intensitySum, 0.0, 1.0), 1.0);
}

vec4 applyNeighborView(int particleIndex) {
	if (particleIndex == 0) { // Draw the first particle in red
		return vec4(1.0, 0.0, 0.0, 1.0);
	}
	// check if current particle is the neighbor of the first particle
	for (int j = 0; j < numParticles; j++) {
		if (neighborIndex[j] == -1) {
			break;
		}
		if (neighborIndex[j] == particleIndex) {
			return vec4(1.0, 1.0, 1.0, 1.0);
		}
	}
	return fillParticleByVelocity(particleIndex);
}

vec4 applyDensityView() {
	float density = calculateDensity(fragTexCoord);
	float maxDisplayDensity = targetDensity * 1.25;
	float minDisplayDensity = targetDensity * 0.75;
	float medHighDensity = targetDensity * 1.125;
	density = min(density, maxDisplayDensity);
	vec4 bgColor;
	if (density < minDisplayDensity) {
		bgColor = mix(black, colorLow, density / minDisplayDensity);
	} else if (density < targetDensity) {
		bgColor = mix(colorLow, colorMed, (density - minDisplayDensity) / (targetDensity - minDisplayDensity));
	} else if (density < medHighDensity) {
		bgColor = mix(colorMed, colorMedHigh, (density - targetDensity) / (medHighDensity - targetDensity));
	} else {
		bgColor = mix(colorMedHigh, colorHigh, (density - medHighDensity) / (maxDisplayDensity - medHighDensity));
	}
	return bgColor;
}

void main() {
	if (isDensityViewActive == 1) {
		outColor = applyDensityView();
	}
	else {
		for (int i = 0; i < numParticles; i++) { // Draw all particles
			vec2 particlePosition = data[i] / dataScale;
			vec2 diff = fragTexCoord - particlePosition;
			float distanceSqr = dot(diff, diff);

			if (distanceSqr < particleRadiusSqr) { // If the pixel is inside the particle
				if (isNeighborViewActive == 1)
					outColor = applyNeighborView(i);
				else
					outColor = fillParticleByVelocity(i);
				return;
			}
	}

		outColor = black;
	}
}