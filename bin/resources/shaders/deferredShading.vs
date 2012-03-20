#version 420 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3

uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 N_WRI; // Move the normals back from the camera space to the world space

layout(location = POSITION) in vec4 Position;
layout(location = NORMAL) 	in vec3 Normal;

out gl_PerVertex
{
    vec4 gl_Position;
};

out Vertex
{
	vec3 v; //Position in view space
	vec3 n; //Normal in world space
};

void main( void )
{	
	gl_Position		= MVP * Position;
	Vertex.v		= vec4(MV * Position).xyz;
	Vertex.n		= normalize(N_WRI * Normal); //Normal in world space
}
