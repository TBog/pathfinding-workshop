//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbFrameConsts : register( b0 )
{
    matrix  g_mViewProjection       : packoffset( c0 );
    float3  g_vCameraPos            : packoffset( c4 );
    float   g_fFogStartZ            : packoffset( c5.x );
    float   g_fInvFogSize           : packoffset( c5.y );
};

