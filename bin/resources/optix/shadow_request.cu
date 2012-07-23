#include <optix.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

//rtBuffer<uchar4, 2>   g_buffer_diffuse_read;
rtBuffer<float4, 2>   g_buffer_position_read;
//rtBuffer<float4, 2>   g_buffer_normal_read;

rtBuffer<uchar4, 2>   g_buffer_diffuse_write;
//rtBuffer<float4, 2>   g_buffer_position_write;
//rtBuffer<float4, 2>   g_buffer_normal_write;

rtDeclareVariable(uint, shadow_ray_type, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

//rtDeclareVariable(float3, light_pos, , );
rtDeclareVariable(rtObject, top_object, , );

struct PerRayData_shadow
{
  float attenuation;
};

RT_PROGRAM void shadow_request()
{
  //float3 ray_origin = make_float3(tex2D(g_buffer_position, launch_index.x, launch_index.y));
  float3 ray_origin = make_float3( g_buffer_position_read[launch_index] ) ; // xyz
  

  PerRayData_shadow prd_shadow;
  prd_shadow.attenuation = 1.0f;

  if( !isnan(ray_origin.x) ) {
	float3 light_pos = make_float3( 2.35f,2.00f,1.10f );
    float3 L = light_pos-ray_origin;
    float dist = sqrtf(dot(L,L));
    float3 ray_direction = L/dist;
    optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, shadow_ray_type, scene_epsilon, dist);
    rtTrace(top_object, ray, prd_shadow);
  }

 //g_buffer_diffuse_write[launch_index].x = static_cast<unsigned char>(prd_shadow.attenuation*255.99f);

  // G-buffer Position debugging with limited range...
  //g_buffer_diffuse_write[launch_index].x = static_cast<unsigned char>(ray_origin.x*255.99f);
  //g_buffer_diffuse_write[launch_index].y = static_cast<unsigned char>(ray_origin.y*255.99f);
  //g_buffer_diffuse_write[launch_index].z = static_cast<unsigned char>(-ray_origin.z*255.99f);
}