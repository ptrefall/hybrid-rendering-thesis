#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, fTime, , ); 
rtBuffer<uchar4, 2>   out_buffer;

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

RT_PROGRAM void sampleTex()
{
	float2 zeroToOne = make_float2(launch_index) / make_float2(launch_dim);
	float2 minusOneToOne = -1.f + 2.f * zeroToOne;
	minusOneToOne *= 5;
	
	out_buffer[launch_index] = make_color( make_float3( minusOneToOne.x, minusOneToOne.y, sin( minusOneToOne.x*minusOneToOne.y+3.f*fTime ) ) );
}
