#version 330 core

#define VERTEX		0
#define NORMAL	 	1
#define TEXCOORD 	2
#define FRAG_COLOR	0

layout(location = VERTEX) in vec3 Vertex;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;

uniform mat4 mvp;

void main()
{
	gl_Position = vec4(Vertex, 0.0);
}