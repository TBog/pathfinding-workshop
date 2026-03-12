#include "Renderer.h"
#include <d3dcompiler.h>
#include <cstring>

// Constant buffer layout matching the HLSL cbuffer
struct PerDrawCB
{
    float orthoMatrix[4][4]; // 64 bytes: row-major orthographic projection
    float posX, posY;        // 8 bytes: top-left pixel position of the quad
    float sizeX, sizeY;      // 8 bytes: pixel size of the quad
    float r, g, b, a;        // 16 bytes: RGBA color
    // Total: 96 bytes (multiple of 16, required by D3D11)
};

// Unit quad vertices: each vertex is (x, y) in [0, 1] range
static const float s_quadVertices[] =
{
    0.0f, 0.0f,  // top-left
    1.0f, 0.0f,  // top-right
    0.0f, 1.0f,  // bottom-left
    1.0f, 1.0f,  // bottom-right
};

// Two triangles covering the unit quad (clockwise winding for D3D11 left-handed default)
static const UINT s_quadIndices[] =
{
    0, 1, 2,
    1, 3, 2,
};

// Vertex shader: transforms a unit-quad vertex to NDC using an orthographic projection.
// posAndSize.xy = world-space top-left position; posAndSize.zw = world-space size.
static const char* s_vertexShaderCode =
    "cbuffer PerDraw : register(b0)\n"
    "{\n"
    "    row_major float4x4 orthoMatrix;\n"
    "    float4 posAndSize;\n"
    "    float4 color;\n"
    "};\n"
    "struct VS_OUT\n"
    "{\n"
    "    float4 pos   : SV_POSITION;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "VS_OUT main(float2 inPos : POSITION)\n"
    "{\n"
    "    VS_OUT output;\n"
    "    float2 worldPos = inPos * posAndSize.zw + posAndSize.xy;\n"
    "    output.pos   = mul(float4(worldPos, 0.0f, 1.0f), orthoMatrix);\n"
    "    output.color = color;\n"
    "    return output;\n"
    "}\n";

// Pixel shader: outputs the interpolated vertex color.
static const char* s_pixelShaderCode =
    "struct PS_IN\n"
    "{\n"
    "    float4 pos   : SV_POSITION;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "float4 main(PS_IN input) : SV_TARGET\n"
    "{\n"
    "    return input.color;\n"
    "}\n";

Renderer::Renderer()
    : m_device(NULL)
    , m_context(NULL)
    , m_swapChain(NULL)
    , m_renderTargetView(NULL)
    , m_vertexShader(NULL)
    , m_pixelShader(NULL)
    , m_inputLayout(NULL)
    , m_vertexBuffer(NULL)
    , m_indexBuffer(NULL)
    , m_constantBuffer(NULL)
    , m_rasterizerState(NULL)
    , m_width(0)
    , m_height(0)
{
    memset(m_orthoMatrix, 0, sizeof(m_orthoMatrix));
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(HWND hwnd, int width, int height)
{
    m_width  = width;
    m_height = height;

    if (!InitializeD3D11(hwnd, width, height))
        return false;

    if (!InitializeShaders())
        return false;

    if (!InitializeBuffers())
        return false;

    if (!InitializeRasterizerState())
        return false;

    return true;
}

bool Renderer::InitializeD3D11(HWND hwnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Width                   = (UINT)width;
    swapChainDesc.BufferDesc.Height                  = (UINT)height;
    swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow                       = hwnd;
    swapChainDesc.SampleDesc.Count                   = 1;
    swapChainDesc.SampleDesc.Quality                 = 0;
    swapChainDesc.Windowed                           = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
    };
    UINT numFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);

    UINT createFlags = 0;
#ifdef _DEBUG
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        createFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context);

    if (FAILED(hr))
    {
        // Retry without the debug layer (the debug layer may not be installed)
        createFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            createFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context);

        if (FAILED(hr))
            return false;
    }

    // Obtain the back buffer and create the render target view
    ID3D11Texture2D* backBuffer = NULL;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr))
        return false;

    hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
    backBuffer->Release();
    if (FAILED(hr))
        return false;

    m_context->OMSetRenderTargets(1, &m_renderTargetView, NULL);

    // Configure the viewport to cover the entire back buffer
    D3D11_VIEWPORT viewport;
    viewport.Width    = (float)width;
    viewport.Height   = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    m_context->RSSetViewports(1, &viewport);

    // Pre-compute the orthographic projection matrix once.
    // Maps screen-pixel coordinates directly to NDC:
    //   (0, 0)      -> NDC (-1,  1)  [top-left]
    //   (width, height) -> NDC ( 1, -1)  [bottom-right]
    float w = (float)width;
    float h = (float)height;
    m_orthoMatrix[0][0] =  2.0f / w; m_orthoMatrix[0][1] = 0.0f;      m_orthoMatrix[0][2] = 0.0f; m_orthoMatrix[0][3] = 0.0f;
    m_orthoMatrix[1][0] =  0.0f;     m_orthoMatrix[1][1] = -2.0f / h; m_orthoMatrix[1][2] = 0.0f; m_orthoMatrix[1][3] = 0.0f;
    m_orthoMatrix[2][0] =  0.0f;     m_orthoMatrix[2][1] = 0.0f;      m_orthoMatrix[2][2] = 1.0f; m_orthoMatrix[2][3] = 0.0f;
    m_orthoMatrix[3][0] = -1.0f;     m_orthoMatrix[3][1] = 1.0f;      m_orthoMatrix[3][2] = 0.0f; m_orthoMatrix[3][3] = 1.0f;

    return true;
}

bool Renderer::InitializeShaders()
{
    HRESULT hr;
    ID3DBlob* vsBlob    = NULL;
    ID3DBlob* psBlob    = NULL;
    ID3DBlob* errorBlob = NULL;

    hr = D3DCompile(
        s_vertexShaderCode,
        strlen(s_vertexShaderCode),
        "VertexShader",
        NULL, NULL,
        "main", "vs_4_0",
        0, 0,
        &vsBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }
    if (errorBlob) { errorBlob->Release(); errorBlob = NULL; }

    hr = D3DCompile(
        s_pixelShaderCode,
        strlen(s_pixelShaderCode),
        "PixelShader",
        NULL, NULL,
        "main", "ps_4_0",
        0, 0,
        &psBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        vsBlob->Release();
        return false;
    }
    if (errorBlob) { errorBlob->Release(); errorBlob = NULL; }

    hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &m_pixelShader);
    psBlob->Release();
    if (FAILED(hr))
    {
        vsBlob->Release();
        return false;
    }

    // Describe the single vertex element: a 2-component float position
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        layoutDesc, 1,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &m_inputLayout);
    vsBlob->Release();
    if (FAILED(hr))
        return false;

    return true;
}

bool Renderer::InitializeBuffers()
{
    HRESULT hr;

    // Immutable vertex buffer for the unit quad
    D3D11_BUFFER_DESC vbDesc;
    memset(&vbDesc, 0, sizeof(vbDesc));
    vbDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(s_quadVertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData;
    memset(&initData, 0, sizeof(initData));
    initData.pSysMem = s_quadVertices;

    hr = m_device->CreateBuffer(&vbDesc, &initData, &m_vertexBuffer);
    if (FAILED(hr))
        return false;

    // Immutable index buffer for the two triangles
    D3D11_BUFFER_DESC ibDesc;
    memset(&ibDesc, 0, sizeof(ibDesc));
    ibDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = sizeof(s_quadIndices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    initData.pSysMem = s_quadIndices;

    hr = m_device->CreateBuffer(&ibDesc, &initData, &m_indexBuffer);
    if (FAILED(hr))
        return false;

    // Dynamic constant buffer updated once per draw call
    D3D11_BUFFER_DESC cbDesc;
    memset(&cbDesc, 0, sizeof(cbDesc));
    cbDesc.Usage          = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth      = sizeof(PerDrawCB);
    cbDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&cbDesc, NULL, &m_constantBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

bool Renderer::InitializeRasterizerState()
{
    // Disable back-face culling for straightforward 2-D rendering
    D3D11_RASTERIZER_DESC rasterDesc;
    memset(&rasterDesc, 0, sizeof(rasterDesc));
    rasterDesc.FillMode        = D3D11_FILL_SOLID;
    rasterDesc.CullMode        = D3D11_CULL_NONE;
    rasterDesc.DepthClipEnable = TRUE;

    HRESULT hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
    if (FAILED(hr))
        return false;

    m_context->RSSetState(m_rasterizerState);
    return true;
}

void Renderer::Shutdown()
{
    if (m_rasterizerState) { m_rasterizerState->Release(); m_rasterizerState = NULL; }
    if (m_constantBuffer)  { m_constantBuffer->Release();  m_constantBuffer  = NULL; }
    if (m_indexBuffer)     { m_indexBuffer->Release();     m_indexBuffer     = NULL; }
    if (m_vertexBuffer)    { m_vertexBuffer->Release();    m_vertexBuffer    = NULL; }
    if (m_inputLayout)     { m_inputLayout->Release();     m_inputLayout     = NULL; }
    if (m_pixelShader)     { m_pixelShader->Release();     m_pixelShader     = NULL; }
    if (m_vertexShader)    { m_vertexShader->Release();    m_vertexShader    = NULL; }
    if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = NULL; }
    if (m_swapChain)       { m_swapChain->Release();       m_swapChain       = NULL; }
    if (m_context)         { m_context->Release();         m_context         = NULL; }
    if (m_device)          { m_device->Release();          m_device          = NULL; }
}

void Renderer::BeginScene(float r, float g, float b, float a)
{
    float clearColor[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);

    m_context->VSSetShader(m_vertexShader, NULL, 0);
    m_context->PSSetShader(m_pixelShader,  NULL, 0);
    m_context->IASetInputLayout(m_inputLayout);

    UINT stride = sizeof(float) * 2;
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    m_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
}

void Renderer::EndScene()
{
    m_swapChain->Present(1, 0);
}

void Renderer::DrawCell(int gridX, int gridY, int cellSize, float r, float g, float b)
{
    // Leave a 1-pixel gap on the right and bottom edge for grid lines
    float x = (float)(gridX * cellSize);
    float y = (float)(gridY * cellSize);
    float s = (float)(cellSize - 1);

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
        return;

    PerDrawCB* cb = (PerDrawCB*)mapped.pData;

    // Copy the precomputed orthographic projection matrix into the constant buffer
    memcpy(cb->orthoMatrix, m_orthoMatrix, sizeof(m_orthoMatrix));

    cb->posX  = x;
    cb->posY  = y;
    cb->sizeX = s;
    cb->sizeY = s;

    cb->r = r;
    cb->g = g;
    cb->b = b;
    cb->a = 1.0f;

    m_context->Unmap(m_constantBuffer, 0);

    m_context->DrawIndexed(6, 0, 0);
}
