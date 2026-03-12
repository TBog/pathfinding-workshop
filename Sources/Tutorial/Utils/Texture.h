#pragma once

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

/// @file Texture.h
/// @brief Direct3D 11 texture wrapper with support for file textures,
///        render targets, depth-stencil surfaces, and CPU-writable textures.

#include "RefCounted.h"

/// @enum ETextureType
/// @brief Specifies the intended usage of a @ref Texture.
enum ETextureType
{
    eTextureType_Default,       ///< Read-only texture sampled by shaders (loaded from file or initialised on GPU).
    eTextureType_CPUWrite,      ///< CPU-writable texture (uses a staging texture for Lock/Unlock).
    eTextureType_RenderTarget,  ///< Colour render target (can also be sampled as a shader resource).
    eTextureType_DepthStencil,  ///< Depth/stencil surface (can be read back as a shader resource where hardware supports it).
};

//===================================================================
//	CLASS Texture
//===================================================================

/// @class Texture
/// @brief Wraps a @c ID3D11Texture2D along with its shader-resource, render-target,
///        and depth-stencil views.
///
/// Supports four usage modes (see @ref ETextureType):
/// - **Default** — loaded from a file via D3DX.
/// - **CPUWrite** — backed by a staging texture; call @ref Lock / @ref Unlock
///                  to map/unmap for CPU writes.
/// - **RenderTarget** — exposes @ref GetRenderTargetView per mip level.
/// - **DepthStencil** — exposes @ref GetDepthStencilView per mip level.
///
/// Inherits @ref RefCounted.  Use @c SAFE_RELEASE() to decrement the ref count.
class Texture : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs and immediately loads a texture from @p _szFileName.
    /// @param _szFileName Wide-character path to the image file.
    Texture                         ( const WCHAR *_szFileName );

    /// @brief Constructs and immediately creates a GPU texture with the specified parameters.
    /// @param _nWidth     Width in texels.
    /// @param _nHeight    Height in texels.
    /// @param _dxFormat   DXGI format.
    /// @param _bWithMips  If @c true, a full mip chain is allocated.
    /// @param _eType      Usage type (see @ref ETextureType).
    Texture                         ( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType );
    virtual ~Texture                ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Creates (or re-creates) the GPU texture with the given parameters.
    /// @param _nWidth     Width in texels.
    /// @param _nHeight    Height in texels.
    /// @param _dxFormat   DXGI format.
    /// @param _bWithMips  If @c true, a full mip chain is allocated.
    /// @param _eType      Usage type.
    void                            Create                  ( int _nWidth, int _nHeight, DXGI_FORMAT _dxFormat, bool _bWithMips, ETextureType _eType );

    /// @brief Loads the texture from a file using D3DX.
    /// @param _szFileName Wide-character path to the image file.
    /// @return @c true on success.
    bool                            Load                    ( const WCHAR *_szFileName );

    /// @brief Releases all GPU resources.
    void                            Unload                  ( );

    /// @brief Returns the number of mip levels in the texture.
    int                             GetMipCount             ( )                     { return m_nMipCount; }

    /// @brief Maps the CPU-writable staging texture for a specific mip level.
    ///
    /// Only valid for @c eTextureType_CPUWrite textures.
    /// @param _nMipMapIdx  Mip level to map (0 = highest resolution).
    /// @param _nOutPitch   [out] Row pitch in bytes for the mapped surface.
    /// @return Pointer to the mapped texel data, or @c nullptr on failure.
    void*                           Lock                    ( UINT _nMipMapIdx, UINT &_nOutPitch );

    /// @brief Unmaps the CPU-writable staging texture and uploads the data to the GPU.
    /// @param _nMipMapIdx Mip level to unmap.
    void                            Unlock                  ( UINT _nMipMapIdx );

    /// @brief Maps the texture for CPU read access (e.g. for read-back).
    /// @param _nMipMapIdx  Mip level to map.
    /// @param _nOutPitch   [out] Row pitch in bytes.
    /// @return Pointer to the read-only texel data.
    void*                           LockRead                ( UINT _nMipMapIdx, UINT &_nOutPitch );

    /// @brief Unmaps a read-locked mip level.
    /// @param _nMipMapIdx Mip level to unmap.
    void                            UnlockRead              ( UINT _nMipMapIdx );

    /// @brief Returns the texture width in texels.
    int                             GetWidth                ( )                     { return m_nWidth; }

    /// @brief Returns the texture height in texels.
    int                             GetHeight               ( )                     { return m_nHeight; }

    /// @brief Binds the shader-resource view to a vertex shader texture slot.
    /// @param _nSamplerId Texture slot index.
    void                            VSSet                   ( int _nSamplerId )     const;

    /// @brief Binds the shader-resource view to a pixel shader texture slot.
    /// @param _nSamplerId Texture slot index.
    void                            PSSet                   ( int _nSamplerId )     const;

    /// @brief Binds the shader-resource view to a compute shader texture slot.
    /// @param _nSamplerId Texture slot index.
    void                            CSSet                   ( int _nSamplerId )     const;

    /// @brief Binds @c nullptr to a vertex shader texture slot (unbinds).
    /// @param _nSamplerId Texture slot index.
    static void                     VSSetNull               ( int _nSamplerId );

    /// @brief Binds @c nullptr to a pixel shader texture slot (unbinds).
    /// @param _nSamplerId Texture slot index.
    static void                     PSSetNull               ( int _nSamplerId );

    /// @brief Binds @c nullptr to a compute shader texture slot (unbinds).
    /// @param _nSamplerId Texture slot index.
    static void                     CSSetNull               ( int _nSamplerId );

    /// @brief Clears the render target at mip 0 to @p _ClearColor.
    /// @param _ClearColor RGBA clear colour.
    void                            ClearRenderTarget       ( const D3DXCOLOR &_ClearColor );

    /// @brief Clears the render target at mip @p _iMipIdx to @p _ClearColor.
    void                            ClearRenderTarget       ( int _iMipIdx , const D3DXCOLOR &_ClearColor );

    /// @brief Clears the depth-stencil surface at mip 0 to @p _clearDepth.
    /// @param _clearDepth Depth value to write (typically @c 1.0f for a reversed-Z setup).
    void                            ClearDepthStencil       ( float _clearDepth );

    /// @brief Clears the depth-stencil surface at mip @p _iMipIdx to @p _clearDepth.
    void                            ClearDepthStencil       ( int _iMipIdx , float _clearDepth );

    /// @brief Returns the render target view for mip level 0.
    ID3D11RenderTargetView*         GetRenderTargetView     ( )                     { return m_pTextureRTV[0]; }

    /// @brief Returns the render target view for the specified mip level.
    ID3D11RenderTargetView*         GetRenderTargetView     ( int _iMipIdx )        { return m_pTextureRTV[_iMipIdx]; }

    /// @brief Returns the depth-stencil view for mip level 0.
    ID3D11DepthStencilView*         GetDepthStencilView     ( )                     { return m_pTextureDSV[0]; }

    /// @brief Returns the depth-stencil view for the specified mip level.
    ID3D11DepthStencilView*         GetDepthStencilView     ( int _iMipIdx )        { return m_pTextureDSV[_iMipIdx]; }

protected:
    ETextureType                    m_eType;

    WCHAR*                          m_szFileName;
    ID3D11ShaderResourceView*       m_pTextureSRV;          ///< Shader-resource view.
    static const int                kMaxMipsRTV = 16;
    ID3D11RenderTargetView*         m_pTextureRTV[kMaxMipsRTV]; ///< Per-mip render target views.
    ID3D11DepthStencilView*         m_pTextureDSV[kMaxMipsRTV]; ///< Per-mip depth-stencil views.
    ID3D11Texture2D*                m_pTexture;             ///< The underlying GPU texture.
    ID3D11Texture2D*                m_pTmpCPUTexture;       ///< Staging texture for CPU read/write.

    int                             m_nWidth;
    int                             m_nHeight;
    
    int                             m_nMipCount;

};

#endif // __TEXTURE_H__
