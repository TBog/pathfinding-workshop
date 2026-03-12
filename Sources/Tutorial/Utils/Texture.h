#pragma once

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "RefCounted.h"

enum ETextureType
{
    eTextureType_Default,
    eTextureType_CPUWrite,
    eTextureType_RenderTarget,
    eTextureType_DepthStencil,
};

//===================================================================
//	CLASS Texture
//===================================================================

class Texture : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Texture                         ( const WCHAR *_szFileName );
    Texture                         ( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType );
    virtual ~Texture                ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            Create                  ( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType );

    bool                            Load                    ( const WCHAR *_szFileName );
    void                            Unload                  ( );

    int                             GetMipCount             ( )                     { return m_nMipCount; }

    void*                           Lock                    ( UINT _nMipMapIdx, UINT &_nOutPitch );
    void                            Unlock                  ( UINT _nMipMapIdx );

    void*                           LockRead                ( UINT _nMipMapIdx, UINT &_nOutPitch );
    void                            UnlockRead              ( UINT _nMipMapIdx );

    int                             GetWidth                ( )                     { return m_nWidth; }
    int                             GetHeight               ( )                     { return m_nHeight; }

    void                            VSSet                   ( int _nSamplerId )     const;
    void                            PSSet                   ( int _nSamplerId )     const;
    void                            CSSet                   ( int _nSamplerId )     const;

    static void                     VSSetNull               ( int _nSamplerId );
    static void                     PSSetNull               ( int _nSamplerId );
    static void                     CSSetNull               ( int _nSamplerId );

    void                            ClearRenderTarget       ( const D3DXCOLOR &_ClearColor );
    void                            ClearRenderTarget       ( int _iMipIdx , const D3DXCOLOR &_ClearColor );
    void                            ClearDepthStencil       ( float _clearDepth );
    void                            ClearDepthStencil       ( int _iMipIdx , float _clearDepth );

    ID3D11RenderTargetView*         GetRenderTargetView     ( )                     { return m_pTextureRTV[0]; }
    ID3D11RenderTargetView*         GetRenderTargetView     ( int _iMipIdx )        { return m_pTextureRTV[_iMipIdx]; }

    ID3D11DepthStencilView*         GetDepthStencilView     ( )                     { return m_pTextureDSV[0]; }
    ID3D11DepthStencilView*         GetDepthStencilView     ( int _iMipIdx )        { return m_pTextureDSV[_iMipIdx]; }

protected:
    ETextureType                    m_eType;

    WCHAR*                          m_szFileName;
    ID3D11ShaderResourceView*       m_pTextureSRV;
    static const int                kMaxMipsRTV = 16;
    ID3D11RenderTargetView*         m_pTextureRTV[kMaxMipsRTV];
    ID3D11DepthStencilView*         m_pTextureDSV[kMaxMipsRTV];
    ID3D11Texture2D*                m_pTexture;
    ID3D11Texture2D*                m_pTmpCPUTexture;

    int                             m_nWidth;
    int                             m_nHeight;
    
    int                             m_nMipCount;

};

#endif // __TEXTURE_H__
