#version 420 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3

in block
{
	vec3 v; //Position in view space
	vec3 n; //Normal in world space
} Vertex;

layout(location = DIFFUSE, 	index = 0) 	out vec4 out_Diffuse;
layout(location = POSITION, index = 0) 	out vec4 out_Position;
layout(location = NORMAL, 	index = 0) 	out vec4 out_Normal;

void main( void )
{
	out_Diffuse		= vec4(1.0, 0.0, 0.0, 1.0);
	out_Position	= vec4(Vertex.v.xyz,0);
	out_Normal		= vec4(Vertex.n.xyz,0);
}
