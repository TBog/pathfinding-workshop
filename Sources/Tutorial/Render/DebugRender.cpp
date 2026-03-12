#include "pch.h"

#include "DebugRender.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"

#include "RenderManager.h"

#include <algorithm>
#include <vector>

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------
static ShaderVS s_DebugRenderVS(L"Shaders/DebugRender_VS.fx");
static ShaderPS s_DebugRenderPS(L"Shaders/DebugRender_PS.fx");

//--------------------------------------------------------------------------------------
// Math constants
//--------------------------------------------------------------------------------------
static const float kPI          = 3.14159265358979323846f;
static const float kTwoPI       = 2.0f * kPI;
static const float kGoldenRatio = 1.61803398874989484820f; // (1 + sqrt(5)) / 2

//--------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------

// Build two unit tangent vectors perpendicular to the given unit axis.
static void s_MakeBasis( const D3DXVECTOR3 &axis, D3DXVECTOR3 &outT1, D3DXVECTOR3 &outT2 )
{
    D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );
    if ( fabsf( D3DXVec3Dot( &axis, &up ) ) > 0.9f )
        up = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
    D3DXVec3Cross( &outT1, &axis, &up );
    D3DXVec3Normalize( &outT1, &outT1 );
    D3DXVec3Cross( &outT2, &outT1, &axis );
    D3DXVec3Normalize( &outT2, &outT2 );
}

// Emit filled icosahedron triangle, recursively subdividing onto the unit sphere.
static void s_DrawIcosahedronTri( DebugRender *dr,
                                   const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color,
                                   const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const D3DXVECTOR3 &c,
                                   int depth )
{
    if ( depth == 0 )
    {
        dr->AddTriangle( center + a * radius, center + b * radius, center + c * radius, color );
        return;
    }

    D3DXVECTOR3 mAB = a + b;  D3DXVec3Normalize( &mAB, &mAB );
    D3DXVECTOR3 mBC = b + c;  D3DXVec3Normalize( &mBC, &mBC );
    D3DXVECTOR3 mCA = c + a;  D3DXVec3Normalize( &mCA, &mCA );

    s_DrawIcosahedronTri( dr, center, radius, color,   a, mAB, mCA, depth - 1 );
    s_DrawIcosahedronTri( dr, center, radius, color, mAB,   b, mBC, depth - 1 );
    s_DrawIcosahedronTri( dr, center, radius, color, mCA, mBC,   c, depth - 1 );
    s_DrawIcosahedronTri( dr, center, radius, color, mAB, mBC, mCA, depth - 1 );
}

// Canonical directed edge (smaller vertex first) for wire icosahedron deduplication.
struct s_IcoEdge { D3DXVECTOR3 a, b; };

static bool s_Vec3Less( const D3DXVECTOR3 &u, const D3DXVECTOR3 &v )
{
    if ( u.x != v.x ) return u.x < v.x;
    if ( u.y != v.y ) return u.y < v.y;
    return u.z < v.z;
}

static s_IcoEdge s_CanonicalEdge( const D3DXVECTOR3 &u, const D3DXVECTOR3 &v )
{
    return s_Vec3Less( u, v ) ? s_IcoEdge{ u, v } : s_IcoEdge{ v, u };
}

static bool s_IcoEdgeLess( const s_IcoEdge &p, const s_IcoEdge &q )
{
    if ( s_Vec3Less( p.a, q.a ) ) return true;
    if ( s_Vec3Less( q.a, p.a ) ) return false;
    return s_Vec3Less( p.b, q.b );
}

static bool s_IcoEdgeEqual( const s_IcoEdge &p, const s_IcoEdge &q )
{
    return p.a.x == q.a.x && p.a.y == q.a.y && p.a.z == q.a.z &&
           p.b.x == q.b.x && p.b.y == q.b.y && p.b.z == q.b.z;
}

// Recursively collect unique edges onto the unit sphere for wire icosahedron rendering.
static void s_CollectIcosahedronEdges( std::vector<s_IcoEdge> &edges,
                                        const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const D3DXVECTOR3 &c,
                                        int depth )
{
    if ( depth == 0 )
    {
        edges.push_back( s_CanonicalEdge( a, b ) );
        edges.push_back( s_CanonicalEdge( b, c ) );
        edges.push_back( s_CanonicalEdge( c, a ) );
        return;
    }

    D3DXVECTOR3 mAB = a + b;  D3DXVec3Normalize( &mAB, &mAB );
    D3DXVECTOR3 mBC = b + c;  D3DXVec3Normalize( &mBC, &mBC );
    D3DXVECTOR3 mCA = c + a;  D3DXVec3Normalize( &mCA, &mCA );

    s_CollectIcosahedronEdges( edges,   a, mAB, mCA, depth - 1 );
    s_CollectIcosahedronEdges( edges, mAB,   b, mBC, depth - 1 );
    s_CollectIcosahedronEdges( edges, mCA, mBC,   c, depth - 1 );
    s_CollectIcosahedronEdges( edges, mAB, mBC, mCA, depth - 1 );
}

// Emit wire circle rings + vertical lines for a cylinder given an explicit tangent basis.
static void s_AddWireCylinderGeometry( DebugRender *dr,
                                        const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top,
                                        const D3DXVECTOR3 &t1, const D3DXVECTOR3 &t2,
                                        float radius, const D3DXCOLOR &color, int tessellation )
{
    for ( int i = 0; i < tessellation; i++ )
    {
        const float a0 = kTwoPI * i / tessellation;
        const float a1 = kTwoPI * ( i + 1 ) / tessellation;
        const D3DXVECTOR3 off0 = t1 * ( radius * cosf( a0 ) ) + t2 * ( radius * sinf( a0 ) );
        const D3DXVECTOR3 off1 = t1 * ( radius * cosf( a1 ) ) + t2 * ( radius * sinf( a1 ) );

        dr->AddLine( bottom + off0, bottom + off1, color ); // bottom ring
        dr->AddLine( top    + off0, top    + off1, color ); // top ring
        dr->AddLine( bottom + off0, top    + off0, color ); // vertical strut
    }
}

// Emit filled side quads + cap triangles for a cylinder given an explicit tangent basis.
static void s_AddFilledCylinderGeometry( DebugRender *dr,
                                          const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top,
                                          const D3DXVECTOR3 &t1, const D3DXVECTOR3 &t2,
                                          float radius, const D3DXCOLOR &color, int tessellation )
{
    for ( int i = 0; i < tessellation; i++ )
    {
        const float a0 = kTwoPI * i / tessellation;
        const float a1 = kTwoPI * ( i + 1 ) / tessellation;
        const D3DXVECTOR3 off0 = t1 * ( radius * cosf( a0 ) ) + t2 * ( radius * sinf( a0 ) );
        const D3DXVECTOR3 off1 = t1 * ( radius * cosf( a1 ) ) + t2 * ( radius * sinf( a1 ) );

        // Side quad
        dr->AddQuad( bottom + off0, bottom + off1, top + off0, top + off1, color );
        // Bottom cap fan (normal faces -axis)
        dr->AddTriangle( bottom, bottom + off1, bottom + off0, color );
        // Top cap fan (normal faces +axis)
        dr->AddTriangle( top,    top    + off0, top    + off1, color );
    }
}

//-------------------------------------------------------------------

DebugRender* DebugRender::s_Instance = NULL;

DebugRender::DebugRender()
    : m_lineVertices        ( 512, 512 )
    , m_lineVBCapacity      ( 0 )
    , m_triangleVertices    ( 1024, 1024 )
    , m_triangleVBCapacity  ( 0 )
    , m_lineVB              ( NULL )
    , m_triangleVB          ( NULL )
    , m_vertexLayout        ( NULL )
    , m_noCullRasterState   ( NULL )
{
}

//-------------------------------------------------------------------

DebugRender::~DebugRender()
{
    DestroyResources();
}

//-------------------------------------------------------------------

void DebugRender::Create()
{
    if (s_Instance)
    {
        myAssert(false, L"DebugRender::Create() already called !");
        return;
    }

    s_Instance = new DebugRender();
}

//-------------------------------------------------------------------

void DebugRender::Destroy()
{
    SAFE_DELETE(s_Instance);
}

//-------------------------------------------------------------------

HRESULT DebugRender::CreateResources()
{
    HRESULT hr = S_OK;

    ID3D11Device* device = g_renderManager->GetDevice();

    // Input layout: POSITION (float3) + COLOR (float4)
    const D3D11_INPUT_ELEMENT_DESC debugLayout[] =
    {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(debugLayout, ARRAYSIZE(debugLayout), s_DebugRenderVS.GetBufferPointer(), s_DebugRenderVS.GetBufferSize(), &m_vertexLayout);
    if (FAILED(hr))
    {
        myAssert(false, L"DebugRender::CreateResources() - CreateInputLayout failed!");
        return hr;
    }

    // No-cull rasterizer state for debug geometry
    {
        D3D11_RASTERIZER_DESC rsDesc;
        rsDesc.AntialiasedLineEnable    = FALSE;
        rsDesc.CullMode                 = D3D11_CULL_NONE;
        rsDesc.DepthBias                = 0;
        rsDesc.DepthBiasClamp           = 0.0f;
        rsDesc.DepthClipEnable          = TRUE;
        rsDesc.FillMode                 = D3D11_FILL_SOLID;
        rsDesc.FrontCounterClockwise    = FALSE;
        rsDesc.MultisampleEnable        = FALSE;
        rsDesc.ScissorEnable            = FALSE;
        rsDesc.SlopeScaledDepthBias     = 0.0f;
        hr = device->CreateRasterizerState(&rsDesc, &m_noCullRasterState);
        if (FAILED(hr))
        {
            myAssert(false, L"DebugRender::CreateResources() - CreateRasterizerState failed!");
            return hr;
        }
    }

    return S_OK;
}

//-------------------------------------------------------------------

void DebugRender::DestroyResources()
{
    SAFE_RELEASE(m_noCullRasterState);
    SAFE_RELEASE(m_vertexLayout);
    SAFE_RELEASE(m_lineVB);
    m_lineVBCapacity = 0;
    SAFE_RELEASE(m_triangleVB);
    m_triangleVBCapacity = 0;
}

//-------------------------------------------------------------------

HRESULT DebugRender::_CreateOrGrowVB(ID3D11Buffer** ppVB, int& capacity, int neededCapacity)
{
    if (neededCapacity <= capacity)
        return S_OK;

    SAFE_RELEASE(*ppVB);
    capacity = 0;

    D3D11_BUFFER_DESC desc;
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags      = 0;
    desc.ByteWidth      = sizeof(DebugVertex) * neededCapacity;

    HRESULT hr = g_renderManager->GetDevice()->CreateBuffer(&desc, NULL, ppVB);
    if (FAILED(hr))
    {
        myAssert(false, L"DebugRender::_CreateOrGrowVB() - CreateBuffer failed!");
        return hr;
    }

    capacity = neededCapacity;
    return S_OK;
}

//-------------------------------------------------------------------

void DebugRender::AddLine(const D3DXVECTOR3& start, const D3DXVECTOR3& end, const D3DXCOLOR& color)
{
    DebugVertex v;

    v.m_position = start;
    v.m_color    = color;
    m_lineVertices.Add(v);

    v.m_position = end;
    m_lineVertices.Add(v);
}

//-------------------------------------------------------------------

void DebugRender::AddTriangle(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, const D3DXCOLOR& color)
{
    DebugVertex v;
    v.m_color = color;

    v.m_position = v0;
    m_triangleVertices.Add(v);

    v.m_position = v1;
    m_triangleVertices.Add(v);

    v.m_position = v2;
    m_triangleVertices.Add(v);
}

//-------------------------------------------------------------------

void DebugRender::AddQuad(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, const D3DXVECTOR3& v3, const D3DXCOLOR& color)
{
    AddTriangle(v0, v1, v2, color);
    AddTriangle(v1, v3, v2, color);
}

//-------------------------------------------------------------------

void DebugRender::AddWireSphere( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    const int stacks = tessellation / 2;
    const int slices = tessellation;

    // Horizontal rings (skip the poles)
    for ( int i = 1; i < stacks; i++ )
    {
        const float theta = kPI * i / stacks - kPI * 0.5f;
        const float y = radius * sinf( theta );
        const float r = radius * cosf( theta );
        for ( int j = 0; j < slices; j++ )
        {
            const float phi0 = kTwoPI * j / slices;
            const float phi1 = kTwoPI * ( j + 1 ) / slices;
            AddLine( D3DXVECTOR3( center.x + r * cosf( phi0 ), center.y + y, center.z + r * sinf( phi0 ) ),
                     D3DXVECTOR3( center.x + r * cosf( phi1 ), center.y + y, center.z + r * sinf( phi1 ) ), color );
        }
    }

    // Vertical meridian arcs
    for ( int i = 0; i < slices; i++ )
    {
        const float phi  = kTwoPI * i / slices;
        const float cosP = cosf( phi );
        const float sinP = sinf( phi );
        for ( int j = 0; j < stacks; j++ )
        {
            const float theta0 = kPI * j / stacks - kPI * 0.5f;
            const float theta1 = kPI * ( j + 1 ) / stacks - kPI * 0.5f;
            AddLine(
                D3DXVECTOR3( center.x + radius * cosf( theta0 ) * cosP,
                             center.y + radius * sinf( theta0 ),
                             center.z + radius * cosf( theta0 ) * sinP ),
                D3DXVECTOR3( center.x + radius * cosf( theta1 ) * cosP,
                             center.y + radius * sinf( theta1 ),
                             center.z + radius * cosf( theta1 ) * sinP ),
                color );
        }
    }
}

//-------------------------------------------------------------------

void DebugRender::AddSphere( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    const int stacks = tessellation / 2;
    const int slices = tessellation;

    for ( int i = 0; i < stacks; i++ )
    {
        const float theta0 = kPI * i / stacks - kPI * 0.5f;
        const float theta1 = kPI * ( i + 1 ) / stacks - kPI * 0.5f;
        const float y0 = radius * sinf( theta0 );
        const float r0 = radius * cosf( theta0 );
        const float y1 = radius * sinf( theta1 );
        const float r1 = radius * cosf( theta1 );

        for ( int j = 0; j < slices; j++ )
        {
            const float phi0 = kTwoPI * j / slices;
            const float phi1 = kTwoPI * ( j + 1 ) / slices;
            const D3DXVECTOR3 v00( center.x + r0 * cosf( phi0 ), center.y + y0, center.z + r0 * sinf( phi0 ) );
            const D3DXVECTOR3 v10( center.x + r0 * cosf( phi1 ), center.y + y0, center.z + r0 * sinf( phi1 ) );
            const D3DXVECTOR3 v01( center.x + r1 * cosf( phi0 ), center.y + y1, center.z + r1 * sinf( phi0 ) );
            const D3DXVECTOR3 v11( center.x + r1 * cosf( phi1 ), center.y + y1, center.z + r1 * sinf( phi1 ) );
            AddTriangle( v00, v10, v01, color );
            AddTriangle( v10, v11, v01, color );
        }
    }
}

//-------------------------------------------------------------------

void DebugRender::AddWireCube( const D3DXVECTOR3 &center, const D3DXVECTOR3 &halfExtents, const D3DXCOLOR &color )
{
    // c[0..7]: corners arranged so that c[k] encodes (±x, ±y, ±z) via bits of k
    // bit 0 = x,  bit 1 = y,  bit 2 = z
    D3DXVECTOR3 c[8];
    for ( int k = 0; k < 8; k++ )
    {
        c[k].x = center.x + ( ( k & 1 ) ? halfExtents.x : -halfExtents.x );
        c[k].y = center.y + ( ( k & 2 ) ? halfExtents.y : -halfExtents.y );
        c[k].z = center.z + ( ( k & 4 ) ? halfExtents.z : -halfExtents.z );
    }
    // 12 edges: vary one bit at a time between pairs
    static const int edges[12][2] =
    {
        {0,1},{2,3},{4,5},{6,7},  // x-edges
        {0,2},{1,3},{4,6},{5,7},  // y-edges
        {0,4},{1,5},{2,6},{3,7}   // z-edges
    };
    for ( int e = 0; e < 12; e++ )
        AddLine( c[ edges[e][0] ], c[ edges[e][1] ], color );
}

//-------------------------------------------------------------------

void DebugRender::AddCube( const D3DXVECTOR3 &center, const D3DXVECTOR3 &halfExtents, const D3DXCOLOR &color )
{
    D3DXVECTOR3 c[8];
    for ( int k = 0; k < 8; k++ )
    {
        c[k].x = center.x + ( ( k & 1 ) ? halfExtents.x : -halfExtents.x );
        c[k].y = center.y + ( ( k & 2 ) ? halfExtents.y : -halfExtents.y );
        c[k].z = center.z + ( ( k & 4 ) ? halfExtents.z : -halfExtents.z );
    }
    // 6 faces, each as a quad (2 triangles).  Indices select the 4 corners of each face.
    // front (z-): k=0,1,2,3  back (z+): k=4,5,6,7
    // left (x-): k=0,2,4,6  right (x+): k=1,3,5,7
    // bottom (y-): k=0,1,4,5  top (y+): k=2,3,6,7
    AddQuad( c[0], c[1], c[2], c[3], color ); // front
    AddQuad( c[5], c[4], c[7], c[6], color ); // back
    AddQuad( c[4], c[0], c[6], c[2], color ); // left
    AddQuad( c[1], c[5], c[3], c[7], color ); // right
    AddQuad( c[0], c[4], c[1], c[5], color ); // bottom
    AddQuad( c[2], c[3], c[6], c[7], color ); // top
}

//-------------------------------------------------------------------

void DebugRender::AddWireIcosahedron( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 0 ) tessellation = 0;

    // Unit-sphere vertices of a regular icosahedron (normalized)
    D3DXVECTOR3 v[12] =
    {
        D3DXVECTOR3( -1,  kGoldenRatio, 0 ), D3DXVECTOR3(  1,  kGoldenRatio, 0 ),
        D3DXVECTOR3( -1, -kGoldenRatio, 0 ), D3DXVECTOR3(  1, -kGoldenRatio, 0 ),
        D3DXVECTOR3(  0, -1,  kGoldenRatio ), D3DXVECTOR3(  0,  1,  kGoldenRatio ),
        D3DXVECTOR3(  0, -1, -kGoldenRatio ), D3DXVECTOR3(  0,  1, -kGoldenRatio ),
        D3DXVECTOR3(  kGoldenRatio, 0, -1 ), D3DXVECTOR3(  kGoldenRatio, 0,  1 ),
        D3DXVECTOR3( -kGoldenRatio, 0, -1 ), D3DXVECTOR3( -kGoldenRatio, 0,  1 )
    };
    for ( int i = 0; i < 12; i++ )
        D3DXVec3Normalize( &v[i], &v[i] );

    static const int faces[20][3] =
    {
        { 0,11, 5}, { 0, 5, 1}, { 0, 1, 7}, { 0, 7,10}, { 0,10,11},
        { 1, 5, 9}, { 5,11, 4}, {11,10, 2}, {10, 7, 6}, { 7, 1, 8},
        { 3, 9, 4}, { 3, 4, 2}, { 3, 2, 6}, { 3, 6, 8}, { 3, 8, 9},
        { 4, 9, 5}, { 2, 4,11}, { 6, 2,10}, { 8, 6, 7}, { 9, 8, 1}
    };

    // Collect all edges (including duplicates from shared face boundaries),
    // then sort and deduplicate so each edge is drawn exactly once.
    // Float equality in s_IcoEdgeEqual is exact here: all shared midpoints are
    // computed from the same pair of base vertices via commutative addition (a+b == b+a
    // in IEEE 754), so identical vertex pairs always produce identical bit patterns.
    std::vector<s_IcoEdge> edges;
    edges.reserve( 60 * ( 1 << ( 2 * tessellation ) ) ); // pre-dedup count: 3 * 20 * 4^N
    for ( int i = 0; i < 20; i++ )
        s_CollectIcosahedronEdges( edges,
                                    v[ faces[i][0] ], v[ faces[i][1] ], v[ faces[i][2] ],
                                    tessellation );
    std::sort( edges.begin(), edges.end(), s_IcoEdgeLess );
    edges.erase( std::unique( edges.begin(), edges.end(), s_IcoEdgeEqual ), edges.end() );

    for ( size_t i = 0; i < edges.size(); i++ )
        AddLine( center + edges[i].a * radius, center + edges[i].b * radius, color );
}

//-------------------------------------------------------------------

void DebugRender::AddIcosahedron( const D3DXVECTOR3 &center, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 0 ) tessellation = 0;

    D3DXVECTOR3 v[12] =
    {
        D3DXVECTOR3( -1,  kGoldenRatio, 0 ), D3DXVECTOR3(  1,  kGoldenRatio, 0 ),
        D3DXVECTOR3( -1, -kGoldenRatio, 0 ), D3DXVECTOR3(  1, -kGoldenRatio, 0 ),
        D3DXVECTOR3(  0, -1,  kGoldenRatio ), D3DXVECTOR3(  0,  1,  kGoldenRatio ),
        D3DXVECTOR3(  0, -1, -kGoldenRatio ), D3DXVECTOR3(  0,  1, -kGoldenRatio ),
        D3DXVECTOR3(  kGoldenRatio, 0, -1 ), D3DXVECTOR3(  kGoldenRatio, 0,  1 ),
        D3DXVECTOR3( -kGoldenRatio, 0, -1 ), D3DXVECTOR3( -kGoldenRatio, 0,  1 )
    };
    for ( int i = 0; i < 12; i++ )
        D3DXVec3Normalize( &v[i], &v[i] );

    static const int faces[20][3] =
    {
        { 0,11, 5}, { 0, 5, 1}, { 0, 1, 7}, { 0, 7,10}, { 0,10,11},
        { 1, 5, 9}, { 5,11, 4}, {11,10, 2}, {10, 7, 6}, { 7, 1, 8},
        { 3, 9, 4}, { 3, 4, 2}, { 3, 2, 6}, { 3, 6, 8}, { 3, 8, 9},
        { 4, 9, 5}, { 2, 4,11}, { 6, 2,10}, { 8, 6, 7}, { 9, 8, 1}
    };

    for ( int i = 0; i < 20; i++ )
        s_DrawIcosahedronTri( this, center, radius, color,
                               v[ faces[i][0] ], v[ faces[i][1] ], v[ faces[i][2] ],
                               tessellation );
}

//-------------------------------------------------------------------
// Cylinder – variant 1: bottom position, height (along +Y), radius
//-------------------------------------------------------------------

void DebugRender::AddWireCylinder( const D3DXVECTOR3 &bottom, float height, float radius, const D3DXCOLOR &color, int tessellation )
{
    AddWireCylinder( bottom, D3DXVECTOR3( bottom.x, bottom.y + height, bottom.z ), radius, color, tessellation );
}

void DebugRender::AddCylinder( const D3DXVECTOR3 &bottom, float height, float radius, const D3DXCOLOR &color, int tessellation )
{
    AddCylinder( bottom, D3DXVECTOR3( bottom.x, bottom.y + height, bottom.z ), radius, color, tessellation );
}

//-------------------------------------------------------------------
// Cylinder – variant 2: bottom position, top position, radius
//-------------------------------------------------------------------

void DebugRender::AddWireCylinder( const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    D3DXVECTOR3 axis = top - bottom;
    const float axisLen = D3DXVec3Length( &axis );
    if ( axisLen < FLT_EPSILON )
        return;
    axis /= axisLen;
    D3DXVECTOR3 t1, t2;
    s_MakeBasis( axis, t1, t2 );
    s_AddWireCylinderGeometry( this, bottom, top, t1, t2, radius, color, tessellation );
}

void DebugRender::AddCylinder( const D3DXVECTOR3 &bottom, const D3DXVECTOR3 &top, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    D3DXVECTOR3 axis = top - bottom;
    const float axisLen = D3DXVec3Length( &axis );
    if ( axisLen < FLT_EPSILON )
        return;
    axis /= axisLen;
    D3DXVECTOR3 t1, t2;
    s_MakeBasis( axis, t1, t2 );
    s_AddFilledCylinderGeometry( this, bottom, top, t1, t2, radius, color, tessellation );
}

//-------------------------------------------------------------------
// Cylinder – variant 3: 4x4 matrix (origin = bottom center, Y row = height axis), height, radius
//-------------------------------------------------------------------

void DebugRender::AddWireCylinder( const D3DXMATRIX &matrix, float height, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    if ( fabsf( height ) < FLT_EPSILON )
        return;
    D3DXVECTOR3 position( matrix._41, matrix._42, matrix._43 );
    D3DXVECTOR3 yAxis( matrix._21, matrix._22, matrix._23 );
    D3DXVECTOR3 xAxis( matrix._11, matrix._12, matrix._13 );
    D3DXVECTOR3 zAxis( matrix._31, matrix._32, matrix._33 );
    const float yLen = D3DXVec3Length( &yAxis );
    const float xLen = D3DXVec3Length( &xAxis );
    const float zLen = D3DXVec3Length( &zAxis );
    if ( yLen < FLT_EPSILON || xLen < FLT_EPSILON || zLen < FLT_EPSILON )
        return;
    yAxis /= yLen;
    xAxis /= xLen;
    zAxis /= zLen;
    s_AddWireCylinderGeometry( this, position, position + yAxis * height, xAxis, zAxis, radius, color, tessellation );
}

void DebugRender::AddCylinder( const D3DXMATRIX &matrix, float height, float radius, const D3DXCOLOR &color, int tessellation )
{
    if ( tessellation < 3 ) tessellation = 3;
    if ( fabsf( height ) < FLT_EPSILON )
        return;
    D3DXVECTOR3 position( matrix._41, matrix._42, matrix._43 );
    D3DXVECTOR3 yAxis( matrix._21, matrix._22, matrix._23 );
    D3DXVECTOR3 xAxis( matrix._11, matrix._12, matrix._13 );
    D3DXVECTOR3 zAxis( matrix._31, matrix._32, matrix._33 );
    const float yLen = D3DXVec3Length( &yAxis );
    const float xLen = D3DXVec3Length( &xAxis );
    const float zLen = D3DXVec3Length( &zAxis );
    if ( yLen < FLT_EPSILON || xLen < FLT_EPSILON || zLen < FLT_EPSILON )
        return;
    yAxis /= yLen;
    xAxis /= xLen;
    zAxis /= zLen;
    s_AddFilledCylinderGeometry( this, position, position + yAxis * height, xAxis, zAxis, radius, color, tessellation );
}

//-------------------------------------------------------------------

void DebugRender::Flush()
{
    if (m_lineVertices.GetSize() == 0 && m_triangleVertices.GetSize() == 0)
        return;

    ID3D11DeviceContext* d3dContext = g_renderManager->GetDeviceContext();

    // Set render states: blend enabled, depth test but no depth write, no cull
    float blendFactor[4] = { 0, 0, 0, 0 };
    d3dContext->OMSetBlendState(g_renderManager->GetBlendEnabledState(), blendFactor, 0xFFFFFFFF);
    d3dContext->OMSetDepthStencilState(g_renderManager->GetNoDepthWriteState(), 0);
    d3dContext->RSSetState(m_noCullRasterState);

    // Set shaders
    s_DebugRenderVS.Set();
    s_DebugRenderPS.Set();

    // Set input layout
    d3dContext->IASetInputLayout(m_vertexLayout);

    _FlushLines();
    _FlushTriangles();
}

//-------------------------------------------------------------------

void DebugRender::_FlushLines()
{
    const int count = m_lineVertices.GetSize();
    if (count == 0)
        return;

    // Grow the GPU buffer if the DynVec has expanded beyond it
    if (FAILED(_CreateOrGrowVB(&m_lineVB, m_lineVBCapacity, m_lineVertices.GetMaxSize())))
    {
        m_lineVertices.Clear();
        return;
    }

    ID3D11DeviceContext* d3dContext = g_renderManager->GetDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = d3dContext->Map(m_lineVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        myAssert(false, L"DebugRender::_FlushLines() - Map failed!");
        m_lineVertices.Clear();
        return;
    }
    memcpy(mapped.pData, m_lineVertices.GetData(), sizeof(DebugVertex) * count);
    d3dContext->Unmap(m_lineVB, 0);

    UINT stride = sizeof(DebugVertex);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &m_lineVB, &stride, &offset);
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    d3dContext->Draw((UINT)count, 0);

    m_lineVertices.Clear();
}

//-------------------------------------------------------------------

void DebugRender::_FlushTriangles()
{
    const int count = m_triangleVertices.GetSize();
    if (count == 0)
        return;

    // Grow the GPU buffer if the DynVec has expanded beyond it
    if (FAILED(_CreateOrGrowVB(&m_triangleVB, m_triangleVBCapacity, m_triangleVertices.GetMaxSize())))
    {
        m_triangleVertices.Clear();
        return;
    }

    ID3D11DeviceContext* d3dContext = g_renderManager->GetDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = d3dContext->Map(m_triangleVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        myAssert(false, L"DebugRender::_FlushTriangles() - Map failed!");
        m_triangleVertices.Clear();
        return;
    }
    memcpy(mapped.pData, m_triangleVertices.GetData(), sizeof(DebugVertex) * count);
    d3dContext->Unmap(m_triangleVB, 0);

    UINT stride = sizeof(DebugVertex);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &m_triangleVB, &stride, &offset);
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    d3dContext->Draw((UINT)count, 0);

    m_triangleVertices.Clear();
}

//-------------------------------------------------------------------
