#include <optix/optix.h>
#include <optix/optix_math.h>

struct PerRayData_radiance
{
  float3 result;
  float  importance;  // This is ignored in this sample.  See phong.h for use.
  int    depth;
};

struct PerRayData_shadow
{
  float attenuation;
};

// shading_normal is set by the closest hit intersection program 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

RT_PROGRAM void closest_hit_radiance()
{
	prd_radiance.result = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal))*0.5f + 0.5f;
}

RT_PROGRAM void any_hit_radiance()
{
	prd_shadow.attenuation = 0.f;
}