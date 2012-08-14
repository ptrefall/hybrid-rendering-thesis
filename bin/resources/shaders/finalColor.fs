#version 330 core
#define DIFFUSE		0
#define POSITION	1
#define NORMAL		2
#define TEXCOORD	3
#define FRAG_COLOR	0

uniform sampler2D TEX_DIFF;
uniform sampler2D TEX_POS;
uniform sampler2D TEX_NORM;
uniform sampler2D TEX_RAY;

uniform vec3 ambient_mat[16];
uniform vec3 diffuse_mat[16];
uniform vec3 specular_mat[16];
uniform vec3 pp_t_ior_mat[16];

uniform struct SLight
{
	vec3 position_vs;			//view space
} light[1];

uniform vec3 CamPos_vs;			//view space

in block
{
	vec2 t; //TexCoord
} Vertex;

layout(location = FRAG_COLOR, index = 0) out vec4 out_FragColor;

float compute_phong_term(in vec3 N, in vec3 L, in vec3 V, in float NdotL, in float shininess)
{
	vec3 R = reflect(N, -L);
	float term = max(clamp(dot(V,R), 0.0, 1.0), 0.0);
	term = pow(term, shininess);
	return term;
}

float compute_blinn_term(in vec3 N, in vec3 L, in vec3 V, in float NdotL, in float shininess)
{
	vec3 H = normalize(L + V);
	float term = dot(N,H);
	term = max(clamp(term, 0.0, 1.0), 0.0);
	term = pow(term, shininess);
	return term;
}

float compute_gauss_term(in vec3 N, in vec3 L, in vec3 V, in float NdotL, in float shininess)
{
	vec3 H = normalize(L+V);
	float ANH = acos(dot(N,H));
	float exponent = ANH / shininess;
	exponent = -(exponent*exponent);
	float term = max(exp(exponent), 0.0);
	return term;
}

void main( void )
{
	vec3 diffuse 	= texture( TEX_DIFF, Vertex.t ).xyz;
	vec3 position_vs 	= texture( TEX_POS,  Vertex.t ).xyz;							//view space
	vec4 normal_matid = texture( TEX_NORM, Vertex.t );
	int material_id = int(normal_matid.a);
	vec4 ray = texture( TEX_RAY, Vertex.t );
	
	//vec3 N = -normalize(cross(dFdy(position_vs), dFdx(position_vs)));	//view space
	//vec3 N = normalize(normal_matid.xyz);								//view space
	vec3 N = normalize(ray.xyz * 2.0 - 1.0);										//object space???
	vec3 light_pos_vs = light[0].position_vs;							//view space
	vec3 L = normalize(light_pos_vs - position_vs);						//view space
	vec3 V = normalize(CamPos_vs-position_vs);							//view space
	
	float NdotL = max(dot(N,L), 0.0);
	float shininess = pp_t_ior_mat[material_id].r;
	float term = compute_gauss_term(N, L, V, NdotL, shininess);
	
	float shadow_att = texture( TEX_DIFF, Vertex.t ).a; // from Optix
    
	//out_FragColor = vec4(diffuse, 1.0);
	//out_FragColor = vec4(-position_vs.zzz*0.1, 1.0);
	//out_FragColor = vec4(N * 0.5 - 0.5, 1.0);
    out_FragColor = vec4(ray.rgb* 2.0 - 1.0,1.0);
    
	/*out_FragColor = vec4( 
		((diffuse * diffuse_mat[material_id] * NdotL) + (specular_mat[material_id] * term) + (diffuse * ambient_mat[material_id])),// * ray.r, 
		1.0
		);
		//+ ray.rgb*/
		
	//out_FragColor = vec4( diffuse, 1.0 );
	//out_FragColor = vec4( N, 1.0 );
	//out_FragColor = vec4( (N + 1.0) * 0.5, 1.0 );
	//out_FragColor = vec4( abs(N), 1.0 );
	//out_FragColor = vec4(((N + 1.0) * 0.5).rgb, 1.0);
	//out_FragColor = normal_matid;
}
