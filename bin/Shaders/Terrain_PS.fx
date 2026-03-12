#include "Common_PS.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbTerrainObjectConsts : register( b2 )
{
    float   g_fTile                 : packoffset( c0.x );
    float   g_fNoiseTile            : packoffset( c0.y );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txColorMap           : register( t0 );
Texture2D    g_txDiffuseBase        : register( t1 );
Texture2D    g_txDiffuse            : register( t2 );
Texture2D    g_txNormalMapNoise     : register( t3 );
Texture2D    g_txAlphaNoise         : register( t4 );

SamplerState g_samWrapAniso         : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float3  vNormal                 : NORMAL;
    float3  vTexcoord0              : TEXCOORD0;
    float2  vTexcoord1              : TEXCOORD1;
    float3  vWorldPos               : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float3 ComputeTerrainNormal( float2 uvPerPatch, float noiseTile, float3 terrainNormal )
{
    float3 noiseNormal = g_txNormalMapNoise.Sample( g_samWrapAniso, uvPerPatch * noiseTile ).xzy;   // z and y swapped to get it into our coord space
    noiseNormal = noiseNormal * 2.f - 1.f;
    noiseNormal.z = -noiseNormal.z;
    noiseNormal.y += 2.f;   // attenuate normals

    noiseNormal = normalize( noiseNormal );

    float3 tangent = float3(1.f, 0.f, 0.f);
    float3 normal = normalize( terrainNormal );
    float3 binormal = cross(tangent, normal );
    float3x3 tangentSpace = { tangent, normal, binormal };

    return normalize( mul( noiseNormal, tangentSpace ) );
}

float3 ComputeTerrainColor( float2 uvPerPatch, float2 uvPerWorld, float tile )
{
    float4 colorMap = g_txColorMap.Sample( g_samWrapAniso, uvPerWorld.xy );

    float range = 0.5f;
    float alphaNoiseTile = 8;
    float alpha = colorMap.a;
    float alphaNoise = g_txAlphaNoise.Sample( g_samWrapAniso, uvPerWorld.xy * alphaNoiseTile ).r;
    alphaNoise *= (1.f - range);
    alpha = saturate( (alpha - alphaNoise) / range );


    float vegetationNoise = g_txDiffuse.Sample( g_samWrapAniso, uvPerPatch * tile ).r;
    float noiseBase = g_txDiffuseBase.Sample( g_samWrapAniso, uvPerPatch * tile ).r;

    float3 outColor;
    float noise = lerp( noiseBase, vegetationNoise, alpha );
    noise = noise * 2.f - 1.f;
    outColor = colorMap.rgb + noise;

    return outColor;
}

float4 main( PS_INPUT Input ) : SV_TARGET
{
    LightParams lightParams = FillLightParams( g_vLightDir, g_vLightColor, g_fLightPower, g_vAmbientColor, g_fAmbientPower );

    float2 uvPerPatch = Input.vTexcoord0.xy;
    float2 uvPerWorld = Input.vTexcoord1.xy;

    MaterialParams materialParams;
    materialParams.albedo = ComputeTerrainColor( uvPerPatch, uvPerWorld, g_fTile );
    materialParams.roughness = 0.66f; // default roughness
    materialParams.metalness = 0.f;
    materialParams.ambientOcclusion = 1.f;  // no ambient occlusion
    materialParams.normal = ComputeTerrainNormal( uvPerPatch, g_fNoiseTile, Input.vNormal );

    float3 viewDir = normalize( g_vCameraPos - Input.vWorldPos );
    // float3 OutColor = PhongLighting( lightParams, materialParams, viewDir );
    float3 OutColor = BlinnPhongLighting( lightParams, materialParams , viewDir );
    // float3 OutColor = CookTorranceBRDFLighting( lightParams, materialParams , viewDir );

    OutColor.rgb = ApplyFog( OutColor.rgb, g_vFogColor.rgb, g_fFogLighPower, Input.vTexcoord0.z );

    return float4( OutColor, 1 );
}
