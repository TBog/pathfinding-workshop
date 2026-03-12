#include "Common_VS.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbCubeConsts : register( b2 )
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
    float2 vTexcoord    : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float2 vTexcoord    : TEXCOORD0;
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

    Output.vPosition = mul( float4(Input.vPosition, 1.f), mWorldViewProj );
    Output.vNormal = mul( Input.vNormal, ( float3x3 )g_mWorld );
    Output.vTangent = mul( Input.vTangent, ( float3x3 )g_mWorld );
    Output.vTexcoord = Input.vTexcoord;
    Output.vWorldPos = mul( float4( Input.vPosition, 1.f ), g_mWorld ).xyz;

    return Output;
}

