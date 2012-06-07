

#include "optix.h"
#include "helpers.h"
#include "optixu/optixu_math_namespace.h"

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );

rtBuffer<uchar4, 2>   g_buffer; // Map this to a PBO

//rtTextureSampler<float4, 2> raster_diffuse_tex;
//rtTextureSampler<float4, 2> raster_position_tex;
//rtTextureSampler<float4, 2> raster_normal_tex;

// Convert a float3 in [0,1)^3 to a uchar4 in [0,255]^4 -- 4th channel is set to 255
#ifdef __CUDACC__
__device__ __inline__ optix::uchar4 make_color(const optix::float3& c)
{
    return optix::make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                               static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                               static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                               255u);                                                 /* A */
}
#endif

RT_PROGRAM void gbuffer_compose()
{ 
	//g_buffer[launch_index].x = 255 - g_buffer[launch_index].x;
	//g_buffer[launch_index].y = 255 - g_buffer[launch_index].y;
	//g_buffer[launch_index].z = 255 - g_buffer[launch_index].z;
	//g_buffer[launch_index].w = 255;
	//float2 zeroToOne = make_float2(launch_index) / make_float2(launch_dim);
	//g_buffer[launch_index] = make_color( make_float3( zeroToOne.x, zeroToOne.y, 0.f ) );
	g_buffer[launch_index] = make_color( make_float3( 255.0f, 0.0f, 0.0f ) );

	/*
	float4 diffuse = ( tex2D( raster_diffuse_tex, uv.x, uv.y ) );
	float4 position = ( tex2D( raster_position_tex, uv.x, uv.y ) );
	float4 normal_matid = ( tex2D( raster_normal_tex, uv.x, uv.y ) );

	result_buffer[launch_index] = shade...
	*/
}

