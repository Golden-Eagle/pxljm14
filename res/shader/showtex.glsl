/*
 *
 * Albireo: shader program to display a (2D) texture
 *
 */

#version 330

#shader fragment

#include "fullscreen.glsl"

// fragment shader
#ifdef _FRAGMENT_

uniform sampler2D sampler_tex;

out vec4 frag_color;

void main() {
	frag_color = vec4(texture(sampler_tex, texCoord).rgb, 1.0);
}

#endif