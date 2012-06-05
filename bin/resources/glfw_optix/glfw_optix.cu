
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <Optix/optix.h>
#include <Optix/optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, fTime, , ); 
rtBuffer<float4, 2>   out_buffer;
rtTextureSampler<float4, 2> tex;

RT_PROGRAM void sampleTex()
{
	float2 zeroToOne = make_float2(launch_index) / make_float2(launch_dim);
	//float2 minusOneToOne = make_float2( -1.f + 2.f * zeroToOne.x, -1.f + 2.f * zeroToOne.y );
	float2 minusOneToOne = -1.f + 2.f * zeroToOne;
	minusOneToOne *= 5;
	//out_buffer[launch_index] = tex2D( tex, zeroToOne.x, zeroToOne.y );
	
	//out_buffer[launch_index] = tex2D( tex, zeroToOne.x, zeroToOne.y );

	out_buffer[launch_index] = make_float4( minusOneToOne.x, minusOneToOne.y, sin( minusOneToOne.x*minusOneToOne.y+3.f*fTime ), 0.f );

	out_buffer[launch_index].x = 255 - out_buffer[launch_index].x;
	out_buffer[launch_index].y = 255 - out_buffer[launch_index].y;
	out_buffer[launch_index].z = 255 - out_buffer[launch_index].z;
	out_buffer[launch_index].w = 255;
}
