
#version 330 core

#shader vertex
#shader fragment

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

uniform sampler2D sampler_thing;

#ifdef _VERTEX_

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

out VertexData {
	vec3 pos_v;
	vec2 uv;
} vertex_out;

void main() {
	vec3 pos_v = (modelview_matrix * vec4(pos, 1.0)).xyz;
	gl_Position = projection_matrix * vec4(pos_v, 1.0);
	vertex_out.pos_v = pos_v;
	vertex_out.uv = uv;
}

#endif

#ifdef _FRAGMENT_

in VertexData {
	vec3 pos_v;
	vec2 uv;
} vertex_in;

out vec4 frag_color;

void main() {
	frag_color = texture(sampler_thing, vertex_in.uv);
}

#endif
