#define PI  3.14159265359f

//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------
struct LightParams
{
    float3 lightDir;
    float3 lightIntensity;
    float3 ambientIntensity;
};

struct MaterialParams
{
    float3 albedo;
    float3 normal;
    float roughness;
    float metalness;
    float ambientOcclusion;
};

LightParams FillLightParams( float3 vLightDir, float3 vLightColor, float fLightPower, float3 vAmbientColor, float fAmbientPower )
{
    LightParams lightParams;
    lightParams.lightDir = vLightDir;
    lightParams.lightIntensity = vLightColor * fLightPower * PI;
    lightParams.ambientIntensity = vAmbientColor * fAmbientPower * PI;

    return lightParams;
}

float3 ApplyNormalMap( Texture2D normalMap, SamplerState samplerState, float2 uv, float3 vertexNormal, float3 vertexTangent )
{
    float3 localNormal = normalMap.Sample( samplerState, uv ).xzy * 2 - 1;  // z and y swapped to get it into our coord space
    localNormal.z = -localNormal.z;

    vertexNormal = normalize( vertexNormal );
    vertexTangent = normalize( vertexTangent );
    float3 vertexBinormal = cross( vertexTangent, vertexNormal );
    float3x3 tangentSpace = { vertexTangent, vertexNormal, vertexBinormal };
    
    float3 normal = normalize( mul( localNormal, tangentSpace ) );

    return normal;
}

float3 ComputeAmbient( LightParams lightParams, MaterialParams materialParams )
{
    float ambientBackground = max( 0.5f + 0.5f * materialParams.normal.y, 0.f );
    float backlight = max( 0.5f + 0.5f * dot( normalize( float3(-lightParams.lightDir.x, 0., lightParams.lightDir.z) ), materialParams.normal ), 0.f );
    float3 ambient = lightParams.ambientIntensity * ( materialParams.albedo / PI ) * ( ambientBackground + backlight );

    return ambient;
}


//--------------------------------------------------------------------------------------
// PHONG LIGHTING
//--------------------------------------------------------------------------------------
float3 PhongLighting( LightParams lightParams, MaterialParams materialParams, float3 viewDir )
{
    float glossiness = 1.f - materialParams.roughness;
    float shininess = pow(2, 13 * glossiness);
    float specularRatio = lerp( 0.05f, 0.5f, pow( glossiness, 2.f ) );

    float NdotL = max(dot(lightParams.lightDir, materialParams.normal), 0.f);
    float3 diffuse = lightParams.lightIntensity * materialParams.albedo / PI * NdotL;

    float3 reflectDir = reflect(-lightParams.lightDir, materialParams.normal);
    float specAngle = max(dot(reflectDir, viewDir), 0.f);
    float3 specular = lightParams.lightIntensity * specularRatio * pow(specAngle, shininess / 4.f);

    float3 ambient = ComputeAmbient(lightParams, materialParams);

    float3 color = ambient + diffuse + specular;
    color *= materialParams.ambientOcclusion;

    return color;
}

//--------------------------------------------------------------------------------------
// BLINN-PHONG LIGHTING
//--------------------------------------------------------------------------------------
float3 BlinnPhongLighting( LightParams lightParams, MaterialParams materialParams, float3 viewDir )
{
    float glossiness = 1.f - materialParams.roughness;
    float shininess = pow(2, 13 * glossiness);
    float specularRatio = lerp( 0.05f, 0.5f, pow( glossiness, 2.f ) );

    float NdotL = max(dot(lightParams.lightDir, materialParams.normal), 0.f);
    float3 diffuse = lightParams.lightIntensity * materialParams.albedo / PI * NdotL;

    float3 halfDir = normalize(lightParams.lightDir + viewDir);
    float specAngle = max(dot(halfDir, materialParams.normal), 0.0);
    float3 specular = lightParams.lightIntensity * specularRatio * pow(specAngle, shininess);

    float3 ambient = ComputeAmbient( lightParams, materialParams );

    float3 color = ambient + diffuse + specular;
    color *= materialParams.ambientOcclusion;

    return color;
}

//--------------------------------------------------------------------------------------
// COOK TORRANCE BRDF LIGHTING
//--------------------------------------------------------------------------------------

/**
* GGX/Trowbridge-Reitz NDF
*
* Calculates the specular highlighting from surface roughness.
*
* Roughness lies on the range [0.0, 1.0], with lower values
* producing a smoother, "glossier", surface. Higher values
* produce a rougher surface with the specular lighting distributed
* over a larger surface area.
*/
float CalculateNDF( float3 normal, float3 halfDir, float roughness )
{
    float a2 = ( roughness * roughness );
    float dot_n_h = dot( normal, halfDir );

    return ( a2 / ( PI * pow( ( 1 + (a2 - 1) * dot_n_h * dot_n_h ), 2 ) ) );
}

/**
* GGX/Schlick-Beckmann microfacet geometric attenuation.
*
* The attenuation is modified by the roughness (input as k)
* and approximates the influence/amount of microfacets in the surface.
* A microfacet is a sub-pixel structure that affects light
* reflection/occlusion.
*/
float CalculateAttenuation( float3 normal, float3 v, float k )
{
    float dot_n_v  = max(dot(normal, v), 0.f);
    return ( dot_n_v / ((dot_n_v * (1 - k)) + k) );
}

/**
* GGX/Schlick-Beckmann attenuation for analytical light sources.
*/
float CalculateGeometryAttenuation( float3 normal, float3 lightDir, float3 viewDir, float roughness )
{
    float k = pow( (roughness + 1), 2 ) * 0.125f;

    float lightAtten = CalculateAttenuation( normal, lightDir, k );
    float viewAtten = CalculateAttenuation( normal, viewDir, k );

    return (lightAtten * viewAtten);
}

/**
* Calculates the Fresnel reflectivity.
* The metallic parameter controls the fresnel incident value (fresnel0).
*/
float3 CalculateFresnel( float3 halfDir, float3 viewDir, float3 F0 )
{
    float dot_h_v = max(dot(halfDir, viewDir), 0.f);

    return F0 + ( (1 - F0) * pow( 1 - dot_h_v, 5 ) );
}

/**
* Cook-Torrance BRDF for analytical light sources.
*/
float3 CalculateSpecular( float3 normal, float3 lightDir, float3 viewDir, float3  F0, out float3 F, float roughness )
{
    float3 halfDir = normalize( lightDir + viewDir );
    float dot_n_l = saturate( dot( normal, lightDir ) );
    float dot_n_v = saturate( dot( normal, viewDir ) );

    float D = CalculateNDF( normal, halfDir, roughness );
    float G = CalculateGeometryAttenuation( normal, lightDir, viewDir, roughness );

    F = CalculateFresnel( halfDir, viewDir, F0 );

    float3 numerator = (D * F * G);
    float denominator = (4 * dot_n_l * dot_n_v) + 0.001f;

    return (numerator / denominator);
}

/**
* Standard Lambertian diffuse lighting.
*/
float3 CalculateDiffuse( float3 albedo )
{
    return ( albedo / PI );
}

/**
* Calculates the total light contribution for the analytical light source.
*/
float3 CalculateLighting( float3 normal, float3 ligthDir, float3 viewDir, float3 albedo, float roughness, float metalness )
{
    float3 F0 = lerp( 0.04f, albedo, metalness );
    float3 ks = 0.f;
    float3 diffuse = CalculateDiffuse( albedo );
    float3 specular = CalculateSpecular( normal, ligthDir, viewDir, F0, ks, roughness );
    float3 kd = ( 1.f - ks );

    float dot_n_l = saturate( dot(normal, ligthDir) );

    return ( ( kd * diffuse ) + specular ) * dot_n_l;
}

float3 CookTorranceBRDFLighting( LightParams lightParams, MaterialParams materialParams, float3 viewDir )
{
    float3 directLight = CalculateLighting( materialParams.normal, lightParams.lightDir, viewDir, materialParams.albedo, materialParams.roughness, materialParams.metalness );
    float3 ambient = ComputeAmbient( lightParams, materialParams );
    float3 color = ambient + directLight * lightParams.lightIntensity;
    color *= materialParams.ambientOcclusion;

    return color;
}

//--------------------------------------------------------------------------------------
// FOG
//--------------------------------------------------------------------------------------

float ComputeFogFactor(float viewDist, float fogStartZ, float invFogDist )
{
    return saturate( (viewDist - fogStartZ) * invFogDist );
}

float3 ApplyFog(float3 color, float3 fogColor, float fogLightPower, float fogFactor)
{
    return lerp(color, fogColor * fogLightPower, fogFactor);
}

//--------------------------------------------------------------------------------------
// HDR
//--------------------------------------------------------------------------------------

float3 ApplyHDRCorrection(float3 color, float HDRScale)
{
    return color * HDRScale;
}
