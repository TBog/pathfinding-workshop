#include "pch.h"

#include "Texture.h"

#include "Utils.h"

#include "..\Render\RenderManager.h"

//-------------------------------------------------------------------

Texture::Texture( const WCHAR *_szFileName )
    : m_eType           ( eTextureType_Default )
    , m_szFileName      ( NULL )
    , m_pTextureSRV     ( NULL )
    , m_pTexture        ( NULL )
    , m_pTmpCPUTexture  ( NULL )
    , m_nWidth          ( 0 )
    , m_nHeight         ( 0 )
    , m_nMipCount       ( 1 )
{
    ZeroMemory( m_pTextureRTV, kMaxMipsRTV * sizeof( ID3D11RenderTargetView* ) );
    ZeroMemory( m_pTextureDSV, kMaxMipsRTV * sizeof( ID3D11DepthStencilView* ) );

    Load( _szFileName );
}

//-------------------------------------------------------------------

Texture::Texture( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType )
    : m_eType           ( _eType )
    , m_szFileName      ( NULL )
    , m_pTextureSRV     ( NULL )
    , m_pTexture        ( NULL )
    , m_pTmpCPUTexture  ( NULL )
    , m_nWidth          ( 0 )
    , m_nHeight         ( 0 )
    , m_nMipCount       ( 1 )
{
    ZeroMemory( m_pTextureRTV, kMaxMipsRTV * sizeof( ID3D11RenderTargetView* ) );
    ZeroMemory( m_pTextureDSV, kMaxMipsRTV * sizeof( ID3D11DepthStencilView* ) );

    Create( _nWidth, _nHeight, _dxFormat, _bWithMips, _eType );
}

//-------------------------------------------------------------------

Texture::~Texture( )
{
    Unload( );
}

//-------------------------------------------------------------------

void Texture::Create( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType  )
{
    Unload();

    m_eType = _eType;

    DXGI_FORMAT DepthFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT SRVFormat = _dxFormat;

    if ( m_eType == eTextureType_DepthStencil )
    {
        switch( _dxFormat )
        {
        case DXGI_FORMAT_D32_FLOAT:
            DepthFormat = DXGI_FORMAT_R32_TYPELESS;
            SRVFormat = DXGI_FORMAT_R32_FLOAT;
            break;

        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            DepthFormat = DXGI_FORMAT_R24G8_TYPELESS;
            SRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;

        case DXGI_FORMAT_D16_UNORM:
            DepthFormat = DXGI_FORMAT_R16_TYPELESS;
            SRVFormat = DXGI_FORMAT_R16_UNORM;
            break;

        default:
            DepthFormat = DXGI_FORMAT_UNKNOWN;
            break;
        }
        myAssert( DepthFormat != DXGI_FORMAT_UNKNOWN, L"Texture::Create() DepthFormat unknown!" );
    }


    D3D11_TEXTURE2D_DESC t2desc;

    t2desc.Width        = _nWidth;
    t2desc.Height       = _nHeight;
    t2desc.MipLevels    = _bWithMips ? 0 : 1;
    t2desc.ArraySize    = 1;
    t2desc.Format       = (m_eType != eTextureType_DepthStencil) ? _dxFormat : DepthFormat;
    t2desc.SampleDesc.Count = 1;
    t2desc.SampleDesc.Quality = 0;
    t2desc.Usage = (m_eType != eTextureType_CPUWrite ) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
    t2desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    t2desc.CPUAccessFlags = (m_eType != eTextureType_CPUWrite) ? 0 : D3D11_CPU_ACCESS_WRITE;
    t2desc.MiscFlags = 0;

    if ( m_eType == eTextureType_RenderTarget )
        t2desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    else if ( m_eType == eTextureType_DepthStencil )
        t2desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

    g_renderManager->GetDevice()->CreateTexture2D( &t2desc, NULL, &m_pTexture );

    
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = SRVFormat;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MostDetailedMip = 0;
    srDesc.Texture2D.MipLevels = (UINT)-1;

    g_renderManager->GetDevice( )->CreateShaderResourceView( m_pTexture, &srDesc, &m_pTextureSRV );

    
    m_pTexture->GetDesc( &t2desc );
    m_nMipCount = t2desc.MipLevels;

    if ( m_eType == eTextureType_RenderTarget )
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = _dxFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        for ( int iRTVIdx = 0; iRTVIdx < m_nMipCount; iRTVIdx++ )
        {
            rtvDesc.Texture2D.MipSlice = iRTVIdx;

            g_renderManager->GetDevice( )->CreateRenderTargetView( m_pTexture, &rtvDesc, &m_pTextureRTV[iRTVIdx] );
        }
    }
    else if ( m_eType == eTextureType_DepthStencil )
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = _dxFormat;
        dsvDesc.Flags = 0;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        for ( int iDSVIdx = 0; iDSVIdx < m_nMipCount; iDSVIdx++ )
        {
            dsvDesc.Texture2D.MipSlice = iDSVIdx;

            g_renderManager->GetDevice( )->CreateDepthStencilView( m_pTexture, &dsvDesc, &m_pTextureDSV[iDSVIdx] );
        }
    }

    m_nWidth = _nWidth;
    m_nHeight = _nHeight;
}

//-------------------------------------------------------------------

bool Texture::Load( const WCHAR *_szFileName )
{
    m_szFileName = _wcsdup( _szFileName );

    HRESULT hr = D3DX11CreateTextureFromFile( g_renderManager->GetDevice( ), m_szFileName, NULL, NULL, (ID3D11Resource**)&m_pTexture, NULL );
    
    if ( FAILED( hr ) )
    {
        WCHAR msg[512];
        swprintf_s( msg, 512, L"Texture::Load - \"%s\" failed!", m_szFileName );
        myAssert( false, msg );
    }

    hr = g_renderManager->GetDevice( )->CreateShaderResourceView( m_pTexture, NULL, &m_pTextureSRV );

    D3D11_TEXTURE2D_DESC t2desc;
    m_pTexture->GetDesc( &t2desc );

    m_nWidth = t2desc.Width;
    m_nHeight = t2desc.Height;
    m_nMipCount = t2desc.MipLevels;

    return true;
}

//-------------------------------------------------------------------

void Texture::Unload( )
{
    SAFE_FREE( m_szFileName );

    for ( int iRTVIdx = 0; iRTVIdx < kMaxMipsRTV; iRTVIdx++ )
        SAFE_RELEASE( m_pTextureRTV[iRTVIdx] );
    for ( int iDSVIdx = 0; iDSVIdx < kMaxMipsRTV; iDSVIdx++ )
        SAFE_RELEASE( m_pTextureDSV[iDSVIdx] );
    SAFE_RELEASE( m_pTextureSRV );
    SAFE_RELEASE( m_pTexture );
}

//-------------------------------------------------------------------

void* Texture::Lock( UINT _nMipMapIdx, UINT &_nOutPitch )
{
    D3D11_MAPPED_SUBRESOURCE map2D;
    g_renderManager->GetDeviceContext( )->Map( m_pTexture, D3D11CalcSubresource( _nMipMapIdx, 0, 1 ), D3D11_MAP_WRITE_DISCARD, 0, &map2D );

    _nOutPitch = map2D.RowPitch;

    return map2D.pData;
}

//-------------------------------------------------------------------

void Texture::Unlock( UINT _nMipMapIdx )
{
    g_renderManager->GetDeviceContext( )->Unmap( m_pTexture, D3D11CalcSubresource( _nMipMapIdx, 0, 1 ) );
}

//-------------------------------------------------------------------

void* Texture::LockRead( UINT _nMipMapIdx, UINT &_nOutPitch )
{
    D3D11_TEXTURE2D_DESC t2desc;
    m_pTexture->GetDesc( &t2desc );

    t2desc.Usage = D3D11_USAGE_STAGING;
    t2desc.BindFlags = 0;
    t2desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    g_renderManager->GetDevice( )->CreateTexture2D( &t2desc, NULL, &m_pTmpCPUTexture );

    g_renderManager->GetDeviceContext( )->CopyResource( m_pTmpCPUTexture, m_pTexture );

    D3D11_MAPPED_SUBRESOURCE map2D;
    g_renderManager->GetDeviceContext( )->Map( m_pTmpCPUTexture, D3D11CalcSubresource( _nMipMapIdx, 0, 1 ), D3D11_MAP_READ, 0, &map2D );

    _nOutPitch = map2D.RowPitch;

    return map2D.pData;
}

//-------------------------------------------------------------------

void Texture::UnlockRead( UINT _nMipMapIdx )
{
    g_renderManager->GetDeviceContext( )->Unmap( m_pTmpCPUTexture, D3D11CalcSubresource( _nMipMapIdx, 0, 1 ) );

    SAFE_RELEASE( m_pTmpCPUTexture );
}

//-------------------------------------------------------------------

void Texture::VSSet( int _nSamplerId ) const
{
    g_renderManager->GetDeviceContext( )->VSSetShaderResources( _nSamplerId, 1, &m_pTextureSRV );
}

//-------------------------------------------------------------------

void Texture::PSSet( int _nSamplerId ) const
{
    g_renderManager->GetDeviceContext( )->PSSetShaderResources( _nSamplerId, 1, &m_pTextureSRV );
}

//-------------------------------------------------------------------

void Texture::CSSet( int _nSamplerId ) const
{
    g_renderManager->GetDeviceContext( )->CSSetShaderResources( _nSamplerId, 1, &m_pTextureSRV );
}

//-------------------------------------------------------------------

void Texture::VSSetNull( int _nSamplerId )
{
    ID3D11ShaderResourceView *nullSRV = NULL;
    g_renderManager->GetDeviceContext()->VSSetShaderResources( _nSamplerId, 1, &nullSRV );
}

//-------------------------------------------------------------------

void Texture::PSSetNull( int _nSamplerId )
{
    ID3D11ShaderResourceView *nullSRV = NULL;
    g_renderManager->GetDeviceContext()->PSSetShaderResources( _nSamplerId, 1, &nullSRV );
}

//-------------------------------------------------------------------

void Texture::CSSetNull( int _nSamplerId )
{
    ID3D11ShaderResourceView *nullSRV = NULL;
    g_renderManager->GetDeviceContext()->CSSetShaderResources( _nSamplerId, 1, &nullSRV );
}

//-------------------------------------------------------------------

void Texture::ClearRenderTarget( const D3DXCOLOR &_ClearColor )
{
    ClearRenderTarget( 0, _ClearColor );
}

//-------------------------------------------------------------------

void Texture::ClearRenderTarget( int _iMipIdx , const D3DXCOLOR &_ClearColor )
{
    if ( m_pTextureRTV[_iMipIdx] )
        g_renderManager->GetDeviceContext()->ClearRenderTargetView( m_pTextureRTV[_iMipIdx], _ClearColor );
}

//-------------------------------------------------------------------

void Texture::ClearDepthStencil( float _clearDepth )
{
    ClearDepthStencil( 0, _clearDepth );
}

//-------------------------------------------------------------------

void Texture::ClearDepthStencil( int _iMipIdx, float _clearDepth )
{
    if ( m_pTextureDSV[_iMipIdx] )
        g_renderManager->GetDeviceContext()->ClearDepthStencilView( m_pTextureDSV[_iMipIdx], D3D11_CLEAR_DEPTH, _clearDepth, 0 );
}

//-------------------------------------------------------------------
