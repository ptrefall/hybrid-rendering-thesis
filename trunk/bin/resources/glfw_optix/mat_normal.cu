#include <optix/optix.h>
#include <optix/optix_math.h>
// Used by all the material cuda files
#include "commonStructs.h"

struct PerRayData_radiance
{
  float3 result;
  float  importance;  // This is ignored in this sample.  See phong.h for use.
  int    depth;
};

struct PerRayData_shadow
{
  float3 attenuation;
};

// shading_normal is set by the closest hit intersection program 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

//
// Returns a solid color as the shading result 
// 
RT_PROGRAM void closest_hit_radiance()
{
  prd_radiance.result = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal))*0.5f + 0.5f;
}
