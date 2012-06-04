
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

#include "optix.h"
#include "helpers.h"
#include "optixu/optixu_math_namespace.h"

using namespace optix;

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtBuffer<uchar4, 2>   g_buffer; // Map this to a PBO

rtTextureSampler<float4, 2> raster_diffuse_tex;
rtTextureSampler<float4, 2> raster_position_tex;
rtTextureSampler<float4, 2> raster_normal_tex;


RT_PROGRAM void gbuffer_compose()
{ 
	//g_buffer[launch_index].x = 255 - g_buffer[launch_index].x;
	//g_buffer[launch_index].y = 255 - g_buffer[launch_index].y;
	//g_buffer[launch_index].z = 255 - g_buffer[launch_index].z;
	//g_buffer[launch_index].w = 255;
	//float2 zeroToOne = make_float2(launch_index) / make_float2(launch_dim);
	//g_buffer[launch_index] = make_color( make_float3( zeroToOne.x, zeroToOne.y, 0.f ) );
	g_buffer[launch_index] = make_color( make_float3( 255.0f, 255.0f, 255.0f ) );
	//result_buffer[launch_index] = tex2D( tex, zeroToOne.x, zeroToOne.y );


	/*
	float4 diffuse = ( tex2D( raster_diffuse_tex, uv.x, uv.y ) );
	float4 position = ( tex2D( raster_position_tex, uv.x, uv.y ) );
	float4 normal_matid = ( tex2D( raster_normal_tex, uv.x, uv.y ) );

	result_buffer[launch_index] = shade...
	*/
}

