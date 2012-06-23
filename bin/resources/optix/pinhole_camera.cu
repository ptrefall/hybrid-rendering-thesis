

#include <Optix/optix_world.h>
#include "helpers.h"

using namespace optix;

struct PerRayData_radiance
{
  float3 result;
  float  importance;
  int    depth;
};

struct PerRayData_shadow
{
  float attenuation;
};

rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float,         scene_epsilon, , );

//rtBuffer<uchar4, 2>              output_buffer;
rtBuffer<uchar4, 2>   g_buffer_diffuse;
rtBuffer<float4, 2>   g_buffer_position;
rtBuffer<float4, 2>   g_buffer_normal;

rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(unsigned int,  radiance_ray_type, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, time_view_scale, , ) = 1e-6f;
rtDeclareVariable(float, fTime, , ); 
//#define TIME_VIEW


RT_PROGRAM void pinhole_camera()
{
#ifdef TIME_VIEW
  clock_t t0 = clock(); 
#endif
  float2 d = make_float2(launch_index) / make_float2(launch_dim) * 2.f - 1.f;
  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);
  
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData_radiance prd;
  prd.importance = 1.f;
  prd.depth = 0;

  rtTrace(top_object, ray, prd);

  //g_buffer_diffuse[launch_index] = make_color( prd.result );

	//g_buffer_diffuse[launch_index].x = g_buffer_diffuse[launch_index].x;
	//g_buffer_diffuse[launch_index].y = g_buffer_diffuse[launch_index].y;
	//g_buffer_diffuse[launch_index].z = g_buffer_diffuse[launch_index].z;
	//g_buffer_diffuse[launch_index].w = g_buffer_diffuse[launch_index].w;

	//g_buffer_position[launch_index].x = g_buffer_position[launch_index].x;
	//g_buffer_position[launch_index].y = g_buffer_position[launch_index].y;
	//g_buffer_position[launch_index].z = g_buffer_position[launch_index].z;
	//g_buffer_position[launch_index].w = g_buffer_position[launch_index].w;

	//g_buffer_normal[launch_index].x = g_buffer_normal[launch_index].x;
	//g_buffer_normal[launch_index].y = g_buffer_normal[launch_index].y;
	//g_buffer_normal[launch_index].z = g_buffer_normal[launch_index].z;
	//g_buffer_normal[launch_index].w = g_buffer_normal[launch_index].w;
}

//RT_PROGRAM void exception()
//{
//  const unsigned int code = rtGetExceptionCode();
//  rtPrintf( "Caught exception 0x%X at launch index (%d,%d)\n", code, launch_index.x, launch_index.y );
//  g_buffer_diffuse[launch_index] = make_color( bad_color );
//}
