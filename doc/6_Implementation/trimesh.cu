
rtBuffer<float3>              vertex_buffer;     
rtBuffer<int3>                index_buffer;    // position indices

rtDeclareVariable(float3,     texcoord,         attribute texcoord, ); 
rtDeclareVariable(float3,     geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3,     shading_normal,   attribute shading_normal, ); 
rtDeclareVariable(int,        normal_offset,    , ); 
rtDeclareVariable(optix::Ray, ray,              rtCurrentRay, );

RT_PROGRAM void mesh_intersect( int primIdx )
{
	int3 v_idx = index_buffer[primIdx];

	float3 p0 = vertex_buffer[ v_idx.x ];
	float3 p1 = vertex_buffer[ v_idx.y ];
	float3 p2 = vertex_buffer[ v_idx.z ];

	// Intersect ray with triangle
	float3 n;
	float  t, beta, gamma;

	if( intersect_triangle( ray, p0, p1, p2, n, t, beta, gamma ) ) {
	if ( dot(ray.direction, n) < 0 ) // Don't render backfaces
	if(  rtPotentialIntersection( t ) ) {

	geometric_normal = normalize( n );

	if ( normal_offset <= 0 ) {
			shading_normal   = geometric_normal;
		} else {
			float3 n0 = vertex_buffer[ v_idx.x+normal_offset ];
			float3 n1 = vertex_buffer[ v_idx.y+normal_offset ];
			float3 n2 = vertex_buffer[ v_idx.z+normal_offset ];
			shading_normal = 
			normalize( n1*beta + n2*gamma + n0*(1.0f-beta-gamma) );
		}

		texcoord = make_float3( 0.0f, 0.0f, 0.0f );

		rtReportIntersection( 0 );
	}
	}
}
