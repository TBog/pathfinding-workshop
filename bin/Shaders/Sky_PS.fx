#include "Common_PS.h"
#include "Utils.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbSkyObjectConsts : register( b2 )
{
    float3  g_vSkyColor                 : packoffset( c0 );
    float   g_vSkyLightPower            : packoffset( c0.w );
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
    float3 screenPos = float3( float2(Input.vTexcoord.x, 1.f - Input.vTexcoord.y) * 2.f - 1.f, 1.f );
    float3 viewPos = ScreenToView( screenPos, g_vProjParams );
    float3 dir = mul( normalize( viewPos ), (float3x3)g_mInvView );     // direction in world
    
    float sundot = saturate( dot(dir, g_vLightDir) );

    float3 bottomExtraBlue = float3( 0, 0.1f, 0.2f );
    float3 col = g_vSkyColor - max( dir.y, 0.01f ) * max( dir.y, 0.01f ) * 0.5f;
    col = lerp( col, max(g_vFogColor - bottomExtraBlue, 0.f), pow( 1.f - max( dir.y, 0.f ), 6.f ) );

    col += 0.25f * g_vLightColor * float3( 1.f, 0.875f, 0.66f ) * pow( sundot, 5.f );
    col += 0.25f * g_vLightColor * pow( sundot, 64.f );
    col += 0.20f * g_vLightColor * pow( sundot, 512.f );

    col += saturate( (0.1f - dir.y) * 10.f ) * bottomExtraBlue;
    col += 0.2 * g_vLightColor * pow( sundot, 8.f );

    float3 OutColor = col * g_vSkyLightPower;

    return float4( OutColor, 1 );
}
