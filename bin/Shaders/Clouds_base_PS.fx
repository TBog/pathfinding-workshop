#include "Common_PS.h"
#include "Clouds_utils.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbCloudsObjectConsts : register( b2 )
{
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float2 vTexcoord    : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 main( PS_INPUT Input ) : SV_TARGET
{
    float2 vUV = Input.vTexcoord.xy;
    float3 coord = frac( float3( vUV + float2( 0.2f, 0.62f), 0.5f ) );

    float4 OutColor = 1;

    float mfbm = 0.9f;
    float mvor = 0.7f;

    OutColor.r = lerp( 1.f, tilableFbm( coord, 7, 4 ), mfbm) * 
        lerp(1., tilableVoronoi( coord, 8, 9 ), mvor);
    OutColor.g = 0.625 * tilableVoronoi( coord + 0.f, 3, 15 ) +
        0.250 * tilableVoronoi(  coord + 0.f, 3, 19 ) +
        0.125 * tilableVoronoi( coord + 0.f, 3, 23 ) 
        -1.;
    OutColor.b = 1. - tilableVoronoi( coord + 0.5f, 6, 9 );

    return OutColor;
}
