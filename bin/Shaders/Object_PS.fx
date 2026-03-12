#include "Common_PS.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbObjectConsts : register( b2 )
{
};

cbuffer cbObjectMaterialConsts : register( b3 )
{
    float   g_fDiffuseTextureEnabled            : packoffset( c0.x );
    float   g_fNormalTextureEnabled             : packoffset( c0.y );
    float   g_fRoughnessTextureEnabled          : packoffset( c0.z );
    float   g_fAmbientOcclusionTextureEnabled   : packoffset( c0.w );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D   g_txDiffuse     : register( t0 );
Texture2D   g_txNormal      : register( t1 );
Texture2D   g_txRoughness   : register( t2 );
Texture2D   g_txAmbientOcclusion : register( t3 );

SamplerState g_samWrapLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vTexcoord    : TEXCOORD0;
    float3 vWorldPos    : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input ) : SV_TARGET
{
    LightParams lightParams = FillLightParams( g_vLightDir, g_vLightColor, g_fLightPower, g_vAmbientColor, g_fAmbientPower );

    float2 uv = Input.vTexcoord.xy;

    MaterialParams materialParams;
    if ( g_fDiffuseTextureEnabled == 1.f )
        materialParams.albedo = g_txDiffuse.Sample( g_samWrapLinear, uv ).rgb;
    else
        materialParams.albedo = 1.f;    // white

    if ( g_fRoughnessTextureEnabled == 1.f )
        materialParams.roughness = g_txRoughness.Sample( g_samWrapLinear, uv ).r;
    else
        materialParams.roughness = 0.66f; // default roughness

    materialParams.metalness = 0.f;

    if ( g_fAmbientOcclusionTextureEnabled == 1.f )
        materialParams.ambientOcclusion = g_txAmbientOcclusion.Sample( g_samWrapLinear, uv ).r;
    else
        materialParams.ambientOcclusion = 1.f;  // no ambient occlusion


    materialParams.normal = Input.vNormal;
    if ( g_fNormalTextureEnabled == 1.f )
    {
        materialParams.normal = ApplyNormalMap( g_txNormal, g_samWrapLinear, uv, Input.vNormal, Input.vTangent );
    }

    float3 viewDir = normalize( g_vCameraPos - Input.vWorldPos );
    // float3 OutColor = PhongLighting( lightParams, materialParams, viewDir );
    float3 OutColor = BlinnPhongLighting( lightParams, materialParams , viewDir );
    // float3 OutColor = CookTorranceBRDFLighting( lightParams, materialParams , viewDir );

    OutColor.rgb = ApplyFog( OutColor.rgb, g_vFogColor.rgb, g_fFogLighPower, Input.vTexcoord.z );

    return float4( OutColor, 1 );
}

