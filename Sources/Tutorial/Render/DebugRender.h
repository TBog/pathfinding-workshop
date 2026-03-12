#pragma once

#ifndef __DEBUGRENDER_H__
#define __DEBUGRENDER_H__

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

protected:
    static DebugRender*             s_Instance;

    static const int                MAX_LINE_VERTICES       = 4096;
    static const int                MAX_TRIANGLE_VERTICES   = 8192;

    DebugVertex                     m_lineVertices[MAX_LINE_VERTICES];
    int                             m_lineVertexCount;

    DebugVertex                     m_triangleVertices[MAX_TRIANGLE_VERTICES];
    int                             m_triangleVertexCount;

    ID3D11Buffer*                   m_lineVB;
    ID3D11Buffer*                   m_triangleVB;
    ID3D11InputLayout*              m_vertexLayout;
    ID3D11RasterizerState*          m_noCullRasterState;
};

#define g_debugRender               DebugRender::Get()

#endif // __DEBUGRENDER_H__
