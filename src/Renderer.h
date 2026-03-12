#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void BeginScene(float r, float g, float b, float a);
    void EndScene();

    void DrawCell(int gridX, int gridY, int cellSize, float r, float g, float b);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool InitializeD3D11(HWND hwnd, int width, int height);
    bool InitializeShaders();
    bool InitializeBuffers();
    bool InitializeRasterizerState();

    ID3D11Device*           m_device;
    ID3D11DeviceContext*    m_context;
    IDXGISwapChain*         m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;

    ID3D11VertexShader*     m_vertexShader;
    ID3D11PixelShader*      m_pixelShader;
    ID3D11InputLayout*      m_inputLayout;
    ID3D11Buffer*           m_vertexBuffer;
    ID3D11Buffer*           m_indexBuffer;
    ID3D11Buffer*           m_constantBuffer;
    ID3D11RasterizerState*  m_rasterizerState;

    // Precomputed row-major orthographic projection (screen pixels -> NDC).
    // Computed once during initialization and reused for every draw call.
    float m_orthoMatrix[4][4];

    int m_width;
    int m_height;
};
