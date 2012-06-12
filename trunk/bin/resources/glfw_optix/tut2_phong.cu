#include <optix/optix.h>
#include <optix/optix_math.h>

#include "commonStructs.h"

struct PerRayData_radiance
{
  float3 result;
  float importance;
  int depth;
};

rtDeclareVariable(float3,              geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3,              shading_normal,   attribute shading_normal, ); 
rtDeclareVariable(PerRayData_radiance, prd_radiance,     rtPayload, );
rtDeclareVariable(optix::Ray,          ray,              rtCurrentRay, );
rtDeclareVariable(float,               t_hit,            rtIntersectionDistance, );

rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float3, Ks, , );
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float,  phong_exp, , );
rtDeclareVariable(float3, ambient_light_color, , );
rtBuffer<BasicLight> lights; 

RT_PROGRAM void closest_hit_radiance()
{
	float3 world_geo_normal = normalize(rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ));
	float3 world_shade_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal ));
	float3 ffnormal = faceforward(world_shade_normal, -ray.direction, world_geo_normal); // -N or +N
	float3 color = Ka * ambient_light_color;
	float3 hit_point = ray.origin + t_hit * ray.direction;

	for(int i = 0; i < lights.size(); ++i) {
		BasicLight light = lights[i];
		float3 L = normalize(light.pos - hit_point);
		float nDl = dot( ffnormal, L);
		if( nDl > 0.f ) {
			float3 Lc = light.color;
			color += Kd * nDl * Lc;
			float3 H = normalize( L-ray.direction );
			float nDh = dot( ffnormal, H );
			if ( nDh > 0.f ) {
				color += Ks * Lc * pow(nDh, phong_exp);
			}
		}
	}
	prd_radiance.result = color;
}