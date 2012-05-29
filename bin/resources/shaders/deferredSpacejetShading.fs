#version 330 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform sampler2D light_tex;

uniform float material_id;

in block
{
	vec4 position_ws;	//world space
	vec4 position_vs; 	//view space
	vec3 normal_vs; 	//view space
	vec3 tangent_vs;	//view space
	vec3 bitangent_vs;	//view space
	vec2 texcoord;
} Vertex;

layout(location = DIFFUSE, 	index = 0) 	out vec4 out_Diffuse;
layout(location = POSITION, index = 0) 	out vec4 out_Position;
layout(location = NORMAL, 	index = 0) 	out vec4 out_Normal;

vec4 Diffuse();
vec3 Position();
vec3 Normal();
float Light();

void main( void )
{	
	out_Diffuse		= Diffuse();
	out_Position	= vec4(Position(), Light());
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
	vec3 frontNormal_vs = gl_FrontFacing ? Vertex.normal_vs : -Vertex.normal_vs;			//view space
	vec3 frontTangent_vs = gl_FrontFacing ? Vertex.tangent_vs : -Vertex.tangent_vs;			//view space
	vec3 frontBitangent_vs = gl_FrontFacing ? Vertex.bitangent_vs : -Vertex.bitangent_vs;	//view space
	
	vec3 Normal_ts = texture(normal_tex, Vertex.texcoord).rgb * 2.0 - 1.0;					//tangent space
	vec3 Normal_vs = Normal_ts.x * frontTangent_vs - Normal_ts.y * frontBitangent_vs + Normal_ts.z * frontNormal_vs; //Tangent space to View space
	return normalize(Normal_vs);
}

float Light()
{
	return texture(light_tex, Vertex.texcoord).r;
}
