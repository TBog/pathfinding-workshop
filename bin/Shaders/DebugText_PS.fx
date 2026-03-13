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
// The texture is uploaded as DXGI_FORMAT_B8G8R8A8_UNORM from a GDI DIBSection.
// DXGI swizzles the in-memory BGRX bytes so the shader receives correct RGB values.
// The alpha channel is written explicitly by _FlushText (background fill, outline and
// main text each receive the requested alpha value), so we can use it directly.
// Alpha blending (SRC_ALPHA / INV_SRC_ALPHA) is enabled by the caller, so texels
// with alpha = 0 are fully transparent and let the scene show through.
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input ) : SV_TARGET
{
    return g_txTexture.Sample( g_samPoint, Input.vTexcoord.xy );
}
