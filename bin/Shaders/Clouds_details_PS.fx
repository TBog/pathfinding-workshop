#include "Common_PS.h"
#include "Clouds_utils.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbCloudsObjectConsts : register( b2 )
{
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float2 vTexcoord    : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 main( PS_INPUT Input ) : SV_TARGET
{
    float2 fragCoord = Input.vTexcoord * float2( 34 * 8, 34 * 4 ); // [r]

    // pack 32x32x32 3d texture in 2d texture (with padding)
    float z = floor( fragCoord.x / 34 ) + 8 * floor( fragCoord.y / 34 );
    float2 uv = fmod( fragCoord.xy, 34 ) - 1;
    float3 coord = float3( uv, z ) / 32;

    float r = tilableVoronoi( coord, 16,  3 );
    float g = tilableVoronoi( coord,  4,  8 );
    float b = tilableVoronoi( coord,  4, 16 );

    float c = max( 0, 1 - ( r + g * 0.5f + b * 0.25f ) / 1.75f );

    return float4( c, c, c, c );
}
