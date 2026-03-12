//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float2 vPosition    : POSITION;
};

struct VS_OUTPUT
{
    float2 vTexcoord    : TEXCOORD0;
    float4 vPosition    : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
    VS_OUTPUT Output;

    Output.vPosition = float4( Input.vPosition.xy * 2.f - 1.f, 0.5f, 1.f );
    Output.vPosition.y = -Output.vPosition.y;
    Output.vTexcoord.xy = Input.vPosition;

    return Output;
}

