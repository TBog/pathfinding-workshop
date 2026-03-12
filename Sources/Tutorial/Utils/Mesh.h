#pragma once

#ifndef __MESH_H__
#define __MESH_H__

class Material;

//===================================================================
//	CLASS Mesh
//===================================================================

class Mesh
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Mesh                          ( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes, ID3D11InputLayout *_vertexLayout );
    virtual ~Mesh                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            Draw                    ( );

    void                            SetMaterial             ( Material *_material );
    Material*                       GetMaterial             ( )                     { return m_material; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _CreateBuffers          ( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes );
    void                            _DestroyBuffers         ( );

    void                            _DrawMesh               ( );

protected:
    ID3D11Buffer*                   m_VB;
    int                             m_VBStride;
    int                             m_VBVertexCount;
    ID3D11Buffer*                   m_IB;
    int                             m_IBIndexCount;
    ID3D11InputLayout*              m_vertexLayout;

    Material*                       m_material;
};

#endif // __MESH_H__
