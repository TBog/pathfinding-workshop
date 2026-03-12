#include "pch.h"

#include "Clouds.h"

#include "..\Extern\tinyxml2.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"
#include "..\Utils\Mesh.h"
#include "..\Utils\Texture.h"
#include "..\Utils\Camera.h"

#include "RenderManager.h"
#include "TexturesManager.h"


ShaderVS g_CloudsVS( L"Shaders/Clouds_VS.fx" );
ShaderPS g_CloudsPS( L"Shaders/Clouds_PS.fx" );
ShaderPS g_Clouds_basePS( L"Shaders/Clouds_base_PS.fx" );
ShaderPS g_Clouds_detailsPS( L"Shaders/Clouds_details_PS.fx" );
ShaderPS g_Clouds_applyPS( L"Shaders/Clouds_apply_PS.fx" );


struct st_Clouds_CB_VS_PER_OBJECT
{
    float                   m_pad1;
};

struct st_Clouds_CB_PS_PER_OBJECT
{
    D3DXMATRIX              m_prevViewProjectionMatrix;
    float                   m_time;
};

//-------------------------------------------------------------------

Clouds::Clouds( )
    : m_szFileName      ( NULL )
    , m_cloudsBlendState( NULL )
    , m_currentCloudsRT ( 0 )
    , m_cloudsBaseMap   ( NULL )
    , m_cloudsDetailsMap( NULL )
{
    m_cloudsRenderTarget[0] = NULL;
    m_cloudsRenderTarget[1] = NULL;

    D3DXMatrixIdentity( &m_prevViewProjectionMatrix );
}

//-------------------------------------------------------------------

Clouds::~Clouds( )
{
    Unload( );
}

//-------------------------------------------------------------------

void Clouds::CreateResources( )
{
    D3D11_BLEND_DESC BSDesc;
    ZeroMemory(&BSDesc, sizeof(D3D11_BLEND_DESC));

    BSDesc.RenderTarget[0].BlendEnable = TRUE;
    BSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    BSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
    BSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

    g_renderManager->GetDevice()->CreateBlendState( &BSDesc, &m_cloudsBlendState );


    m_cloudsRenderTarget[0] = new Texture( g_renderManager->GetResolutionWidth(), g_renderManager->GetResolutionHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, false, eTextureType_RenderTarget );
    m_cloudsRenderTarget[1] = new Texture( g_renderManager->GetResolutionWidth(), g_renderManager->GetResolutionHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, false, eTextureType_RenderTarget );

    m_cloudsBaseMap = new Texture( g_renderManager->GetResolutionWidth(), g_renderManager->GetResolutionHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false, eTextureType_RenderTarget );
    m_cloudsDetailsMap = new Texture( 34 * 8, 34 * 4, DXGI_FORMAT_R8G8B8A8_UNORM, false, eTextureType_RenderTarget );
}

//-------------------------------------------------------------------

void Clouds::DestroyResources( )
{
    SAFE_RELEASE( m_cloudsBlendState );

    SAFE_RELEASE( m_cloudsRenderTarget[0] );
    SAFE_RELEASE( m_cloudsRenderTarget[1] );

    SAFE_RELEASE( m_cloudsBaseMap );
    SAFE_RELEASE( m_cloudsDetailsMap );
}

//-------------------------------------------------------------------

bool Clouds::Load(const WCHAR *_szFileName)
{
    Unload( );

    m_szFileName = _wcsdup( _szFileName );

    // convert file name to char*
    char szFileNameChar[256];
    ConvertWdieStringToString( m_szFileName, szFileNameChar );

    // load the clouds xml file
    tinyxml2::XMLDocument cloudsFile;
    cloudsFile.LoadFile( szFileNameChar );
    if ( cloudsFile.ErrorID() != 0 )
    {
        WCHAR msg[512];
        swprintf_s( msg, 512, L"Clouds::Load() - \"%s\" failed!", m_szFileName );
        myAssert( false, msg );

        return false;
    }

    tinyxml2::XMLElement *cloudsElement = cloudsFile.FirstChildElement("clouds");


    return true;
}

//-------------------------------------------------------------------

void Clouds::Unload( )
{
    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void Clouds::PreRender( )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

    // Set the blend state and the depth stencil state
    float BlendFactor[4] = { 0, 0, 0, 0 };
    d3dImmediateContext->OMSetBlendState( g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF );
    d3dImmediateContext->OMSetDepthStencilState( g_renderManager->GetNoDepthTestAndNoWriteState(), 0 );
    d3dImmediateContext->RSSetState( g_renderManager->GetCullRasterState() );

    // Set the constants
    st_Clouds_CB_VS_PER_OBJECT* pVSPerObject = (st_Clouds_CB_VS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectVS );
    g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectVS);

    st_Clouds_CB_PS_PER_OBJECT* pPSPerObject = (st_Clouds_CB_PS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectPS );
    g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectPS);

    ID3D11SamplerState *samWrapLinear = g_renderManager->GetSamWrapLinear();
    d3dImmediateContext->PSSetSamplers( 0, 1, &samWrapLinear );

    // Set the shaders
    g_CloudsVS.Set();
    g_Clouds_basePS.Set();

    g_renderManager->SetRenderTargets( m_cloudsBaseMap, NULL );

    g_renderManager->GetQuadMesh()->Draw();


    // Set the shaders
    g_CloudsVS.Set();
    g_Clouds_detailsPS.Set();

    g_renderManager->SetRenderTargets( m_cloudsDetailsMap, NULL );

    g_renderManager->GetQuadMesh()->Draw();
}

//-------------------------------------------------------------------

void Clouds::Render( Texture *_backColorBuffer, Texture *_backDepthBuffer )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext();

    
    // render the clouds in a render target (using temporal re-projection to eliminate noise)
    {
        g_renderManager->SetRenderTargets(m_cloudsRenderTarget[m_currentCloudsRT], NULL);

        // Set the blend state and the depth stencil state
        float BlendFactor[4] = { 0, 0, 0, 0 };
        d3dImmediateContext->OMSetBlendState( g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF );
        d3dImmediateContext->OMSetDepthStencilState( g_renderManager->GetNoDepthTestAndNoWriteState(), 0 );
        d3dImmediateContext->RSSetState( g_renderManager->GetCullRasterState() );

        // Set the constants
        st_Clouds_CB_VS_PER_OBJECT* pVSPerObject = (st_Clouds_CB_VS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectVS );
        g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectVS );

        st_Clouds_CB_PS_PER_OBJECT* pPSPerObject = (st_Clouds_CB_PS_PER_OBJECT*)g_renderManager->LockConstantBuffer( eConstantBufferType_PerObjectPS );
        D3DXMatrixTranspose( &pPSPerObject->m_prevViewProjectionMatrix, &m_prevViewProjectionMatrix );
        pPSPerObject->m_time = (float)g_renderManager->GetFrameTime().GetTime();
        g_renderManager->UnlockConstantBuffer( eConstantBufferType_PerObjectPS );

        ID3D11SamplerState *samWrapLinear = g_renderManager->GetSamWrapLinear();
        ID3D11SamplerState *samClampLinear = g_renderManager->GetSamClampLinear();
        ID3D11SamplerState *samPoint = g_renderManager->GetSamPoint();
        d3dImmediateContext->PSSetSamplers( 0, 1, &samWrapLinear );
        d3dImmediateContext->PSSetSamplers( 1, 1, &samClampLinear );
        d3dImmediateContext->PSSetSamplers( 2, 1, &samPoint );

        // Set the shaders
        g_CloudsVS.Set();
        g_CloudsPS.Set();

        m_cloudsRenderTarget[1 - m_currentCloudsRT]->PSSet( 0 );
        _backDepthBuffer->PSSet( 1 );
        m_cloudsBaseMap->PSSet( 2 );
        m_cloudsDetailsMap->PSSet( 3 );

        g_renderManager->GetQuadMesh()->Draw();

        Texture::PSSetNull( 1 );
        Texture::PSSetNull( 2 );
        Texture::PSSetNull( 3 );
    }

    // compose with the scene with alpha blend
    {
        g_renderManager->SetRenderTargets( _backColorBuffer, _backDepthBuffer );

        float BlendFactor[4] = { 0, 0, 0, 0 };
        d3dImmediateContext->OMSetBlendState( m_cloudsBlendState, BlendFactor, 0xFFFFFFFF );

        // Set the shaders
        g_CloudsVS.Set();
        g_Clouds_applyPS.Set();

        m_cloudsRenderTarget[m_currentCloudsRT]->PSSet( 0 );

        g_renderManager->GetQuadMesh()->Draw();
    }

    m_currentCloudsRT = 1 - m_currentCloudsRT;
    m_prevViewProjectionMatrix = g_renderManager->GetCamera()->GetViewMatrix() * g_renderManager->GetCamera()->GetProjMatrix();
}

//-------------------------------------------------------------------
