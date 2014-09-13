
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
	tc *= vec2(min(1.0, ratio / tr), min(1.0, tr / ratio));
	tc += 0.5;
	
	frag_color = texture(sampler_bg, tc);

	gl_FragDepth = 1.0;
	
}

#endif