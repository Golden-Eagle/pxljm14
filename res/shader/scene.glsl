
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
	

}