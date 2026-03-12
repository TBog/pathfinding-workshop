#include "pch.h"

#include "PostProcess.h"

#include "..\Extern\tinyxml2.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"
#include "..\Utils\Mesh.h"
#include "..\Utils\Texture.h"

#include "RenderManager.h"


ShaderVS g_PostProcessVS( L"Shaders/PostProcess_VS.fx" );
ShaderPS g_PostProcessPS( L"Shaders/PostProcess_PS.fx" );


struct st_PostProcess_CB_VS_PER_OBJECT
{
    float                   m_pad1;
};

struct st_PostProcess_CB_PS_PER_OBJECT
{
    float                   m_pad1;
};

//-------------------------------------------------------------------

PostProcess::PostProcess( )
    : m_szFileName      ( NULL )
{
}

//-------------------------------------------------------------------

PostProcess::~PostProcess( )
{
    Unload( );
}

//-------------------------------------------------------------------

bool PostProcess::Load( const WCHAR *_szFileName )
{
    Unload( );

    m_szFileName = _wcsdup( _szFileName );

    // convert file name to char*
    char szFileNameChar[256];
    ConvertWdieStringToString( m_szFileName, szFileNameChar );

    // load the post process xml file
    tinyxml2::XMLDocument postProcessFile;
    postProcessFile.LoadFile( szFileNameChar );
    if ( postProcessFile.ErrorID() != 0 )
    {
        WCHAR msg[512];
        swprintf_s( msg, 512, L"PostProcess::Load() - \"%s\" failed!", m_szFileName );
        myAssert( false, msg );

        return false;
    }

    tinyxml2::XMLElement *postProcessElement = postProcessFile.FirstChildElement( "postProcess" );


    return true;
}

//-------------------------------------------------------------------

void PostProcess::Unload( )
{
    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void PostProcess::Render( Texture *_HDRColorBuffer, Texture *_depthBuffer )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

    // Set the blend state and the depth stencil state
    float BlendFactor[4] = { 0, 0, 0, 0 };
    d3dImmediateContext->OMSetBlendState( g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF );
    d3dImmediateContext->OMSetDepthStencilState( g_renderManager->GetNoDepthTestAndNoWriteState(), 0 );
    d3dImmediateContext->RSSetState( g_renderManager->GetCullRasterState() );

    // Set the shaders
    g_PostProcessVS.Set();
    g_PostProcessPS.Set();

    // Set the constants
    st_PostProcess_CB_VS_PER_OBJECT* pVSPerObject = (st_PostProcess_CB_VS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectVS );
    g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectVS );

    st_PostProcess_CB_PS_PER_OBJECT* pPSPerObject = (st_PostProcess_CB_PS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectPS );
    g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectPS );

    ID3D11SamplerState *samWrapLinear = g_renderManager->GetSamWrapLinear();
    d3dImmediateContext->PSSetSamplers( 0, 1, &samWrapLinear );

    _HDRColorBuffer->PSSet( 0 );
    _depthBuffer->PSSet( 1 );


    g_renderManager->GetQuadMesh()->Draw( );


    Texture::PSSetNull( 0 );
    Texture::PSSetNull( 1 );
}

//-------------------------------------------------------------------
