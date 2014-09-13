
#version 330 core

#shader vertex
#shader fragment

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

uniform sampler2D sampler_atlas;

#ifdef _VERTEX_

layout(location = 0) in vec4 data1; //positionX, positionY, depth1, depth2
layout(location = 1) in uvec4 data2; //localpotiionX, localPositionY, texture location1, textureLocation2

out VertexData {
	vec3 pos_v;
	vec2 uv;
	flat uvec4 data2;
	flat uint id;
} vertex_out;

void main() {
	vec3 pos_v = (modelview_matrix * vec4(data1.xy + vec2(data2.xy), 0.0, 1.0)).xyz;
	gl_Position = projection_matrix * vec4(pos_v, 1.0);
	vertex_out.pos_v = pos_v;
	vertex_out.uv = data1.xy;
	vertex_out.data2 = data2;
	vertex_out.id = uint(gl_InstanceID);
}

#endif

#ifdef _FRAGMENT_

in VertexData {
	vec3 pos_v;
	vec2 uv;
	flat uvec4 data2;
	flat uint id;
} vertex_in;

out vec4 frag_color;

void main() {
	
	// TODO properly determine subtexture
	vec2 uvp;

	
	uvp.x = float(vertex_in.id % 4u + vertex_in.uv.x * 0.75 + 0.125) / 4.0;
	uvp.y = float(vertex_in.id / 4u + vertex_in.uv.y * 0.75 + 0.125) / 4.0;
	
	frag_color = texture(sampler_atlas, uvp).rgba;
}

#endif
