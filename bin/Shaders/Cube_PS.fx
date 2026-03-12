#include "Common_PS.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbCubeConsts : register( b2 )
{
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D   g_txBaseColor   : register( t0 );
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
    float2 vTexcoord    : TEXCOORD0;
    float3 vWorldPos    : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input ) : SV_TARGET
{
    LightParams lightParams = FillLightParams( g_vLightDir, g_vLightColor, g_fLightPower, g_vAmbientColor, g_fAmbientPower );

    float2 uv = Input.vTexcoord;

    MaterialParams materialParams;
    materialParams.albedo = g_txBaseColor.Sample( g_samWrapLinear, uv ).rgb;
    materialParams.roughness = g_txRoughness.Sample( g_samWrapLinear, uv ).r;
    materialParams.metalness = 0.f;
    materialParams.ambientOcclusion = g_txAmbientOcclusion.Sample( g_samWrapLinear, uv ).r;
    materialParams.normal = ApplyNormalMap( g_txNormal, g_samWrapLinear, uv, Input.vNormal, Input.vTangent );

    // tests
    //materialParams.albedo = 0.5f;
    //materialParams.roughness = 0.5f;
    //materialParams.metalness = 0.f;
    //materialParams.ambientOcclusion = 1.f;
    //materialParams.normal = normalize( Input.vNormal );

    float3 viewDir = normalize( g_vCameraPos - Input.vWorldPos );
    // float3 OutColor = PhongLighting( lightParams, materialParams, viewDir );
    float3 OutColor = BlinnPhongLighting( lightParams, materialParams , viewDir );
    // float3 OutColor = CookTorranceBRDFLighting( lightParams, materialParams , viewDir );

    return float4( OutColor, 1 );
}

