//#include <optix_world.h>

#include <optix/optix.h>
#include <optix/optix_math.h>

rtDeclareVariable(float3, bg_color, , );

struct PerRayData_radiance
{
  float3 result;
  float importance;
  int depth;
};

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );

RT_PROGRAM void miss()
{
  prd_radiance.result = bg_color;
}
