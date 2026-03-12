#include "Common_PS.h"
#include "Utils.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbPostProcessObjectConsts : register( b2 )
{
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D   g_txColor       : register( t0 );
Texture2D   g_txDepth       : register( t1 );

SamplerState g_samWrapLinear : register( s0 );

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
    float depth = g_txDepth.Sample( g_samWrapLinear, Input.vTexcoord.xy ).r;
    float3 screenPos = float3( float2(Input.vTexcoord.x, 1.f - Input.vTexcoord.y) * 2.f - 1.f, depth );
    float3 viewPos = ScreenToView( screenPos, g_vProjParams );

    float3 OutColor = g_txColor.Sample( g_samWrapLinear, Input.vTexcoord ).rgb;

    OutColor = ApplyHDRCorrection( OutColor, 1.f / g_fLightPower );  // bring to LDR range

    return float4( OutColor, 1 );
}
