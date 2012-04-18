#version 420 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3
#define FRAG_COLOR	0

uniform sampler2D TEX_DIFF; 
uniform sampler2D TEX_POS;
uniform sampler2D TEX_NORM;

uniform vec3 CamPos;

in block
{
	vec2 t; //TexCoord
} Vertex;

layout(location = FRAG_COLOR, index = 0) out vec4 out_FragColor;

void main( void )
{
	vec4 diffuse 	= texture( TEX_DIFF, Vertex.t );
	vec3 position 	= texture( TEX_POS,  Vertex.t ).xyz;
	vec3 normal 	= normalize(texture( TEX_NORM, Vertex.t ).xyz);
	
	vec3 light = vec3(0,0,10);
	vec3 lightDir = normalize(light - position);
	
	vec3 eyeDir = normalize(CamPos-position);
	vec3 vHalfVector = normalize(lightDir+eyeDir);
	
	out_FragColor = max(dot(normal,lightDir),0) * diffuse + pow(max(dot(normal,vHalfVector),0.0), 100) * 1.5;
}
