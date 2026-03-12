#include "pch.h"

#include "DebugRender.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Shader.h"

#include "RenderManager.h"

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------
static ShaderVS s_DebugRenderVS(L"Shaders/DebugRender_VS.fx");
static ShaderPS s_DebugRenderPS(L"Shaders/DebugRender_PS.fx");

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
