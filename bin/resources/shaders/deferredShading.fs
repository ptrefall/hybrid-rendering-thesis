#version 330 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3

uniform sampler2D diffuse_tex;

uniform float material_id;

in block
{
	vec4 position_ws;	//world space
	vec4 position_vs; 	//view space
	vec3 normal_vs; 	//view space
	vec2 texcoord;
} Vertex;

layout(location = DIFFUSE, 	index = 0) 	out vec4 out_Diffuse;
layout(location = POSITION, index = 0) 	out vec4 out_Position;
layout(location = NORMAL, 	index = 0) 	out vec4 out_Normal;

vec4 Diffuse();
vec3 Position();
vec3 Normal();

void main( void )
{
	out_Diffuse		= Diffuse();
	out_Position	= vec4(Position(), 1.0);
	out_Normal		= vec4(Normal(),material_id);
}

vec4 Diffuse()
{
	return texture(diffuse_tex, Vertex.texcoord);
}

vec3 Position()
{
	return Vertex.position_vs.xyz;
}

vec3 Normal()
{
	vec3 frontNormal_vs = gl_FrontFacing ? Vertex.normal_vs : -Vertex.normal_vs;	//view space
	return normalize(frontNormal_vs);
}
