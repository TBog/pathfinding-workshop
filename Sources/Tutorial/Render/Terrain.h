#pragma once

#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "../Utils/DynVec.h"

class Mesh;
class Texture;

//===================================================================
//	CLASS Terrain
//===================================================================

class Terrain
{
protected:
    struct TerrainPatchData
    {
    public:
        TerrainPatchData()
            : m_boundingAABB_min    ( 0.f, 0.f, 0.f )
            , m_boundingAABB_max    ( 0.f, 0.f, 0.f )
            , m_boundingSphere_center( 0.f, 0.f, 0.f )
            , m_boundingSphere_radius( 0.f )
        { }
        
        void                    UpdateAABB              ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max );

        const D3DXVECTOR3&      GetBoundingSphere_center( ) const { return m_boundingSphere_center; }
        float                   GetBoundingSphere_radius( ) const { return m_boundingSphere_radius; }

        const D3DXVECTOR3&      GetBoundingAABB_min     ( ) const { return m_boundingAABB_min; }
        const D3DXVECTOR3&      GetBoundingAABB_max     ( ) const { return m_boundingAABB_max; }

        const D3DXVECTOR3       GetSize                 ( ) const { return m_boundingAABB_max - m_boundingAABB_min; }

    private:
        D3DXVECTOR3             m_boundingAABB_min;
        D3DXVECTOR3             m_boundingAABB_max;

        D3DXVECTOR3             m_boundingSphere_center;
        float                   m_boundingSphere_radius;
    };

public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Terrain                 ( const WCHAR *_szFileName );
    virtual ~Terrain        ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    bool                            Load                    ( const WCHAR *_szFileName );
    void                            Unload                  ( );

    void                            Render                  ( );

    inline float                    GetHeight               ( float x, float z );
    void                            GetMinMaxHeights        ( const D3DXVECTOR3 &_startPos, const D3DXVECTOR3 &_size, float &_outMinHeight, float &_outMaxHeight );

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _LoadHeightTexture      ( const char *_szFileName );
    void                            _LoadHeightFromRaw16    ( const char *_szFileName );
    void                            _UnloadHeightData       ( );

    void                            _BuildHeightAndNormalsTextures( Texture *_refTexture );
    void                            _BuildHeightAndNormalsTextures( WORD *_data16, int _width, int _height );
    void                            _BuildNormalMapTexture  ( );

    void                            _BuildTerrainPatchesGrid( );

    void                            _CreateRenderData       ( );
    void                            _DestroyRenderData      ( );

protected:
    WCHAR*                          m_szFileName;

    D3DXVECTOR3                     m_center;
    D3DXVECTOR3                     m_bottomLeft;
    float                           m_size;

    Texture*                        m_heightTexture;
    Texture*                        m_normalsTexture;

    float*                          m_heightData;
    int                             m_heightData_width;
    int                             m_heightData_height;
    float                           m_heightScalator;

    DynVec<TerrainPatchData>        m_terrainPatchesGrid;
    int                             m_terrainPatchesGridSize;

    ID3D11InputLayout*              m_terrainVertexLayout;

    Mesh*                           m_patchMesh;

    Texture*                        m_colorMapTexture;
    Texture*                        m_texture;
    Texture*                        m_baseTexture;
    Texture*                        m_noiseNormalMapTexture;
    Texture*                        m_noiseTexture;
    
    float                           m_textureTile;
    float                           m_noiseTextureTile;

};

float Terrain::GetHeight( float x, float z )
{
    
    float fRow = m_heightData_height * ( 1.f - (z - m_bottomLeft.z) / m_size );
    float fCol = m_heightData_width * ( (x - m_bottomLeft.x) / m_size );
    int nRow = (int)(fRow);
    int nCol = (int)(fCol);
    int nRow_1 = min( nRow + 1, m_heightData_height - 1 );
    int nCol_1 = min( nCol + 1, m_heightData_width - 1 );
    float fRowFrac = fRow - nRow;
    float fColFrac = fCol - nCol;

    float h00 = m_heightData[ nRow * m_heightData_width + nCol ];
    float h01 = m_heightData[ nRow * m_heightData_width + nCol_1 ];
    float h10 = m_heightData[ nRow_1 * m_heightData_width + nCol ];
    float h11 = m_heightData[ nRow_1 * m_heightData_width + nCol_1 ];

    float h0 = h00 + fColFrac * ( h01 - h00 );
    float h1 = h10 + fColFrac * ( h11 - h10 );

    return h0 + fRowFrac * ( h1 - h0 );
}

#endif // __TERRAIN_H__
