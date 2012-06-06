#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, fTime, , ); 
rtBuffer<float4, 2>   out_buffer;


RT_PROGRAM void sampleTex()
{
	float2 zeroToOne = make_float2(launch_index) / make_float2(launch_dim);
	float2 minusOneToOne = -1.f + 2.f * zeroToOne;
	minusOneToOne *= 5;
	
	out_buffer[launch_index] = make_float4( minusOneToOne.x, minusOneToOne.y, sin( minusOneToOne.x*minusOneToOne.y+3.f*fTime ), 0.f );
}
