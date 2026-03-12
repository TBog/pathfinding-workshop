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
// GDI always writes 0 into the reserved (alpha) byte, so we derive alpha from the
// colour: any non-black pixel was painted by GDI and should be fully opaque.
// Alpha blending (SRC_ALPHA / INV_SRC_ALPHA) is enabled by the caller, so texels
// with alpha = 0 are fully transparent and let the scene show through.
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT Input ) : SV_TARGET
{
    float4 c = g_txTexture.Sample( g_samPoint, Input.vTexcoord.xy );
    // Derive alpha: painted pixels have at least one non-zero colour channel.
    c.a = step( 1.0 / 255.0, dot( c.rgb, float3( 1.0, 1.0, 1.0 ) ) );
    return c;
}
