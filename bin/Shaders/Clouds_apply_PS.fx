#include "Common_PS.h"

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txTexture : register( t0 );

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
    float4 OutColor = g_txTexture.Sample( g_samWrapLinear, Input.vTexcoord.xy );

    OutColor.rgb *= g_fLightPower;

    return OutColor;
}
