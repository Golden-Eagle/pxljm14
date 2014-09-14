
#version 330 core

#ifndef MAX_LIGHTS
#define MAX_LIGHTS 1000
#endif

struct Light {
	vec3 pos;
	vec3 intensity;
};

layout(std140) uniform LightsBlock {
	Light lights[MAX_LIGHTS];
};

uniform uint num_lights;

vec3 scene_radiance(vec3 p, vec3 n, vec3 kd, vec3 ks) {
	
	vec3 l = vec3(0.0);
	
	for (uint i = 0u; i < num_lights; i++) {
		Light light = lights[i];
		float d = distance(light.pos, p);

		vec3 a = abs(dot(normalize(light.pos - p), n)) * light.intensity / pow(d, 2.0);

		// TODO physically correct, scale for kd is arbitrary

		l += kd * a * 0.05;

		l += ks * a;

	}

	return l;
}