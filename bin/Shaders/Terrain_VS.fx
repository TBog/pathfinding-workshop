#include "Common_VS.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbTerrainObjectConsts : register( b2 )
{
    float2  g_patchStartPos         : packoffset( c0 );
    float   g_patchSize             : packoffset( c0.z );
    float4  g_heightMapInfo         : packoffset( c1 );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txHeightMap          : register( t0 );
Texture2D    g_txNormalMap          : register( t1 );

SamplerState g_samWrapLinear        : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float2  vPosition               : POSITION;
};

struct VS_OUTPUT
{
    float3  vNormal                 : NORMAL;
    float3  vTexcoord0              : TEXCOORD0;
    float2  vTexcoord1              : TEXCOORD1;
    float3  vWorldPos               : TEXCOORD2;
    float4  vPosition               : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
    VS_OUTPUT Output;

    float3 worldPos;
    worldPos.xz = g_patchStartPos + Input.vPosition * g_patchSize;
    
    float2 heightMapUV = ( worldPos.xz - g_heightMapInfo.xy ) * g_heightMapInfo.zz;
    heightMapUV.y = 1.f - heightMapUV.y;
    
    float3 terrainNormal = g_txNormalMap.SampleLevel( g_samWrapLinear, heightMapUV, 0 ).rgb * 2.f - 1.f;
    float terrainHeight = g_txHeightMap.SampleLevel( g_samWrapLinear, heightMapUV, 0 ).r;
    
    worldPos.y = terrainHeight;
    
    float viewDist = length( g_vCameraPos - worldPos );

    Output.vPosition = mul( float4( worldPos, 1.f ), g_mViewProjection );
    Output.vNormal = terrainNormal;
    Output.vTexcoord0.xy = Input.vPosition;
    Output.vTexcoord0.y = 1.f - Output.vTexcoord0.y;
    Output.vTexcoord0.z = ComputeFogFactor( viewDist, g_fFogStartZ, g_fInvFogSize );
    Output.vTexcoord1.xy = heightMapUV;
    
    Output.vWorldPos = worldPos;
    
    return Output;
}

