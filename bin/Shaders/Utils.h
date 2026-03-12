//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

float3 ScreenToView( float3 screenPos, float4 projParams )
{
    float3 viewPos;

    viewPos.z = projParams.w / (screenPos.z - projParams.z);
    viewPos.xy = viewPos.z * screenPos.xy / projParams.xy;

    return viewPos;
}
