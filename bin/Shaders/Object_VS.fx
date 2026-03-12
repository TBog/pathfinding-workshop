#include "Common_VS.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbObjectConsts : register( b2 )
{
    matrix  g_mWorld        : packoffset( c0 );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 vPosition    : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vColor       : COLOR;
    float2 vTexcoord    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vTexcoord    : TEXCOORD0;
    float3 vWorldPos    : TEXCOORD1;
    float4 vPosition    : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
    VS_OUTPUT Output;

    matrix mWorldViewProj = mul( g_mWorld, g_mViewProjection );

    float3 worldPos = mul( float4( Input.vPosition, 1.f ), g_mWorld ).xyz;
    float viewDist = length( g_vCameraPos - worldPos );

    Output.vPosition = mul( float4(Input.vPosition, 1.f), mWorldViewProj );
    Output.vNormal = mul( Input.vNormal, ( float3x3 )g_mWorld );
    Output.vTangent = mul( Input.vTangent, ( float3x3 )g_mWorld );
    Output.vTexcoord.xy = Input.vTexcoord;
    Output.vTexcoord.z = ComputeFogFactor( viewDist, g_fFogStartZ, g_fInvFogSize );
    Output.vWorldPos = worldPos;

    return Output;
}

