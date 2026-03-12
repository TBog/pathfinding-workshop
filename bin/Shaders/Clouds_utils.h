//--------------------------------------------------------------------------------------
// Helper functions for the clouds
//--------------------------------------------------------------------------------------

float hash12( float2 p )
{
    p  = 50.f * frac( p * 0.3183099f );
    return frac( p.x * p.y * ( p.x+p.y ) );
}

float hash13( float3 p3 )
{
    p3  = frac( p3 * 1031.1031f );
    p3 += dot( p3, p3.yzx + 19.19f );
    return frac( (p3.x + p3.y) * p3.z );
}

float3 hash33( float3 p3 )
{
    p3 = frac( p3 * float3( 0.1031f, 0.1030f, 0.0973f ) );
    p3 += dot( p3, p3.yxz + 19.19f );
    return frac( (p3.xxy + p3.yxx) * p3.zyx );
}

float valueHash( float3 p3 )
{
    p3  = frac( p3 * 0.1031f );
    p3 += dot( p3, p3.yzx + 19.19f );
    return frac( (p3.x + p3.y) * p3.z );
}

//
// Noise functions used for cloud shapes
//
float valueNoise( in float3 x, float tile )
{
    float3 p = floor( x );
    float3 f = frac( x );
    f = f * f * ( 3.f - 2.f * f );

    return lerp(lerp(lerp( valueHash(fmod(p+float3(0,0,0),tile)), 
        valueHash(fmod(p+float3(1,0,0),tile)),f.x),
        lerp( valueHash(fmod(p+float3(0,1,0),tile)), 
            valueHash(fmod(p+float3(1,1,0),tile)),f.x),f.y),
        lerp(lerp( valueHash(fmod(p+float3(0,0,1),tile)), 
            valueHash(fmod(p+float3(1,0,1),tile)),f.x),
            lerp( valueHash(fmod(p+float3(0,1,1),tile)), 
                valueHash(fmod(p+float3(1,1,1),tile)),f.x),f.y),f.z);
}

float voronoi( float3 x, float tile )
{
    float3 p = floor( x );
    float3 f = frac( x );

    float res = 100.;
    for ( int k = -1; k <= 1; k++ )
    {
        for ( int j = -1; j <= 1 ; j++ )
        {
            for( int i = -1; i <= 1; i++ )
            {
                float3 b = float3( i, j, k );
                float3 c = p + b;

                if ( tile > 0.f )
                {
                    c = fmod( c, tile );
                }

                float3 r = float3(b) - f + hash13( c );
                float d = dot( r, r );

                if ( d < res )
                {
                    res = d;
                }
            }
        }
    }

    return 1.f - res;
}

float tilableVoronoi( float3 p, const int octaves, float tile )
{
    float f = 1.f;
    float a = 1.f;
    float c = 0.f;
    float w = 0.f;

    if ( tile > 0.f )
        f = tile;

    for ( int i=0; i<octaves; i++ )
    {
        c += a * voronoi( p * f, f );
        f *= 2.f;
        w += a;
        a *= 0.5f;
    }

    return c / w;
}

float tilableFbm( float3 p, const int octaves, float tile )
{
    float f = 1.f;
    float a = 1.f;
    float c = 0.f;
    float w = 0.f;

    if ( tile > 0. )
        f = tile;

    for ( int i = 0; i < octaves; i++ )
    {
        c += a * valueNoise( p * f, f );
        f *= 2.f;
        w += a;
        a *= 0.5f;
    }

    return c / w;
}
