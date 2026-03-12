//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbFrameConsts : register( b0 )
{
    float3  g_vCameraPos        : packoffset( c0 );
    float4  g_vProjParams       : packoffset( c1 );
    matrix  g_mInvView          : packoffset( c2 );
    float3  g_vLightDir         : packoffset( c6 );
    float3  g_vLightColor       : packoffset( c7 );
    float   g_fLightPower       : packoffset( c7.w );
    float3  g_vAmbientColor     : packoffset( c8 );
    float   g_fAmbientPower     : packoffset( c8.w );
    float3  g_vFogColor         : packoffset( c9 );
    float   g_fFogLighPower     : packoffset( c9.w );
};

