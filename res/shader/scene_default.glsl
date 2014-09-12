
#version 330 core

#shader vertex
#shader fragment

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

#ifdef _VERTEX_

layout(location = 0) in vec3 pos_m;

void main() {
	gl_Position = projection_matrix * modelview_matrix * vec4(pos_m, 1.0);
}

#endif

#ifdef _FRAGMENT_

out vec4 frag_color;

void main() {
	frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif
