#include "pch.h"

#include "Sky.h"

#include "..\Extern\tinyxml2.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"
#include "..\Utils\Mesh.h"

#include "RenderManager.h"


ShaderVS g_SkyVS(L"Shaders/Sky_VS.fx");
ShaderPS g_SkyPS(L"Shaders/Sky_PS.fx");


struct st_Sky_CB_VS_PER_OBJECT
{
    float                   m_pad1;
};

struct st_Sky_CB_PS_PER_OBJECT
{
    D3DXVECTOR3             m_skyColor;
    float                   m_skyLightPower;
};

//-------------------------------------------------------------------

Sky::Sky( )
    : m_szFileName      ( NULL )
    , m_light_dir       ( 1.0f, 1.0f, 1.f ) // we'll normalize this on init
    , m_light_color     ( 1.0f, 1.0f, 0.81f )
    , m_light_power     ( 40.f )
    , m_light_ambientColor( 0.15f, 0.58f, 0.83f )
    , m_light_ambientPower( 10.f )
    , m_sky_color       ( 0.62f, 0.84f, 0.94f )
    , m_sky_lightPower  ( 40.f )
    , m_fog_color       ( 0.62f, 0.84f, 0.94f )
    , m_fog_lightPower  ( 40.f )
    , m_fog_start       ( 250.f )
    , m_fog_end         ( 500.f )
{
    D3DXVec3Normalize( &m_light_dir, &m_light_dir );
}

//-------------------------------------------------------------------

Sky::~Sky( )
{
    Unload( );
}

//-------------------------------------------------------------------

bool Sky::Load(const WCHAR *_szFileName)
{
    Unload( );

    m_szFileName = _wcsdup( _szFileName );

    // convert file name to char*
    char szFileNameChar[256];
    ConvertWdieStringToString( m_szFileName, szFileNameChar );

    // load the sky xml file
    tinyxml2::XMLDocument skyFile;
    skyFile.LoadFile( szFileNameChar );
    if ( skyFile.ErrorID() != 0 )
    {
        WCHAR msg[512];
        swprintf_s( msg, 512, L"Sky::Load() - \"%s\" failed!", m_szFileName );
        myAssert( false, msg );

        return false;
    }

    tinyxml2::XMLElement *skyElement = skyFile.FirstChildElement("sky");

    skyElement->QueryFloatAttribute("light_dir_x", &m_light_dir.x);
    skyElement->QueryFloatAttribute("light_dir_y", &m_light_dir.y);
    skyElement->QueryFloatAttribute("light_dir_z", &m_light_dir.z);

    skyElement->QueryFloatAttribute("light_color_r", &m_light_color.x);
    skyElement->QueryFloatAttribute("light_color_g", &m_light_color.y);
    skyElement->QueryFloatAttribute("light_color_b", &m_light_color.z);
    skyElement->QueryFloatAttribute("light_power", &m_light_power);

    skyElement->QueryFloatAttribute("light_ambientColor_r", &m_light_ambientColor.x);
    skyElement->QueryFloatAttribute("light_ambientColor_g", &m_light_ambientColor.y);
    skyElement->QueryFloatAttribute("light_ambientColor_b", &m_light_ambientColor.z);
    skyElement->QueryFloatAttribute("light_ambientPower", &m_light_ambientPower);


    skyElement->QueryFloatAttribute("sky_color_r", &m_sky_color.x);
    skyElement->QueryFloatAttribute("sky_color_g", &m_sky_color.y);
    skyElement->QueryFloatAttribute("sky_color_b", &m_sky_color.z);
    skyElement->QueryFloatAttribute("sky_lightPower", &m_sky_lightPower);

    skyElement->QueryFloatAttribute("fog_color_r", &m_fog_color.x);
    skyElement->QueryFloatAttribute("fog_color_g", &m_fog_color.y);
    skyElement->QueryFloatAttribute("fog_color_b", &m_fog_color.z);
    skyElement->QueryFloatAttribute("fog_lightPower", &m_fog_lightPower);
    skyElement->QueryFloatAttribute("fog_start", &m_fog_start);
    skyElement->QueryFloatAttribute("fog_end", &m_fog_end);

    D3DXVec3Normalize( &m_light_dir, &m_light_dir );

    return true;
}

//-------------------------------------------------------------------

void Sky::Unload( )
{
    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void Sky::Render( )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

    // Set the blend state and the depth stencil state
    float BlendFactor[4] = { 0, 0, 0, 0 };
    d3dImmediateContext->OMSetBlendState( g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF );
    d3dImmediateContext->OMSetDepthStencilState( g_renderManager->GetNoDepthWriteState(), 0 );
    d3dImmediateContext->RSSetState( g_renderManager->GetCullRasterState() );

    // Set the shaders
    g_SkyVS.Set();
    g_SkyPS.Set();

    // Set the constants
    st_Sky_CB_VS_PER_OBJECT* pVSPerObject = (st_Sky_CB_VS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectVS );
    g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectVS );

    st_Sky_CB_PS_PER_OBJECT* pPSPerObject = (st_Sky_CB_PS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectPS );
    pPSPerObject->m_skyColor        = m_sky_color;
    pPSPerObject->m_skyLightPower   = m_sky_lightPower;

    g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectPS );


    g_renderManager->GetQuadMesh()->Draw( );
}

//-------------------------------------------------------------------

void Sky::SetLightParams( const D3DXVECTOR3 &_light_dir, const D3DXVECTOR3 &_light_color, float _light_power, const D3DXVECTOR3 &_light_ambientColor, float _light_ambientPower )
{
    m_light_dir         = _light_dir;
    m_light_color       = _light_color;
    m_light_power       = _light_power;
    m_light_ambientColor = _light_ambientColor;
    m_light_ambientPower = _light_ambientPower;

    D3DXVec3Normalize( &m_light_dir, &m_light_dir );
}

//-------------------------------------------------------------------

void Sky::SetSkyColorParams( const D3DXVECTOR3 &_sky_color, float _sky_lightPower )
{
    m_sky_color         = _sky_color;
    m_sky_lightPower    = _sky_lightPower;
}

//-------------------------------------------------------------------

void Sky::SetFogParams( const D3DXVECTOR3 &_fog_color, float _fog_lightPower, float _fog_start, float _fog_end )
{
    m_fog_color         = _fog_color;
    m_fog_lightPower    = _fog_lightPower;
    m_fog_start         = _fog_start;
    m_fog_end           = _fog_end;
}

//-------------------------------------------------------------------
