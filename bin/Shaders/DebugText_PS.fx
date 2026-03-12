//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D    g_txTexture : register( t0 );

SamplerState g_samPoint : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float2 vTexcoord    : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
// Samples the debug-text overlay (RGBA, premultiplied-like alpha written by the CPU).
// Alpha blending (SRC_ALPHA / INV_SRC_ALPHA) is enabled by the caller so transparent
// texels are invisible and opaque texels overwrite the scene.
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input ) : SV_TARGET
{
    return g_txTexture.Sample( g_samPoint, Input.vTexcoord.xy );
}
