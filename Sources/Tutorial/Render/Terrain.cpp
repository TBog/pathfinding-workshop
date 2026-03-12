#include "pch.h"

#include "Terrain.h"

#include "..\Extern\tinyxml2.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"
#include "..\Utils\Mesh.h"
#include "..\Utils\Texture.h"
#include "..\Utils\Camera.h"

#include "RenderManager.h"
#include "TexturesManager.h"


ShaderVS g_TerrainVS(L"Shaders/Terrain_VS.fx");
ShaderPS g_TerrainPS(L"Shaders/Terrain_PS.fx");


struct st_Terrain_CB_VS_PER_OBJECT
{
    float                   m_patchStartPosX;
    float                   m_patchStartPosZ;
    float                   m_patchSizeX;
    float                   m_patchSizeZ;
    D3DXVECTOR4             m_heightMapInfo;
};

struct st_Terrain_CB_PS_PER_OBJECT
{
    float                   m_tile;
    float                   m_noiseTile;
};

//-------------------------------------------------------------------

Terrain::Terrain( const WCHAR *_szFileName )
    : m_szFileName      ( NULL )
    , m_center          ( 0.f, 0.f, 0.f )
    , m_bottomLeft      ( 0.f, 0.f, 0.f )
    , m_size            ( 0.f )
    , m_heightTexture   ( NULL )
    , m_normalsTexture  ( NULL )
    , m_heightData      ( NULL )
    , m_heightData_width    ( 0 )
    , m_heightData_height   ( 0 )
    , m_heightScalator  ( 1.f )
    , m_terrainPatchesGrid( 16, 16 )
    , m_terrainPatchesGridSize( 0 )
    , m_terrainVertexLayout( NULL )
    , m_patchMesh       ( NULL )
    , m_colorMapTexture ( NULL )
    , m_texture         ( NULL )
    , m_baseTexture     ( NULL )
    , m_noiseNormalMapTexture( NULL )
    , m_noiseTexture    ( NULL )
    , m_textureTile     ( 32.f )
    , m_noiseTextureTile( 1.f )
{
    Load( _szFileName );
}

//-------------------------------------------------------------------

Terrain::~Terrain( )
{
    Unload( );
}

//-------------------------------------------------------------------

bool Terrain::Load( const WCHAR *_szFileName )
{
    Unload ();

    m_szFileName = _wcsdup( _szFileName );

    // convert file name to char*
    char szFileNameChar[256];
    ConvertWdieStringToString( m_szFileName, szFileNameChar );

    // load the terrain xml file
    tinyxml2::XMLDocument terrainFile;
    terrainFile.LoadFile( szFileNameChar );
    if ( terrainFile.ErrorID() != 0 )
    {
        WCHAR msg[512];
        swprintf_s( msg, 512, L"Terrain::Load() - \"%s\" failed!", m_szFileName );
        myAssert( false, msg );

        return false;
    }

    tinyxml2::XMLElement *terrainElement = terrainFile.FirstChildElement( "terrain" );

    terrainElement->QueryFloatAttribute( "centerX", &m_center.x );
    terrainElement->QueryFloatAttribute( "centerY", &m_center.y );
    terrainElement->QueryFloatAttribute( "centerZ", &m_center.z );

    terrainElement->QueryFloatAttribute( "size", &m_size );

    m_bottomLeft.x = m_center.x - m_size * 0.5f;
    m_bottomLeft.y = m_center.y;
    m_bottomLeft.z = m_center.z - m_size * 0.5f;

    terrainElement->QueryFloatAttribute( "heightScalator", &m_heightScalator );
    
    terrainElement->QueryIntAttribute( "gridSize", &m_terrainPatchesGridSize );

    const char *szHeightMapFileName = terrainElement->Attribute( "heightMapFileName" );
    _LoadHeightTexture( szHeightMapFileName );
    
    
    _CreateRenderData( );

    const char *szColorMapFileName = terrainElement->Attribute( "colorMapFileName" );
    m_colorMapTexture = g_texturesManager->Load( szColorMapFileName );


    return true;
}

//-------------------------------------------------------------------

void Terrain::Unload( )
{
    SAFE_RELEASE( m_colorMapTexture );

    _DestroyRenderData( );

    _UnloadHeightData( );

    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void Terrain::GetMinMaxHeights( const D3DXVECTOR3 &_startPos, const D3DXVECTOR3 &_size, float &_outMinHeight, float &_outMaxHeight )
{
    _outMinHeight = 10000.f;
    _outMaxHeight = -10000.f;

    float fRow = m_heightData_height * (1.f - (_startPos.z + _size.z - m_bottomLeft.z) / m_size);
    float fCol = m_heightData_width * ((_startPos.x - m_bottomLeft.x) / m_size);
    int nRow = (int)(fRow);
    int nCol = (int)(fCol);
    int n = (int)(m_heightData_width * _size.x / m_size) + 1;
    int nRow_1 = min(nRow + n, m_heightData_height - 1);
    int nCol_1 = min(nCol + n, m_heightData_width - 1);

    for ( int nR = nRow; nR <= nRow_1; nR++ )
    {
        for ( int nC = nCol; nC <= nCol_1; nC++ )
        {
            _outMinHeight = min( _outMinHeight, m_heightData[ nR * m_heightData_width + nC ] );
            _outMaxHeight = max( _outMaxHeight, m_heightData[ nR * m_heightData_width + nC ] );
        }
    }
}

//-------------------------------------------------------------------

void Terrain::_LoadHeightTexture( const char *_szFileName )
{
    int n = strlen( _szFileName );
    if (_szFileName[n - 1] == 'w')  // if the extension is '.raw'
    {
        _LoadHeightFromRaw16( _szFileName );
    }
    else
    {
        WCHAR wszFileName[256];
        ConvertStringToWideString( _szFileName, wszFileName );
        
        Texture *refTexture = new Texture( wszFileName );

        _BuildHeightAndNormalsTextures( refTexture );

        SAFE_DELETE( refTexture );
    }

    _BuildTerrainPatchesGrid( );
}

//-------------------------------------------------------------------

void Terrain::_LoadHeightFromRaw16( const char *_szFileName )
{
    FILE *file = NULL;
    fopen_s( &file, _szFileName, "rb" );
    fseek( file, 0, SEEK_END );
    int fileSize = ftell( file );
    fseek( file, 0, SEEK_SET );

    WORD *data16 = (WORD*)malloc( fileSize );
    int n = (int)sqrtf( fileSize / 2.f );

    fread_s( data16, fileSize, 1, fileSize, file );

    m_heightData_width = m_heightData_height = n;

    _BuildHeightAndNormalsTextures( data16, n, n );

    free( data16 );

    fclose( file );
}

//-------------------------------------------------------------------

void Terrain::_UnloadHeightData()
{
    SAFE_DELETE( m_heightTexture );
    SAFE_DELETE( m_normalsTexture );

    SAFE_FREE( m_heightData );
}

//-------------------------------------------------------------------

void Terrain::_BuildHeightAndNormalsTextures( Texture *_refTexture )
{
    m_heightData_width = _refTexture->GetWidth( );
    m_heightData_height = _refTexture->GetHeight( );

    m_heightData = (float*)malloc( m_heightData_width * m_heightData_height * sizeof(float) );

    BYTE *refHeightData = NULL;
    UINT refHeightTexturePitch = 0;

    refHeightData = (BYTE*)_refTexture->LockRead( 0, refHeightTexturePitch );

    {
        float *heightData = NULL;
        UINT heightTexturePitch = 0;
        
        m_heightTexture = new Texture( m_heightData_width, m_heightData_height, DXGI_FORMAT_R32_FLOAT, false, eTextureType_CPUWrite );

        heightData = (float*)m_heightTexture->Lock( 0, heightTexturePitch );

        for ( int iRow = 0; iRow < m_heightData_height; iRow++ )
        {
            for ( int iCol = 0; iCol < m_heightData_width; iCol++ )
            {
                int idx = iCol + m_heightData_width * iRow;
                float f = (float)refHeightData[idx] / 255.f;
                heightData[idx] = m_heightData[idx] = f * m_heightScalator;
            }
        }

        m_heightTexture->Unlock( 0 );
    }

    _refTexture->UnlockRead( 0 );

    _BuildNormalMapTexture( );
}

//-------------------------------------------------------------------

void Terrain::_BuildHeightAndNormalsTextures( WORD *_data16, int _width, int _height )
{
    m_heightData = (float*)malloc( _width * _height * sizeof(float) );

    float *heightData = NULL;
    UINT heightTexturePitch = 0;

    m_heightTexture = new Texture( _width, _height, DXGI_FORMAT_R32_FLOAT, false, eTextureType_CPUWrite );

    heightData = (float*)m_heightTexture->Lock( 0, heightTexturePitch );

    for ( int iRow = 0; iRow < _height; iRow++ )
    {
        for ( int iCol = 0; iCol < _width; iCol++ )
        {
            int idx = iCol + _width * iRow;
            float f = _data16[idx] / 65535.f;
            heightData[idx] = m_heightData[idx] = f * m_heightScalator;
        }
    }

    m_heightTexture->Unlock( 0 );

    _BuildNormalMapTexture( );
}

//-------------------------------------------------------------------

void Terrain::_BuildNormalMapTexture( )
{
    BYTE *normalMapData = NULL;
    UINT normalMapTexturePitch = 0;

    int width = m_heightTexture->GetWidth();
    int height = m_heightTexture->GetHeight();

    m_normalsTexture = new Texture( width, height, DXGI_FORMAT_R8G8B8A8_UNORM, false, eTextureType_CPUWrite );

    normalMapData = (BYTE*)m_normalsTexture->Lock( 0, normalMapTexturePitch );

    float fDist = m_size / ( width - 1 );

    int vDelta[] = {
         0, -1,
        -1,  0,
         0,  1,
         1,  0,
    };

    for ( int iRow = 0; iRow < height; iRow++ )
    {
        for ( int iCol = 0; iCol < width; iCol++ )
        {
            int idx = iRow + width * iCol;

            float fScale = 2.f;
            D3DXVECTOR3 v(m_bottomLeft.x + iCol * fDist, m_bottomLeft.y + m_heightData[idx], m_bottomLeft.z + m_size - iRow * fDist);
            v.y *= fScale;
            D3DXVECTOR3 vTmp[4];
            for ( int i = 0; i < 4; i++ )
            {
                vTmp[i].x = v.x + vDelta[ i * 2 + 0 ] * fDist;
                vTmp[i].z = v.z - vDelta[ i * 2 + 1 ] * fDist;

                vTmp[i].y = v.y;
                int iNewRow = iRow + vDelta[ i * 2 + 0 ];
                int iNewCol = iCol + vDelta[ i * 2 + 1 ];

                if ( iNewRow >= 0 && iNewRow < height && iNewCol >= 0 && iNewCol < width )
                    vTmp[i].y = fScale * ( m_bottomLeft.y + m_heightData[ iNewRow + width * iNewCol ] );
            }

            D3DXVECTOR3 vNormalSum( 0.f, 0.f, 0.f );
            for ( int i = 0; i < 4; i++ )
            {
                D3DXVECTOR3 vAux;
                D3DXVECTOR3 vA;
                D3DXVec3Subtract( &vAux, &vTmp[i], &v );
                D3DXVec3Normalize( &vA, &vAux );
                D3DXVECTOR3 vB;
                D3DXVec3Subtract( &vAux, &vTmp[(i + 1) % 4], &v );
                D3DXVec3Normalize( &vB, &vAux );

                D3DXVec3Cross( &vAux, &vB, &vA );
                vNormalSum += vAux * 0.25f;
            }

            D3DXVECTOR3 vNormal;
            D3DXVec3Normalize( &vNormal, &vNormalSum );

            normalMapData[ idx * 4 + 0 ] = (BYTE)floor( (vNormal.x + 1.f) * 0.5f * 255.f );
            normalMapData[ idx * 4 + 1 ] = (BYTE)floor( (vNormal.y + 1.f) * 0.5f * 255.f );
            normalMapData[ idx * 4 + 2 ] = (BYTE)floor( (vNormal.z + 1.f) * 0.5f * 255.f );
            normalMapData[ idx * 4 + 3 ] = 128;
        }
    }

    m_normalsTexture->Unlock( 0 );
}

//-------------------------------------------------------------------

void Terrain::_BuildTerrainPatchesGrid( )
{
    m_terrainPatchesGrid.SetAllocParams( m_terrainPatchesGridSize * m_terrainPatchesGridSize, 16 );
    m_terrainPatchesGrid.SetSize( m_terrainPatchesGridSize * m_terrainPatchesGridSize, false );

    D3DXVECTOR3 patchSize( m_size / m_terrainPatchesGridSize, 0.f, m_size / m_terrainPatchesGridSize );

    for ( int iRow = 0; iRow < m_terrainPatchesGridSize; iRow++ )
    {
        for ( int iCol = 0; iCol < m_terrainPatchesGridSize; iCol++ )
        {
            int patchIdx = iRow * m_terrainPatchesGridSize + iCol;
            
            D3DXVECTOR3 AABB_min;
            D3DXVECTOR3 AABB_max;

            AABB_min.x = m_bottomLeft.x + iRow * patchSize.x;
            AABB_min.y = m_bottomLeft.y;
            AABB_min.z = m_bottomLeft.z + iCol * patchSize.z;

            AABB_max = AABB_min + patchSize;

            GetMinMaxHeights( AABB_min, patchSize, AABB_min.y, AABB_max.y );

            m_terrainPatchesGrid[patchIdx].UpdateAABB( AABB_min, AABB_max );
        }
    }
}

//-------------------------------------------------------------------

void Terrain::_CreateRenderData( )
{
    HRESULT hr = S_OK;

    // input layout
    const D3D11_INPUT_ELEMENT_DESC terrainLayout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = g_renderManager->GetDevice()->CreateInputLayout( terrainLayout, ARRAYSIZE(terrainLayout), g_TerrainVS.GetBufferPointer(), g_TerrainVS.GetBufferSize(), &m_terrainVertexLayout );


    // mesh data
    int n = 1 + (m_heightData_width - 1) / m_terrainPatchesGridSize;

    // vertex buffer
    int dataVBSizeInBytes = n * n * 2 * sizeof(float);
    float *dataVB = (float*)malloc( dataVBSizeInBytes );
    {
        float fSize = 1.f / (n - 1);

        for ( int iRow = 0; iRow < n; iRow++ )
        {
            for ( int iCol = 0; iCol < n; iCol++ )
            {
                int iCrtVBIdx = iRow * n + iCol;

                dataVB[iCrtVBIdx * 2 + 0] = iCol * fSize;
                dataVB[iCrtVBIdx * 2 + 1] = ( n - 1 - iRow ) * fSize;
            }
        }
    }

    // index buffer
    int indexDataSizeInBytes = (n - 1) * (n - 1) * 2 * 3 * sizeof(WORD);
    WORD *dataIB = (WORD*)malloc( indexDataSizeInBytes );
    {
        for ( int iRow = 0; iRow < n - 1; iRow++ )
        {
            for ( int iCol = 0; iCol < n - 1; iCol++ )
            {
                int iCrtVBIdx = iRow * n + iCol;
                int iCrtIBIdx = ( iRow * (n - 1) + iCol ) * 2 * 3;
                WORD v[] = { (WORD)iCrtVBIdx, (WORD)(iCrtVBIdx + 1), (WORD)(iCrtVBIdx + n), (WORD)(iCrtVBIdx + n + 1) };

                dataIB[iCrtIBIdx + 0] = v[0];
                dataIB[iCrtIBIdx + 1] = v[3];
                dataIB[iCrtIBIdx + 2] = v[2];

                dataIB[iCrtIBIdx + 3] = v[0];
                dataIB[iCrtIBIdx + 4] = v[1];
                dataIB[iCrtIBIdx + 5] = v[3];
            }
        }
    }


    m_patchMesh = new Mesh( (BYTE*)dataVB, dataVBSizeInBytes, 8, (BYTE*)dataIB, indexDataSizeInBytes, m_terrainVertexLayout );

    free( dataVB );
    free( dataIB );

    m_texture = g_texturesManager->Load( L"Textures/Terrain/NoiseDirt.dds" );
    m_baseTexture = g_texturesManager->Load( L"Textures/Terrain/NoiseDirt.dds" );
    m_noiseNormalMapTexture = g_texturesManager->Load( L"Textures/Terrain/NoiseNormal.dds" );
    m_noiseTexture = g_texturesManager->Load( L"Textures/Terrain/Noise.dds" );
}

//-------------------------------------------------------------------

void Terrain::_DestroyRenderData( )
{
    SAFE_RELEASE( m_terrainVertexLayout );
    SAFE_DELETE( m_patchMesh );
    
    SAFE_RELEASE( m_texture );
    SAFE_RELEASE( m_baseTexture );
    SAFE_RELEASE( m_noiseNormalMapTexture );
    SAFE_RELEASE( m_noiseTexture );
}

//-------------------------------------------------------------------

void Terrain::Render( )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext( );

    // Set the blend state and the depth stencil state
    float BlendFactor[4] = { 0, 0, 0, 0 };
    d3dImmediateContext->OMSetBlendState( g_renderManager->GetNoBlendState(), BlendFactor, 0xFFFFFFFF );
    d3dImmediateContext->OMSetDepthStencilState( g_renderManager->GetDepthWriteState(), 0 );
    d3dImmediateContext->RSSetState( g_renderManager->GetCullRasterState() );

    // Set the shaders
    g_TerrainVS.Set();
    g_TerrainPS.Set();

    //IA setup
    d3dImmediateContext->IASetInputLayout( m_terrainVertexLayout );

    //Render
    ID3D11SamplerState *samWrapLinear = g_renderManager->GetSamWrapLinear( );
    ID3D11SamplerState *samWrapAniso = g_renderManager->GetSamWrapAniso( );
    d3dImmediateContext->VSSetSamplers( 0, 1, &samWrapLinear );
    d3dImmediateContext->PSSetSamplers( 0, 1, &samWrapAniso );

    m_heightTexture->VSSet( 0 );
    m_normalsTexture->VSSet( 1 );

    m_colorMapTexture->PSSet( 0 );
    m_baseTexture->PSSet( 1 );
    m_texture->PSSet( 2 );
    m_noiseNormalMapTexture->PSSet( 3 );
    m_noiseTexture->PSSet( 4 );

    int patchesCount = m_terrainPatchesGrid.GetSize();
    for ( int i = 0; i < patchesCount; i++ )
    {
        const TerrainPatchData &patchData = m_terrainPatchesGrid[i];

        bool bIsFullyInFrustum = false;
        if ( !g_renderManager->GetCamera()->IsBoundingSphereInFrustum( patchData.GetBoundingSphere_center(), patchData.GetBoundingSphere_radius(), bIsFullyInFrustum ) || !g_renderManager->GetCamera()->IsAABBInFrustum( patchData.GetBoundingAABB_min(), patchData.GetBoundingAABB_max(), bIsFullyInFrustum ) )
            continue;

        // VS Per object
        st_Terrain_CB_VS_PER_OBJECT* pVSPerObject = (st_Terrain_CB_VS_PER_OBJECT*)g_renderManager->LockConstantBuffer(eConstantBufferType_PerObjectVS);
        pVSPerObject->m_patchStartPosX      = patchData.GetBoundingAABB_min().x;
        pVSPerObject->m_patchStartPosZ      = patchData.GetBoundingAABB_min().z;
        pVSPerObject->m_patchSizeX          = patchData.GetSize().x;
        pVSPerObject->m_patchSizeZ          = patchData.GetSize().z;
        pVSPerObject->m_heightMapInfo.x     = m_bottomLeft.x;
        pVSPerObject->m_heightMapInfo.y     = m_bottomLeft.z;
        pVSPerObject->m_heightMapInfo.z     = 1.f / m_size;
        pVSPerObject->m_heightMapInfo.w     = m_heightScalator;
        g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectVS);

        // PS Per object
        st_Terrain_CB_PS_PER_OBJECT* pPSPerObject = (st_Terrain_CB_PS_PER_OBJECT*)g_renderManager->LockConstantBuffer(eConstantBufferType_PerObjectPS);
        pPSPerObject->m_tile                = m_textureTile;
        pPSPerObject->m_noiseTile           = m_noiseTextureTile;
        g_renderManager->UnlockConstantBuffer(eConstantBufferType_PerObjectPS);


        m_patchMesh->Draw();
    }
}

//-------------------------------------------------------------------

void Terrain::TerrainPatchData::UpdateAABB( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max )
{
    m_boundingAABB_min = _AABB_min;
    m_boundingAABB_max = _AABB_max;

    m_boundingSphere_center =  (m_boundingAABB_min + m_boundingAABB_max ) * 0.5f;
    D3DXVECTOR3 diag = m_boundingAABB_max - m_boundingAABB_min;
    m_boundingSphere_radius = D3DXVec3Length( &diag ) * 0.5f;
}

//-------------------------------------------------------------------
