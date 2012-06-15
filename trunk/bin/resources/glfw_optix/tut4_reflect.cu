#include <optix/optix.h>
#include <optix/optix_math.h>

#include "commonStructs.h"

struct PerRayData_radiance
{
  float3 result;
  float importance;
  int depth;
};

struct PerRayData_shadow
{
  float attenuation;
};

rtDeclareVariable(float3,              geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3,              shading_normal,   attribute shading_normal, ); 
rtDeclareVariable(PerRayData_radiance, prd_radiance,     rtPayload, );
rtDeclareVariable(PerRayData_shadow,   prd_shadow,       rtPayload, );
rtDeclareVariable(optix::Ray,          ray,              rtCurrentRay, );
rtDeclareVariable(float,               t_hit,            rtIntersectionDistance, );


rtDeclareVariable(unsigned int,        radiance_ray_type, , );
rtDeclareVariable(unsigned int,        shadow_ray_type, , );
rtDeclareVariable(float,               scene_epsilon, , );
rtDeclareVariable(rtObject,            top_object, , );
rtDeclareVariable(rtObject,            top_shadower, , );

rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float3, Ks, , );
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float,  phong_exp, , );
rtDeclareVariable(float3, ambient_light_color, , );
rtBuffer<BasicLight> lights; 

// NEW 
rtDeclareVariable(float3, reflectivity, , );
//rtDeclareVariable(float, importance_cutoff, , );
rtDeclareVariable(int, max_depth, , );

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
			// cast shadow ray
			PerRayData_shadow prd_shadow;
			prd_shadow.attenuation = 1.0f;
			float Ldist = length(light.pos - hit_point);
			optix::Ray shadow_ray( hit_point, L, shadow_ray_type, scene_epsilon, Ldist );
			rtTrace(top_shadower, shadow_ray, prd_shadow);
			float light_att = prd_shadow.attenuation;

			if ( light_att > 0.0f ) {
				float3 Lc = light.color * light_att;
				color += Kd * nDl * Lc;

				float3 H = normalize( L-ray.direction );
				float nDh = dot( ffnormal, H );
				if ( nDh > 0 ) {
					color += Ks * Lc * pow(nDh, phong_exp);
				}
			}
		}
	}

	// Tracking a rays "importance" can improve performance by avoiding
	// creating reflection rays when a color is too dim to have any effect.
	// The luminance function computes a brightness value for the color
	// so we can compute its importance
	float importance = prd_radiance.importance * optix::luminance( reflectivity );

	// reflection ray
	float importance_cutoff = 0.01f;
	if ( importance > importance_cutoff && prd_radiance.depth < max_depth )
	{
		PerRayData_radiance prd_reflect;
		prd_reflect.importance = importance;
		prd_reflect.depth = prd_radiance.depth+1;
		float3 R = reflect( ray.direction, ffnormal );
		optix::Ray refl_ray( hit_point, R, radiance_ray_type, scene_epsilon );
		rtTrace( top_object, refl_ray, prd_reflect );
		color += reflectivity * prd_reflect.result;
	}

	prd_radiance.result = color;
}

RT_PROGRAM void any_hit_shadow()
{
	prd_shadow.attenuation = 0.0f; // fully attenuate on opaque...
	rtTerminateRay();
}