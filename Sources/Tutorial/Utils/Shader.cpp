#include "pch.h"

#include "Shader.h"

#include "D3DX11async.h"
#include "D3Dcompiler.h"

#include "Utils.h"

#include "..\Render\RenderManager.h"

//-------------------------------------------------------------------

#define SHADER_MODEL_VS     "vs_5_0"
#define SHADER_MODEL_PS     "ps_5_0"
#define SHADER_MODEL_GS     "gs_5_0"
#define SHADER_MODEL_CS     "cs_5_0"

HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

//===================================================================
//	CLASS Shader
//===================================================================

//-------------------------------------------------------------------

void Shader::ReloadAll( )
{
    Shader *pShaderIt = GetGlobalListHead();
    while ( pShaderIt )
    {
        if ( pShaderIt->bIsFileChanged() )
            pShaderIt->Load();

        pShaderIt = pShaderIt->GetNext();
    }
}

//-------------------------------------------------------------------

void Shader::UnloadAll( )
{
    Shader *pShaderIt = GetGlobalListHead();
    while ( pShaderIt )
    {
        pShaderIt->Unload();

        pShaderIt = pShaderIt->GetNext();
    }
}

//-------------------------------------------------------------------

Shader::Shader( const WCHAR *_szFileName )
    : m_szFileName( NULL )    
    , m_shaderBuffer( NULL )
{
    m_szFileName = _wcsdup( _szFileName );
}

//-------------------------------------------------------------------

Shader::~Shader( )
{
    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void Shader::SaveLastModifiedTime( )
{
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData = {0};
    GetFileAttributesEx( m_szFileName, GetFileExInfoStandard, &fileAttrData );
    m_lastModifiedDate = fileAttrData.ftLastWriteTime;
}

//-------------------------------------------------------------------

bool Shader::bIsFileChanged( )
{
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData = { 0 };
    GetFileAttributesEx(m_szFileName, GetFileExInfoStandard, &fileAttrData);

    return ( CompareFileTime( &m_lastModifiedDate, &fileAttrData.ftLastWriteTime ) < 0 );
}

//-------------------------------------------------------------------

//===================================================================
//	CLASS ShaderVS
//===================================================================

//-------------------------------------------------------------------

ShaderVS::~ShaderVS( )
{
    Unload();
}

//-------------------------------------------------------------------

HRESULT ShaderVS::Load( )
{
    HRESULT hr = S_OK;

    Unload();

    SaveLastModifiedTime( );

    V_RETURN( CompileShaderFromFile( m_szFileName, "main", SHADER_MODEL_VS, &m_shaderBuffer ) );

    V_RETURN( g_renderManager->GetDevice()->CreateVertexShader( m_shaderBuffer->GetBufferPointer( ), m_shaderBuffer->GetBufferSize( ), NULL, &m_vertexShader ) );

    return S_OK;
}

//-------------------------------------------------------------------

void ShaderVS::Unload( )
{
    SAFE_RELEASE( m_vertexShader );
    SAFE_RELEASE( m_shaderBuffer );
}

//-------------------------------------------------------------------

void ShaderVS::Set( )
{
    g_renderManager->GetDeviceContext( )->VSSetShader( m_vertexShader, NULL, 0 );
}

//-------------------------------------------------------------------

void ShaderVS::Unset( )
{
    g_renderManager->GetDeviceContext( )->VSSetShader( NULL, NULL, 0 );
}

//-------------------------------------------------------------------

//===================================================================
//	CLASS ShaderPS
//===================================================================

//-------------------------------------------------------------------

ShaderPS::~ShaderPS( )
{
    Unload();
}

//-------------------------------------------------------------------

HRESULT ShaderPS::Load( )
{
    HRESULT hr = S_OK;

    Unload();

    SaveLastModifiedTime( );

    m_shaderBuffer = NULL;
    V_RETURN( CompileShaderFromFile( m_szFileName, "main", SHADER_MODEL_PS, &m_shaderBuffer ) );

    V_RETURN( g_renderManager->GetDevice( )->CreatePixelShader( m_shaderBuffer->GetBufferPointer( ), m_shaderBuffer->GetBufferSize( ), NULL, &m_pixelShader ) );

    return S_OK;
}

//-------------------------------------------------------------------

void ShaderPS::Unload( )
{
    SAFE_RELEASE( m_pixelShader );
    SAFE_RELEASE( m_shaderBuffer );
}

//-------------------------------------------------------------------

void ShaderPS::Set( )
{
    g_renderManager->GetDeviceContext( )->PSSetShader( m_pixelShader, NULL, 0 );
}

//-------------------------------------------------------------------

void ShaderPS::Unset( )
{
    g_renderManager->GetDeviceContext( )->PSSetShader( NULL, NULL, 0 );
}

//-------------------------------------------------------------------

//===================================================================
//	CLASS ShaderGS
//===================================================================

//-------------------------------------------------------------------

ShaderGS::ShaderGS( const WCHAR *_szFileName, const D3D11_SO_DECLARATION_ENTRY *pSODeclaration, UINT NumEntries, const UINT *pBufferStrides, UINT NumStrides, UINT RasterizedStream )
    : Shader( _szFileName )
    , m_geometryShader( NULL )
    , m_SODeclaration( NULL )
    , m_NumEntries( NumEntries )
    , m_bufferStrides( NULL )
    , m_NumStrides( NumStrides )
    , m_RasterizedStream( RasterizedStream )
{
    m_SODeclaration = (D3D11_SO_DECLARATION_ENTRY*)malloc( NumEntries * sizeof(D3D11_SO_DECLARATION_ENTRY) );
    m_bufferStrides = (UINT*)malloc( NumStrides * sizeof(UINT) );
    for ( UINT i = 0; i < m_NumEntries; i++ )
        m_SODeclaration[i] = pSODeclaration[i];
    for ( UINT i = 0; i < m_NumStrides; i++ )
        m_bufferStrides[i] = pBufferStrides[i];
}

//-------------------------------------------------------------------

ShaderGS::~ShaderGS( )
{
    Unload();

    SAFE_FREE( m_SODeclaration );
    SAFE_FREE( m_bufferStrides );
}

//-------------------------------------------------------------------

HRESULT ShaderGS::Load( )
{
    HRESULT hr = S_OK;

    Unload();

    SaveLastModifiedTime( );

    V_RETURN( CompileShaderFromFile( m_szFileName, "main", SHADER_MODEL_GS, &m_shaderBuffer ) );

    if ( m_SODeclaration )
    {
        V_RETURN( g_renderManager->GetDevice( )->CreateGeometryShaderWithStreamOutput( m_shaderBuffer->GetBufferPointer( ), m_shaderBuffer->GetBufferSize( ), m_SODeclaration, m_NumEntries, m_bufferStrides, m_NumStrides, m_RasterizedStream, NULL, &m_geometryShader ) );
    }
    else
    {
        V_RETURN( g_renderManager->GetDevice( )->CreateGeometryShader( m_shaderBuffer->GetBufferPointer( ), m_shaderBuffer->GetBufferSize( ), NULL, &m_geometryShader ) );
    }

    return S_OK;
}

//-------------------------------------------------------------------

void ShaderGS::Unload( )
{
    SAFE_RELEASE( m_geometryShader );
    SAFE_RELEASE( m_shaderBuffer );
}

//-------------------------------------------------------------------

void ShaderGS::Set( )
{
    g_renderManager->GetDeviceContext( )->GSSetShader( m_geometryShader, NULL, 0 );
}

//-------------------------------------------------------------------

void ShaderGS::Unset( )
{
    g_renderManager->GetDeviceContext( )->GSSetShader( NULL, NULL, 0 );
}

//-------------------------------------------------------------------

//===================================================================
//	CLASS ShaderCS
//===================================================================

//-------------------------------------------------------------------

ShaderCS::~ShaderCS( )
{
    Unload();
}

//-------------------------------------------------------------------

HRESULT ShaderCS::Load( )
{
    HRESULT hr = S_OK;

    Unload();

    SaveLastModifiedTime( );

    V_RETURN( CompileShaderFromFile( m_szFileName, "main", SHADER_MODEL_CS, &m_shaderBuffer ) );

    V_RETURN( g_renderManager->GetDevice( )->CreateComputeShader( m_shaderBuffer->GetBufferPointer( ), m_shaderBuffer->GetBufferSize( ), NULL, &m_computeShader ) );

    return S_OK;
}

//-------------------------------------------------------------------

void ShaderCS::Unload( )
{
    SAFE_RELEASE( m_computeShader );
    SAFE_RELEASE( m_shaderBuffer );
}

//-------------------------------------------------------------------

void ShaderCS::Set( )
{
    g_renderManager->GetDeviceContext( )->CSSetShader( m_computeShader, NULL, 0 );
}

//-------------------------------------------------------------------

void ShaderCS::Unset( )
{
    g_renderManager->GetDeviceContext( )->CSSetShader( NULL, NULL, 0 );
}

//-------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if ( pErrorBlob != NULL )
        {
            WCHAR text_wchar[512];
            ConvertStringToWideString( ( char* )pErrorBlob->GetBufferPointer( ), text_wchar );

            myAssert( false, text_wchar );
        }
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

//-------------------------------------------------------------------
