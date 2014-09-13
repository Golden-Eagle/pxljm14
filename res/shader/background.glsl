
#version 330 core

#include "fullscreen.glsl"

uniform float ratio;
uniform sampler2D sampler_bg;

#ifdef _FRAGMENT_

out vec4 frag_color;

void main() {
	
	vec2 f = vec2(textureSize(sampler_bg, 0));
	float tr = f.x / f.y;

	vec2 tc = texCoord - 0.5;
	tc /= vec2(1.0 / min(1.0, ratio), max(1.0, ratio));
	tc /= vec2(max(1.0, tr), 1.0 / min(1.0, tr));
	tc += 0.5;

	frag_color = vec4(texture(sampler_bg, tc).rgb, 1.0);

	gl_FragDepth = 1.0;
	
}

#endif