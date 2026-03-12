#pragma once

#ifndef __SHADER_H__
#define __SHADER_H__

#include "AutoList.h"
#include "d3d11.h"

//===================================================================
//	CLASS Shader
//===================================================================

class Shader : public AutoList<Shader>
{
public:
    static void                     ReloadAll               ( );
    static void                     UnloadAll               ( );

    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Shader                          ( const WCHAR *_szFileName );
    virtual ~Shader                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    virtual HRESULT                 Load                    ( ) = 0;
    virtual void                    Unload                  ( ) = 0;
    virtual void                    Set                     ( ) = 0;

    void                            SaveLastModifiedTime    ( );
    bool                            bIsFileChanged          ( );

    LPVOID                          GetBufferPointer        ( ) { return m_shaderBuffer->GetBufferPointer(); }
    int                             GetBufferSize           ( ) { return m_shaderBuffer->GetBufferSize(); }

protected:
    WCHAR*                          m_szFileName;
    FILETIME                        m_lastModifiedDate;
    ID3DBlob*                       m_shaderBuffer;

};

//===================================================================
//	CLASS ShaderVS
//===================================================================

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
    virtual HRESULT                 Load                    ( );
    virtual void                    Unload                  ( );
    virtual void                    Set                     ( );
    static void                     Unset                   ( );

protected:
    ID3D11VertexShader*             m_vertexShader;

};

//===================================================================
//	CLASS ShaderPS
//===================================================================

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
    virtual HRESULT                 Load                    ( );
    virtual void                    Unload                  ( );
    virtual void                    Set                     ( );
    static  void                    Unset                   ( );

protected:
    ID3D11PixelShader*              m_pixelShader;

};

//===================================================================
//	CLASS ShaderGS
//===================================================================

class ShaderGS : public Shader
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    ShaderGS                        ( const WCHAR *_szFileName ) : Shader( _szFileName ), m_geometryShader( NULL ), m_SODeclaration( NULL ), m_NumEntries( 0 ), m_bufferStrides( NULL ), m_NumStrides( 0 ), m_RasterizedStream( 0 ) { }
    ShaderGS                        ( const WCHAR *_szFileName, const D3D11_SO_DECLARATION_ENTRY *pSODeclaration, UINT NumEntries, const UINT *pBufferStrides, UINT NumStrides, UINT RasterizedStream );
    virtual ~ShaderGS               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    virtual HRESULT                 Load                    ( );
    virtual void                    Unload                  ( );
    virtual void                    Set                     ( );
    static  void                    Unset                   ( );

protected:
    ID3D11GeometryShader*           m_geometryShader;

    D3D11_SO_DECLARATION_ENTRY*     m_SODeclaration;
    UINT                            m_NumEntries;
    UINT*                           m_bufferStrides;
    UINT                            m_NumStrides;
    UINT                            m_RasterizedStream;
};

//===================================================================
//	CLASS ShaderCS
//===================================================================

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
    virtual HRESULT                 Load                    ( );
    virtual void                    Unload                  ( );
    virtual void                    Set                     ( );
    static  void                    Unset                   ( );

protected:
    ID3D11ComputeShader*             m_computeShader;

};

#endif // __SHADER_H__
