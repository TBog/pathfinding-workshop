#pragma once

#ifndef __DEBUGRENDER_H__
#define __DEBUGRENDER_H__

#include "..\Utils\DynVec.h"

//===================================================================
//	CLASS DebugRender
//===================================================================

class DebugRender
{
private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    DebugRender             ( );
    virtual ~DebugRender    ( );

public:
    //---------------------------------------------------------------
    //	SINGLETON FUNCTIONS
    //---------------------------------------------------------------
    static DebugRender*             Get                     ( ) { return s_Instance; }

    static void                     Create                  ( );
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    HRESULT                         CreateResources         ( );
    void                            DestroyResources        ( );

    void                            AddLine                 ( const D3DXVECTOR3 &start, const D3DXVECTOR3 &end, const D3DXCOLOR &color );
    void                            AddTriangle             ( const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, const D3DXCOLOR &color );
    void                            AddQuad                 ( const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, const D3DXVECTOR3 &v3, const D3DXCOLOR &color );

    // Sphere (wire and filled), default tessellation = 16 slices/stacks
    void                            AddWireSphere           ( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddSphere               ( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation = 16 );

    // Axis-aligned cube (wire and filled), defined by center and per-axis half-extents
    void                            AddWireCube             ( const D3DXVECTOR3 &center, const D3DXVECTOR3 &halfExtents, const D3DXCOLOR &color );
    void                            AddCube                 ( const D3DXVECTOR3 &center, const D3DXVECTOR3 &halfExtents, const D3DXCOLOR &color );

    // Icosahedron (wire and filled), tessellation = subdivision level (0 = base 20-face shape)
    void                            AddWireIcosahedron      ( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation = 0 );
    void                            AddIcosahedron          ( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation = 0 );

    // Cylinder variant 1: bottom position, height (along +Y), radius
    void                            AddWireCylinder         ( const D3DXVECTOR3 &bottom, float height, float radius, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddCylinder             ( const D3DXVECTOR3 &bottom, float height, float radius, const D3DXCOLOR &color, int tessellation = 16 );

    // Cylinder variant 2: bottom position, top position, radius
    void                            AddWireCylinder         ( const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top, float radius, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddCylinder             ( const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top, float radius, const D3DXCOLOR &color, int tessellation = 16 );

    // Cylinder variant 3: 4x4 matrix (Y axis = height direction, origin = bottom center), height, radius
    void                            AddWireCylinder         ( const D3DXMATRIX &matrix, float height, float radius, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddCylinder             ( const D3DXMATRIX &matrix, float height, float radius, const D3DXCOLOR &color, int tessellation = 16 );

    void                            Flush                   ( );

protected:
    //---------------------------------------------------------------
    //	PROTECTED TYPES
    //---------------------------------------------------------------
    struct DebugVertex
    {
        D3DXVECTOR3             m_position;
        D3DXCOLOR               m_color;
    };

    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _FlushLines             ( );
    void                            _FlushTriangles         ( );

    HRESULT                         _CreateOrGrowVB         ( ID3D11Buffer **ppVB, int &capacity, int neededCapacity );

protected:
    static DebugRender*             s_Instance;

    DynVec<DebugVertex>             m_lineVertices;
    int                             m_lineVBCapacity;

    DynVec<DebugVertex>             m_triangleVertices;
    int                             m_triangleVBCapacity;

    ID3D11Buffer*                   m_lineVB;
    ID3D11Buffer*                   m_triangleVB;
    ID3D11InputLayout*              m_vertexLayout;
    ID3D11RasterizerState*          m_noCullRasterState;
};

#define g_debugRender               DebugRender::Get()

#endif // __DEBUGRENDER_H__
