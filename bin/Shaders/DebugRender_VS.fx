#include "Common_VS.h"

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 vPosition    : POSITION;
    float4 vColor       : COLOR;
};

struct VS_OUTPUT
{
    float4 vColor       : COLOR;
    float4 vPosition    : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
    VS_OUTPUT Output;

    Output.vPosition = mul( float4( Input.vPosition, 1.f ), g_mViewProjection );
    Output.vColor = Input.vColor;

    return Output;
}
