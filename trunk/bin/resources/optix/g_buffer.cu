
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
#include "optixu/optixu_math_namespace.h"

using namespace optix;

struct PerRayData_tex
{
  float3 diffuse;
  float3 position;
  float3 normal_matid;
};

rtTextureSampler<float4, 2> raster_diffuse_tex;
rtTextureSampler<float4, 2> raster_position_tex;
rtTextureSampler<float4, 2> raster_normal_tex;
rtDeclareVariable(PerRayData_tex, prd, rtPayload, );
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 

RT_PROGRAM void closest_hit_radiance()
{
  const float3 uv = texcoord;

  prd.diffuse = make_float3( tex2D( raster_diffuse_tex, uv.x, uv.y ) );
  prd.position = make_float3( tex2D( raster_position_tex, uv.x, uv.y ) );
  prd.normal_matid = make_float3( tex2D( raster_normal_tex, uv.x, uv.y ) );
}
