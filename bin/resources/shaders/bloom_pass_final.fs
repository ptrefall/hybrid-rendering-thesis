#version 330 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3
#define FRAG_COLOR	0

uniform sampler2D TEX_DIFF;  
uniform sampler2D TEX_BLOOM; 

in block
{
	vec2 t; //TexCoord
} Vertex;

layout(location = FRAG_COLOR, index = 0) out vec4 out_FragColor;

vec4 adjustSaturation(in vec4 color, in float saturation)
{
	float grey = dot(color, vec4(vec3(0.3, 0.59, 0.11), 0.0));
	vec4 grey_color = vec4(grey, grey, grey, 0.0);
	return mix(grey_color, color, saturation);
}

vec4 bloomCombine()
{
	float bloomIntensity = 1.6;
	float bloomSaturation = 1.2;
    vec4 bloom = texture(TEX_BLOOM, Vertex.t);
    bloom = (adjustSaturation(bloom, bloomSaturation) * bloomIntensity);

	float baseIntensity = 1.2;
	float baseSaturation = 1.1;
    vec4 base = texture(TEX_DIFF, Vertex.t);
    base = (adjustSaturation(base, baseSaturation) * baseIntensity);
	
    base *= (1.0 - clamp(bloom, 0.0, 1.0));
    return (base + bloom) + 0.2;
}

void main()
{
    out_FragColor = bloomCombine();
}