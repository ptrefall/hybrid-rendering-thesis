#version 330 core
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
	vec2 pos_norm = Position; // vertices go from 0,0 to 1,1
	Vertex.t = (pos_norm * madd) + madd; // Scale to 0-1 range
	
	//gl_Position = vec4( 2.0*Position - vec2(1.0,1.0) , 0.0, 1.0); // screen coords are -1 to +1
	gl_Position = vec4( Position - vec2(.5,.5) , 0.0, 1.0); // why does 1/2 size look like correct pixel size?
}
