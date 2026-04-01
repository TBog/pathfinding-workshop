#pragma once

#ifndef __DEBUGRENDER_H__
#define __DEBUGRENDER_H__

#include "..\Utils\DynVec.h"

#include <format>

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

    // Colour/style descriptor for AddText and AddTextFmt.
    // Use C++20 designated initializers at the call site to name only the fields you need:
    //   g_debugRender->AddText(x, y, text, {.color=COLOR_WHITE});
    //   g_debugRender->AddText(x, y, text, {.color=COLOR_WHITE, .bgColor=COLOR_BLACK});
    // NOTE: .color is mandatory – omitting it produces transparent (invisible) text.
    struct DebugTextStyle
    {
        D3DXCOLOR               color;                               // text foreground color (required)
        D3DXCOLOR               bgColor      = D3DXCOLOR(0,0,0,0);  // alpha=0: no background fill
        D3DXCOLOR               outlineColor = D3DXCOLOR(0,0,0,0);  // alpha=0: no outline/shadow
    };

    // Screen-space text at pixel coordinates (x, y).
    // Set bgColor.a > 0 for a filled background rectangle.
    // Set outlineColor.a > 0 for a 1-pixel outline drawn in four directions.
    void                            AddText                 ( int x, int y, const wchar_t *text, const DebugTextStyle &style );

    // World-space text: the 3D position is projected to screen; the text is always camera-facing
    void                            AddText                 ( const D3DXVECTOR3 &worldPos, const wchar_t *text, const DebugTextStyle &style );

    // World-space text with a full transform matrix: position is taken from the matrix translation.
    // Pass a billboard matrix (built from camera axes) for camera-facing text, or any other
    // world matrix to orient the label in the scene (e.g. lying flat on a wall).
    void                            AddText                 ( const D3DXMATRIX &worldMatrix, const wchar_t *text, const DebugTextStyle &style );

    // Formatted text – format strings use the C++20 std::format "{}" syntax.
    // Colour options are bundled in DebugTextStyle; use designated initializers.

    // Screen-space formatted text
    template<class... Args>
    void                            AddTextFmt              ( int x, int y, const DebugTextStyle &style, std::wformat_string<Args...> fmt, Args&&... args )
    {
        const std::wstring text = std::format( fmt, std::forward<Args>( args )... );
        AddText( x, y, text.c_str(), style );
    }

    // World-space 3D-position formatted text
    template<class... Args>
    void                            AddTextFmt              ( const D3DXVECTOR3 &worldPos, const DebugTextStyle &style, std::wformat_string<Args...> fmt, Args&&... args )
    {
        const std::wstring text = std::format( fmt, std::forward<Args>( args )... );
        AddText( worldPos, text.c_str(), style );
    }

    // World-space matrix formatted text
    template<class... Args>
    void                            AddTextFmt              ( const D3DXMATRIX &worldMatrix, const DebugTextStyle &style, std::wformat_string<Args...> fmt, Args&&... args )
    {
        const std::wstring text = std::format( fmt, std::forward<Args>( args )... );
        AddText( worldMatrix, text.c_str(), style );
    }

    // Sphere (wire and filled), tessellation controls slice/stack detail (default 16 => 16 slices, 8 stacks)
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

    // Circle (wire and filled), defined by center, radius and normal (tessellation = number of segments, default 16)
    void                            AddWireCircle           ( const D3DXVECTOR3 &center, float radius, const D3DXVECTOR3 &normal, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddCircle               ( const D3DXVECTOR3 &center, float radius, const D3DXVECTOR3 &normal, const D3DXCOLOR &color, int tessellation = 16 );

    // Circle (wire and filled), defined by a 4x4 matrix (Y axis = normal, origin = center) and radius
    void                            AddWireCircle           ( const D3DXMATRIX &matrix, float radius, const D3DXCOLOR &color, int tessellation = 16 );
    void                            AddCircle               ( const D3DXMATRIX &matrix, float radius, const D3DXCOLOR &color, int tessellation = 16 );

    // Wire triangle: draws the three edges of a triangle
    void                            AddWireTriangle         ( const D3DXVECTOR3 &v0, const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2, const D3DXCOLOR &color );

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

    // Maximum character count for a single text label (including null terminator)
    static const int            kTextMaxLen = 256;

    struct DebugTextEntry
    {
        wchar_t                 m_text[kTextMaxLen];
        int                     m_screenX;
        int                     m_screenY;
        D3DXCOLOR               m_color;
        D3DXCOLOR               m_bgColor;       // alpha=0: no background fill
        D3DXCOLOR               m_outlineColor;  // alpha=0: no outline/shadow
    };

    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _FlushLines             ( );
    void                            _FlushTriangles         ( );
    void                            _FlushText              ( );

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

    // GDI resources used to rasterise text glyphs into a CPU-side bitmap
    HDC                             m_textHDC;
    HBITMAP                         m_textBitmap;
    HFONT                           m_textFont;
    BYTE*                           m_textBitmapBits;   // raw pixel data owned by m_textBitmap

    // D3D11 dynamic texture + SRV for the per-frame text overlay
    ID3D11Texture2D*                m_textOverlayTex;
    ID3D11ShaderResourceView*       m_textOverlaySRV;

    // Pending text entries (accumulated between Flush calls)
    DynVec<DebugTextEntry>          m_textEntries;
};

#define g_debugRender               DebugRender::Get()

#endif // __DEBUGRENDER_H__
