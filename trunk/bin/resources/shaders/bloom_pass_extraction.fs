#version 330 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3
#define FRAG_COLOR	0

uniform sampler2D TEX_DIFF; 

in block
{
	vec2 t; //TexCoord
} Vertex;

layout(location = FRAG_COLOR, index = 0) out vec4 out_FragColor;

vec4 extractHighlights() 
{ 
	float fThreshold = 0.75;
	vec4 c = texture(TEX_DIFF, Vertex.t); 
	return clamp((c - fThreshold) / (1.0 - fThreshold), 0.0, 1.0);
}

void main()
{
    out_FragColor = extractHighlights();
}