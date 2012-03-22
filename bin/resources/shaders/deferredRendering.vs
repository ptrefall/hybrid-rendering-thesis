#version 420 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3

layout(location = POSITION) in vec2 Position;

out gl_PerVertex
{
    vec4 gl_Position;
};

out block
{
	vec2 t; //TexCoord
} Vertex;

void main( void )
{
	vec2 madd = vec2(0.5,0.5);
	gl_Position = vec4(Position, 0.0, 1.0);
	Vertex.t = (Position * madd) + madd; // Scale to 0-1 range
}
