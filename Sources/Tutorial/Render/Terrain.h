#pragma once

#ifndef __TERRAIN_H__
#define __TERRAIN_H__

/// @file Terrain.h
/// @brief Height-map terrain renderer with frustum-culled patch grid.

#include "../Utils/DynVec.h"

class Mesh;
class Texture;

//===================================================================
//	CLASS Terrain
//===================================================================

/// @class Terrain
/// @brief Renders a height-map terrain divided into a regular patch grid.
///
/// The terrain is loaded from an XML descriptor that references:
/// - A RAW-16 or image-format height map.
/// - A colour map, base surface texture, detail normal map, and noise textures.
///
/// At runtime the terrain is divided into square patches.  Each patch stores
/// a bounding sphere and AABB (see @ref TerrainPatchData) used for frustum
/// culling before the patch mesh is submitted for rendering.
///
/// Height queries are provided via @ref GetHeight (bilinear interpolation)
/// and @ref GetMinMaxHeights (range query over an AABB footprint).
class Terrain
{
protected:
    /// @struct TerrainPatchData
    /// @brief Bounding volumes for a single terrain patch, used for frustum culling.
    struct TerrainPatchData
    {
    public:
        TerrainPatchData()
            : m_boundingAABB_min    ( 0.f, 0.f, 0.f )
            , m_boundingAABB_max    ( 0.f, 0.f, 0.f )
            , m_boundingSphere_center( 0.f, 0.f, 0.f )
            , m_boundingSphere_radius( 0.f )
        { }

        /// @brief Updates the AABB and recomputes the bounding sphere from the new bounds.
        void                    UpdateAABB              ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max );

        /// @brief Returns the world-space bounding sphere centre of this patch.
        const D3DXVECTOR3&      GetBoundingSphere_center( ) const { return m_boundingSphere_center; }

        /// @brief Returns the world-space bounding sphere radius of this patch.
        float                   GetBoundingSphere_radius( ) const { return m_boundingSphere_radius; }

        /// @brief Returns the minimum corner of the world-space AABB of this patch.
        const D3DXVECTOR3&      GetBoundingAABB_min     ( ) const { return m_boundingAABB_min; }

        /// @brief Returns the maximum corner of the world-space AABB of this patch.
        const D3DXVECTOR3&      GetBoundingAABB_max     ( ) const { return m_boundingAABB_max; }

        /// @brief Returns the size of this patch as a vector (max - min).
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

    /// @brief Constructs the terrain and immediately loads it from @p _szFileName.
    /// @param _szFileName Wide-character path to the terrain XML descriptor.
    Terrain                 ( const WCHAR *_szFileName );
    virtual ~Terrain        ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads the terrain from its XML descriptor (height map, textures, shader params).
    /// @param _szFileName Wide-character path to the terrain XML descriptor.
    /// @return @c true on success.
    bool                            Load                    ( const WCHAR *_szFileName );

    /// @brief Releases all GPU and CPU resources.
    void                            Unload                  ( );

    /// @brief Renders all patches that pass the camera frustum test.
    void                            Render                  ( );

    /// @brief Queries the interpolated terrain height at world-space position (x, z).
    ///
    /// Performs bilinear interpolation over the four nearest height-map samples.
    ///
    /// @param x World-space X coordinate.
    /// @param z World-space Z coordinate.
    /// @return Interpolated world-space Y height at (x, z).
    inline float                    GetHeight               ( float x, float z );

    /// @brief Queries the minimum and maximum terrain heights within a world-space AABB footprint.
    ///
    /// @param _startPos     Minimum corner of the XZ footprint.
    /// @param _size         Size of the footprint (width, height, depth — Y is ignored).
    /// @param _outMinHeight [out] Minimum terrain height within the footprint.
    /// @param _outMaxHeight [out] Maximum terrain height within the footprint.
    void                            GetMinMaxHeights        ( const D3DXVECTOR3 &_startPos, const D3DXVECTOR3 &_size, float &_outMinHeight, float &_outMaxHeight );

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads and decodes a height map from a standard image file (PNG, DDS, etc.).
    void                            _LoadHeightTexture      ( const char *_szFileName );

    /// @brief Loads a 16-bit raw height map from a binary file.
    void                            _LoadHeightFromRaw16    ( const char *_szFileName );

    /// @brief Frees the CPU-side height-map data array.
    void                            _UnloadHeightData       ( );

    /// @brief Builds the GPU height and normals textures by sampling @p _refTexture.
    void                            _BuildHeightAndNormalsTextures( Texture *_refTexture );

    /// @brief Builds the GPU height and normals textures from a raw 16-bit data array.
    void                            _BuildHeightAndNormalsTextures( WORD *_data16, int _width, int _height );

    /// @brief Generates a normal-map texture from the current height data.
    void                            _BuildNormalMapTexture  ( );

    /// @brief Subdivides the terrain into a regular grid of patches and computes their bounds.
    void                            _BuildTerrainPatchesGrid( );

    /// @brief Creates all D3D11 render resources (vertex layouts, patch meshes).
    void                            _CreateRenderData       ( );

    /// @brief Releases all D3D11 render resources.
    void                            _DestroyRenderData      ( );

protected:
    WCHAR*                          m_szFileName;

    D3DXVECTOR3                     m_center;           ///< World-space centre of the terrain.
    D3DXVECTOR3                     m_bottomLeft;       ///< World-space bottom-left corner.
    float                           m_size;             ///< World-space side length of the terrain.

    Texture*                        m_heightTexture;    ///< GPU height texture (R32_FLOAT or similar).
    Texture*                        m_normalsTexture;   ///< GPU normal-map texture.

    float*                          m_heightData;           ///< CPU-side height samples (row-major).
    int                             m_heightData_width;     ///< Width of the height map in samples.
    int                             m_heightData_height;    ///< Height of the height map in samples.
    float                           m_heightScalator;       ///< Scale factor mapping raw height values to world units.

    DynVec<TerrainPatchData>        m_terrainPatchesGrid;       ///< Per-patch bounding volumes.
    int                             m_terrainPatchesGridSize;   ///< Number of patches along one axis.

    ID3D11InputLayout*              m_terrainVertexLayout;  ///< Vertex layout for terrain mesh vertices.

    Mesh*                           m_patchMesh;            ///< Shared mesh instance drawn once per visible patch.

    Texture*                        m_colorMapTexture;          ///< Per-texel colour map.
    Texture*                        m_texture;                  ///< Primary surface texture.
    Texture*                        m_baseTexture;              ///< Base/rock surface texture.
    Texture*                        m_noiseNormalMapTexture;    ///< High-frequency noise normal map.
    Texture*                        m_noiseTexture;             ///< Scalar noise texture.
    
    float                           m_textureTile;          ///< UV tiling factor for the primary texture.
    float                           m_noiseTextureTile;     ///< UV tiling factor for the noise texture.

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
