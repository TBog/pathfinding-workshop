#include "Common_PS.h"
#include "Clouds_utils.h"
#include "Utils.h"
#include "Lighting.h"

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer cbCloudsObjectConsts : register( b2 )
{
    matrix g_mPrevViewProjection    : packoffset( c0 );
    float g_fTime                   : packoffset( c4.x );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D   g_txPrevCloudsColor : register( t0 );
Texture2D   g_txDepth           : register( t1 );
Texture2D   g_txCloudMapBase    : register( t2 );
Texture2D   g_txCloudMapDetails : register( t3 );

SamplerState g_samWrapLinear    : register( s0 );
SamplerState g_samClampLinear   : register( s1 );
SamplerState g_samPoint         : register( s2 );

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

#define CLOUD_MARCH_STEPS                   12
#define CLOUD_SELF_SHADOW_STEPS             6

#define CLOUDS_MAX_DISTANCE                 (3000.f)

#define CLOUDS_BOTTOM                       (100.f)
#define CLOUDS_TOP                          (200.f)

#define CLOUDS_COVERAGE                     (0.7f)

#define CLOUDS_DETAIL_STRENGTH              (0.2f)
#define CLOUDS_BASE_EDGE_SOFTNESS           (0.1f)
#define CLOUDS_BOTTOM_SOFTNESS              (0.25f)
#define CLOUDS_DENSITY                      (0.03f)
#define CLOUDS_SHADOW_MARGE_STEP_SIZE       (10.f)
#define CLOUDS_SHADOW_MARGE_STEP_MULTIPLY   (1.3f)
#define CLOUDS_FORWARD_SCATTERING_G         (0.8f)
#define CLOUDS_BACKWARD_SCATTERING_G        (-0.2f)
#define CLOUDS_SCATTERING_LERP              (0.5f)

#define CLOUDS_AMBIENT_COLOR_TOP            (float3( 149.f, 167.f, 200.f ) * ( 0.75f / 255.f ))
#define CLOUDS_AMBIENT_COLOR_BOTTOM         (float3( 39.f, 67.f, 87.f ) * ( 2.f / 255.f ))
#define CLOUDS_MIN_TRANSMITTANCE            0.1f

#define CLOUDS_BASE_SCALE                   2.f
#define CLOUDS_DETAIL_SCALE                 20.f

//
// Cloud shape modelling and rendering 
//

float HenyeyGreenstein( float sundotrd, float g)
{
    float gg = g * g;
    return (1.f - gg) / pow( max(1.f + gg - 2.f * g * sundotrd, 0.f), 1.5f );
}

float linearstep( const float s, const float e, float v )
{
    return saturate( (v-s)*(1./(e-s)) );
}

float linearstep0( const float e, float v )
{
    return min( v*(1./e), 1. );
}

float remap( float v, float s, float e )
{
    return (v - s) / (e - s);
}

float cloudMapBase( float3 p, float norY )
{
    float3 uv = p * ( 0.0005f * CLOUDS_BASE_SCALE );
    float3 cloud = g_txCloudMapBase.SampleLevel( g_samWrapLinear, uv.xz, 0 ).rgb;

    float n = norY * norY;
    n *= cloud.b;
    n += pow( max( 1.f - norY, 0.f ), 16.f );

    return remap( cloud.r - n, cloud.g, 1.f );
}

float cloudMapDetail( float3 p )
{ 
    float2 iResolution;
    g_txCloudMapDetails.GetDimensions( iResolution.x, iResolution.y );

    // 3d lookup in 2d texture :(
    p = abs(p) * (0.0016f * CLOUDS_BASE_SCALE * CLOUDS_DETAIL_SCALE);

    float yi = fmod( p.y, 32.f );
    int2 offset = int2( fmod(yi, 8.f), fmod(floor(yi / 8.f), 4.f) ) * 34 + 1;
    float a = g_txCloudMapDetails.SampleLevel( g_samWrapLinear, (fmod(p.xz,32.) + float2(offset.xy)+1.f) / iResolution.xy, 0 ).r;

    yi = fmod( p.y+1.,32.f );
    offset = int2( fmod(yi, 8.f), fmod(floor(yi / 8.f), 4.f) ) * 34 + 1;
    float b = g_txCloudMapDetails.SampleLevel( g_samWrapLinear, (fmod(p.xz,32.) + float2(offset.xy)+1.f) / iResolution.xy, 0 ).r;

    return lerp( a, b, frac(p.y) );
}

float cloudGradient( float norY )
{
    return linearstep( 0.f, 0.05f, norY ) - linearstep( 0.8f, 1.2f, norY );
}

float cloudMap( float3 pos, float3 rd, float norY )
{
    float3 ps = pos;

    float m = cloudMapBase( ps, norY );
    m *= cloudGradient( norY );

    float dstrength = smoothstep( 1.f, 0.5f, m );

    // erode with detail
    if(dstrength > 0.)
    {
        m -= cloudMapDetail( ps ) * dstrength * CLOUDS_DETAIL_STRENGTH;
    }

    m = smoothstep( 0., CLOUDS_BASE_EDGE_SOFTNESS, m+(CLOUDS_COVERAGE-1.) );
    m *= linearstep0( CLOUDS_BOTTOM_SOFTNESS, norY );

    return saturate( m * CLOUDS_DENSITY );
}

float volumetricShadow( in float3 from, in float sundotrd )
{
    float dd = CLOUDS_SHADOW_MARGE_STEP_SIZE;
    float3 rd = g_vLightDir;
    float d = dd * 0.5f;
    float shadow = 1.f;

    for( int s = 0; s < CLOUD_SELF_SHADOW_STEPS; s++ )
    {
        float3 pos = from + rd * d;
        float norY = saturate( (pos.y - CLOUDS_BOTTOM) / (CLOUDS_TOP - CLOUDS_BOTTOM) );

        if ( norY > 1.f )
            return shadow;

        float muE = cloudMap( pos, rd, norY );
        shadow *= exp(-muE * dd);

        dd *= CLOUDS_SHADOW_MARGE_STEP_MULTIPLY;
        d += dd;
    }

    return shadow;
}

float4 renderClouds( float3 ro, float3 rd, inout float dist )
{
    dist = min( dist, CLOUDS_MAX_DISTANCE );

    float div_rdy = rd.y;
    float epsilon = 1.f / 100000.f;
    if (abs(div_rdy) < epsilon )
        div_rdy = sign(rd.y) * epsilon;

    float start = max( (CLOUDS_BOTTOM - ro.y) / div_rdy, 0.f );
    float end  = max( (CLOUDS_TOP - ro.y) / div_rdy, 0.f );
    if (div_rdy < 0.f)
    {
        float tmp = start;
        start = end;
        end  = tmp;
    }

    if (start > dist)
    {
        return float4( 0, 0, 0, 10 );
    }

    end = min( end, dist );

    float sundotrd = dot( rd, -g_vLightDir );

    // raymarch
    float d = start;
    float dD = ( end - start ) / float(CLOUD_MARCH_STEPS);

    float h = hash13( rd + frac(g_fTime) );
    d -= dD * h;

    float scattering =  lerp( HenyeyGreenstein(sundotrd, CLOUDS_FORWARD_SCATTERING_G),
        HenyeyGreenstein(sundotrd, CLOUDS_BACKWARD_SCATTERING_G), CLOUDS_SCATTERING_LERP );

    float transmittance = 1.f;
    float3 scatteredLight = float3( 0.f, 0.f, 0.f );

    for ( int s = 0; s < CLOUD_MARCH_STEPS; s++ )
    {
        float3 p = ro + d * rd;

        float norY = saturate( (p.y - CLOUDS_BOTTOM) / (CLOUDS_TOP - CLOUDS_BOTTOM) );

        float alpha = cloudMap( p, rd, norY );

        if( alpha > 0.f )
        {
            dist = min( dist, d );
            float3 ambientLight = lerp( CLOUDS_AMBIENT_COLOR_BOTTOM, CLOUDS_AMBIENT_COLOR_TOP, norY );

            float3 S = (ambientLight + g_vLightColor * (scattering * volumetricShadow(p, sundotrd))) * alpha;
            float dTrans = exp( -alpha * dD );
            float3 Sint = (S - S * dTrans) * (1. / alpha);
            scatteredLight += transmittance * Sint; 
            transmittance *= dTrans;
        }

        if( transmittance <= CLOUDS_MIN_TRANSMITTANCE )
            break;

        d += dD;
    }

    return float4( scatteredLight, transmittance );
}

float4 main( PS_INPUT Input ) : SV_TARGET
{
    float depth = g_txDepth.Sample( g_samPoint, Input.vTexcoord.xy ).r;
    float3 screenPos = float3( float2(Input.vTexcoord.x, 1.f - Input.vTexcoord.y) * 2.f - 1.f, depth );
    float3 viewPos = ScreenToView( screenPos, g_vProjParams );
    float3 dir = mul( normalize( viewPos ), (float3x3)g_mInvView );     // direction in world
    
    float dist = viewPos.z;
    float4 cloudsColor = renderClouds( g_vCameraPos, dir, dist );
    // float fogAmount = ComputeFogFactor( dist, g_fFogStartZ, g_fInvFogSize );
    // cloudsColor.rgb = lerp( cloudsColor.rgb, g_vFogColor * ( 1.f - cloudsColor.a ), fogAmount );
    if ( cloudsColor.a > 1 )
        cloudsColor = float4( 0, 0, 0, 1 );
    
    float3 worldPos = g_vCameraPos + dir * dist;
    float4 prevScreenPos = mul( float4( worldPos, 1.f ), g_mPrevViewProjection );
    float2 prevUV = prevScreenPos.xy / prevScreenPos.w;
    prevUV = prevUV * 0.5f + 0.5f;
    prevUV.y = 1.f - prevUV.y;
    float4 prevCloudsColor = g_txPrevCloudsColor.Sample( g_samClampLinear, prevUV );
    cloudsColor = lerp( prevCloudsColor, cloudsColor, 0.1f );

    return cloudsColor;
}
