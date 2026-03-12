#pragma once

#ifndef __SHADER_H__
#define __SHADER_H__

/// @file Shader.h
/// @brief HLSL shader wrappers (VS, PS, GS, CS) with hot-reload support.

#include "AutoList.h"
#include "d3d11.h"

//===================================================================
//	CLASS Shader
//===================================================================

/// @class Shader
/// @brief Abstract base class for all HLSL shader types.
///
/// Inherits @ref AutoList<Shader> so that @ref ReloadAll() can iterate every
/// live shader instance and recompile shaders whose source file has changed on
/// disk, enabling hot-reload during development.
///
/// Derived classes (@ref ShaderVS, @ref ShaderPS, @ref ShaderGS, @ref ShaderCS)
/// implement @ref Load, @ref Unload, and @ref Set.
class Shader : public AutoList<Shader>
{
public:
    /// @brief Reloads all live shader instances whose source files have changed.
    ///
    /// Iterates the @ref AutoList and calls @ref Load on each shader for which
    /// @ref bIsFileChanged returns @c true.
    static void                     ReloadAll               ( );

    /// @brief Releases all live shader instances' GPU resources.
    static void                     UnloadAll               ( );

    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs the shader and records the source file path.
    /// @param _szFileName Wide-character path to the HLSL source file.
    Shader                          ( const WCHAR *_szFileName );
    virtual ~Shader                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Compiles (or recompiles) the shader from its source file.
    /// @return S_OK on success; an HRESULT error code on failure.
    virtual HRESULT                 Load                    ( ) = 0;

    /// @brief Releases the compiled shader byte-code and any GPU resources.
    virtual void                    Unload                  ( ) = 0;

    /// @brief Binds this shader to the corresponding pipeline stage.
    virtual void                    Set                     ( ) = 0;

    /// @brief Records the current on-disk modification time of the source file.
    ///
    /// Called after a successful @ref Load so that subsequent @ref bIsFileChanged
    /// calls can detect edits.
    void                            SaveLastModifiedTime    ( );

    /// @brief Returns @c true if the source file has been modified since the last @ref Load.
    bool                            bIsFileChanged          ( );

    /// @brief Returns a pointer to the compiled shader byte-code buffer.
    LPVOID                          GetBufferPointer        ( ) { return m_shaderBuffer->GetBufferPointer(); }

    /// @brief Returns the size of the compiled shader byte-code buffer in bytes.
    int                             GetBufferSize           ( ) { return m_shaderBuffer->GetBufferSize(); }

protected:
    WCHAR*                          m_szFileName;       ///< Source file path.
    FILETIME                        m_lastModifiedDate; ///< Modification time at last successful Load().
    ID3DBlob*                       m_shaderBuffer;     ///< Compiled byte-code blob.

};

//===================================================================
//	CLASS ShaderVS
//===================================================================

/// @class ShaderVS
/// @brief Vertex shader wrapper.
class ShaderVS : public Shader
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    ShaderVS                        ( const WCHAR *_szFileName ) : Shader( _szFileName ), m_vertexShader( NULL ) { }
    virtual ~ShaderVS               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Compiles and creates the vertex shader from @c m_szFileName.
    virtual HRESULT                 Load                    ( );

    /// @brief Releases the vertex shader GPU resource.
    virtual void                    Unload                  ( );

    /// @brief Binds this vertex shader to the VS stage.
    virtual void                    Set                     ( );

    /// @brief Unbinds any vertex shader from the VS stage (sets @c nullptr).
    static void                     Unset                   ( );

protected:
    ID3D11VertexShader*             m_vertexShader;

};

//===================================================================
//	CLASS ShaderPS
//===================================================================

/// @class ShaderPS
/// @brief Pixel (fragment) shader wrapper.
class ShaderPS : public Shader
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    ShaderPS                        ( const WCHAR *_szFileName ) : Shader( _szFileName ), m_pixelShader( NULL ) { }
    virtual ~ShaderPS               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Compiles and creates the pixel shader from @c m_szFileName.
    virtual HRESULT                 Load                    ( );

    /// @brief Releases the pixel shader GPU resource.
    virtual void                    Unload                  ( );

    /// @brief Binds this pixel shader to the PS stage.
    virtual void                    Set                     ( );

    /// @brief Unbinds any pixel shader from the PS stage (sets @c nullptr).
    static  void                    Unset                   ( );

protected:
    ID3D11PixelShader*              m_pixelShader;

};

//===================================================================
//	CLASS ShaderGS
//===================================================================

/// @class ShaderGS
/// @brief Geometry shader wrapper, with optional stream-output support.
class ShaderGS : public Shader
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs a geometry shader without stream-output.
    ShaderGS                        ( const WCHAR *_szFileName ) : Shader( _szFileName ), m_geometryShader( NULL ), m_SODeclaration( NULL ), m_NumEntries( 0 ), m_bufferStrides( NULL ), m_NumStrides( 0 ), m_RasterizedStream( 0 ) { }

    /// @brief Constructs a geometry shader with stream-output configuration.
    ///
    /// @param _szFileName       Source file path.
    /// @param pSODeclaration    Array of stream-output declaration entries.
    /// @param NumEntries        Number of entries in @p pSODeclaration.
    /// @param pBufferStrides    Array of stream-output buffer strides.
    /// @param NumStrides        Number of entries in @p pBufferStrides.
    /// @param RasterizedStream  Index of the stream sent to the rasteriser.
    ShaderGS                        ( const WCHAR *_szFileName, const D3D11_SO_DECLARATION_ENTRY *pSODeclaration, UINT NumEntries, const UINT *pBufferStrides, UINT NumStrides, UINT RasterizedStream );
    virtual ~ShaderGS               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Compiles and creates the geometry shader (with stream-output if configured).
    virtual HRESULT                 Load                    ( );

    /// @brief Releases the geometry shader GPU resource.
    virtual void                    Unload                  ( );

    /// @brief Binds this geometry shader to the GS stage.
    virtual void                    Set                     ( );

    /// @brief Unbinds any geometry shader from the GS stage (sets @c nullptr).
    static  void                    Unset                   ( );

protected:
    ID3D11GeometryShader*           m_geometryShader;

    D3D11_SO_DECLARATION_ENTRY*     m_SODeclaration;    ///< Stream-output declaration (may be nullptr).
    UINT                            m_NumEntries;
    UINT*                           m_bufferStrides;
    UINT                            m_NumStrides;
    UINT                            m_RasterizedStream;
};

//===================================================================
//	CLASS ShaderCS
//===================================================================

/// @class ShaderCS
/// @brief Compute shader wrapper.
class ShaderCS : public Shader
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    ShaderCS                        ( const WCHAR *_szFileName ) : Shader( _szFileName ), m_computeShader( NULL ) { }
    virtual ~ShaderCS               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Compiles and creates the compute shader from @c m_szFileName.
    virtual HRESULT                 Load                    ( );

    /// @brief Releases the compute shader GPU resource.
    virtual void                    Unload                  ( );

    /// @brief Binds this compute shader to the CS stage.
    virtual void                    Set                     ( );

    /// @brief Unbinds any compute shader from the CS stage (sets @c nullptr).
    static  void                    Unset                   ( );

protected:
    ID3D11ComputeShader*             m_computeShader;

};

#endif // __SHADER_H__
