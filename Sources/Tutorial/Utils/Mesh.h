#pragma once

#ifndef __MESH_H__
#define __MESH_H__

/// @file Mesh.h
/// @brief A single draw call: vertex buffer, index buffer, and a material reference.

class Material;

//===================================================================
//	CLASS Mesh
//===================================================================

/// @class Mesh
/// @brief Wraps a D3D11 vertex buffer and index buffer pair for a single draw call.
///
/// Created by @ref Object during OBJ loading.  Holds a reference to a
/// @ref Material that should be active before @ref Draw() is called.
///
/// The vertex layout is supplied at construction time as a pre-created
/// @c ID3D11InputLayout; the mesh does not own this layout (ownership stays
/// with the object/renderer that created it).
class Mesh
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs the mesh and uploads vertex/index data to the GPU.
    ///
    /// @param _vertexData              Pointer to raw vertex data.
    /// @param _vertexDataSizeInBytes   Total size of @p _vertexData in bytes.
    /// @param _vertexDataStride        Size of one vertex in bytes.
    /// @param _indexData               Pointer to raw index data (16-bit indices).
    /// @param _indexDataSizeInBytes    Total size of @p _indexData in bytes.
    /// @param _vertexLayout            Pre-created D3D11 input layout matching @p _vertexData.
    Mesh                          ( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes, ID3D11InputLayout *_vertexLayout );
    virtual ~Mesh                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Binds the vertex buffer, index buffer, and input layout, then issues the draw call.
    void                            Draw                    ( );

    /// @brief Assigns the material used when rendering this mesh.
    /// @param _material Pointer to the material (not owned by the mesh).
    void                            SetMaterial             ( Material *_material );

    /// @brief Returns the material assigned to this mesh, or @c nullptr if none.
    Material*                       GetMaterial             ( )                     { return m_material; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Creates the D3D11 vertex and index buffer resources.
    void                            _CreateBuffers          ( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes );

    /// @brief Releases the D3D11 vertex and index buffer resources.
    void                            _DestroyBuffers         ( );

    /// @brief Issues the actual @c DrawIndexed call (called from @ref Draw).
    void                            _DrawMesh               ( );

protected:
    ID3D11Buffer*                   m_VB;               ///< D3D11 vertex buffer.
    int                             m_VBStride;         ///< Size of one vertex in bytes.
    int                             m_VBVertexCount;    ///< Total number of vertices.
    ID3D11Buffer*                   m_IB;               ///< D3D11 index buffer (16-bit indices).
    int                             m_IBIndexCount;     ///< Total number of indices.
    ID3D11InputLayout*              m_vertexLayout;     ///< D3D11 input layout (not owned).

    Material*                       m_material;         ///< Associated material (not owned).
};

#endif // __MESH_H__
