#pragma once

#ifndef __RENDERMANAGER_H__
#define __RENDERMANAGER_H__

/// @file RenderManager.h
/// @brief Central Direct3D 11 rendering hub (singleton).

#include "..\Utils\DynVec.h"
#include "..\Utils\FrameTime.h"

class Camera;
class PostProcess;
class Sky;
class Clouds;
class Texture;
class Mesh;
class Object;
class Entity;

//===================================================================
//	ENUMS
//===================================================================

/// @enum EConstantBufferType
/// @brief Identifies a managed constant buffer slot.
///
/// RenderManager owns one D3D11 constant buffer per slot.  Callers lock a
/// slot with @ref RenderManager::LockConstantBuffer, write float data into the
/// returned pointer, then unlock it with @ref RenderManager::UnlockConstantBuffer.
///
/// Convention:
/// - **PerFrame** — updated once per frame (time, camera matrices, …).
/// - **PerPass**  — updated per render pass (light parameters, …).
/// - **PerObject** — updated per draw call (world matrix, …).
/// - **VS / PS** suffix — bound to the vertex or pixel shader stage respectively.
enum EConstantBufferType
{
    eConstantBufferType_PerFrameVS,
    eConstantBufferType_PerFramePS,
    eConstantBufferType_PerPassVS,
    eConstantBufferType_PerPassPS,
    eConstantBufferType_PerObjectVS,
    eConstantBufferType_PerObjectPS,
    eConstantBufferType_PerObjectMaterialVS,
    eConstantBufferType_PerObjectMaterialPS,

    eConstantBufferType_Count
};

//===================================================================
//	CLASS RenderManager
//===================================================================

/// @class RenderManager
/// @brief Singleton that owns the D3D11 device, swap-chain, common pipeline
///        states, and high-level draw utilities.
///
/// Responsibilities:
/// - Device and swap-chain creation/destruction.
/// - Common depth-stencil, rasterizer, blend, and sampler states.
/// - Constant-buffer pool (lock/unlock/upload pattern).
/// - High-level draw calls: @ref RenderObject, @ref RenderEntities, @ref RenderQuadMesh.
/// - Owns @ref Camera, @ref PostProcess, @ref Sky, and @ref Clouds.
///
/// Access the singleton via the @c g_renderManager macro.
class RenderManager
{
private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    RenderManager           ( );
    virtual ~RenderManager  ( );

public:
    //---------------------------------------------------------------
    //	SINGLETON FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Returns the singleton instance (may be @c nullptr before Create()).
    static RenderManager*           Get                     ( ) { return s_Instance; }

    /// @brief Creates the singleton instance.  Must be called exactly once.
    static void                     Create                  ( );

    /// @brief Destroys the singleton instance and releases all resources.
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Creates the D3D11 device, swap-chain, and all associated resources.
    /// @param hWnd Window handle to associate the swap-chain with.
    /// @return S_OK on success; an HRESULT error code on failure.
    HRESULT                         CreateDevice            ( HWND hWnd );

    /// @brief Releases the D3D11 device and all associated resources.
    void                            DestroyDevice           ( );

    /// @brief Per-frame update: advances the frame timer and updates the camera.
    /// @param dt Elapsed time in seconds since the last frame.
    void                            Update                  ( float dt );

    /// @brief Presents the current back buffer to the display.
    void                            Present                 ( );

    /// @brief Clears the back-buffer render target and depth-stencil surface.
    /// @param _ClearColor    RGBA colour to clear the colour buffer to.
    /// @param _fClearDepth   Depth value to clear the depth buffer to (typically @c 1.0f).
    void                            ClearBackBufferRenderTarget( const D3DXCOLOR &_ClearColor, float _fClearDepth );

    /// @brief Binds the back-buffer render target and depth-stencil view to the output merger.
    void                            SetBackBufferRenderTarget( );

    /// @brief Binds a single colour render target and depth surface.
    /// @param _textureRT    Colour render target texture (@c eTextureType_RenderTarget).
    /// @param _textureDepth Depth-stencil texture (@c eTextureType_DepthStencil).
    void                            SetRenderTargets        ( Texture *_textureRT, Texture *_textureDepth );

    /// @brief Binds up to four MRT colour targets and a depth surface.
    void                            SetRenderTargets        ( Texture *_textureRT0, Texture *_textureRT1, Texture *_textureRT2, Texture *_textureRT3, Texture *_textureDepth );

    /// @brief Maps a constant buffer for CPU write access.
    ///
    /// @param _eType Constant buffer to lock.
    /// @return Pointer to the mapped float data; write the CB contents here
    ///         before calling @ref UnlockConstantBuffer.
    float*                          LockConstantBuffer      ( EConstantBufferType _eType );

    /// @brief Unmaps a constant buffer and uploads the written data to the GPU.
    /// @param _eType Constant buffer to unlock (must match the previous Lock call).
    void                            UnlockConstantBuffer    ( EConstantBufferType _eType );

    /// @brief Binds the per-frame VS and PS constant buffers to their respective shader stages.
    void                            SetPerFrameConstantBuffers( );

    /// @brief Renders a full-screen quad using the given texture as input.
    /// @param _texture Texture to bind to PS slot 0 before the quad draw call.
    void                            RenderQuadMesh          ( const Texture *_texture );

    /// @brief Renders all meshes of @p _object using @p _worldMatrix.
    ///
    /// Uploads the world matrix to the per-object VS CB and iterates the mesh list.
    /// @param _object      3-D object to render.
    /// @param _worldMatrix World-space transform for this draw call.
    void                            RenderObject            ( const Object *_object, const D3DXMATRIX &_worldMatrix );

    /// @brief Renders a list of entities, performing frustum culling per entity.
    /// @param _entitiesList List of entities to render.
    void                            RenderEntities          ( const DynVec<Entity*> &_entitiesList );

    //---------------------------------------------------------------
    //	GETTER FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Returns the D3D11 device.
    ID3D11Device*                   GetDevice               ( )                 { return m_d3dDevice; }

    /// @brief Returns the D3D11 immediate device context.
    ID3D11DeviceContext*            GetDeviceContext        ( )                 { return m_d3dImmediateContext; }

    /// @brief Returns the frame-time tracker.
    FrameTime&                      GetFrameTime            ( )                 { return m_frameTime; }

    /// @brief Returns the first-person camera.
    Camera*                         GetCamera               ( )                 { return m_camera; }

    /// @brief Returns the post-process (tone-mapping) system.
    PostProcess*                    GetPostProcess          ( )                 { return m_postProcess; }

    /// @brief Returns the sky-dome renderer.
    Sky*                            GetSky                  ( )                 { return m_sky; }

    /// @brief Returns the cloud renderer.
    Clouds*                         GetClouds               ( )                 { return m_clouds; }

    /// @brief Returns the render-target width in pixels.
    int                             GetResolutionWidth      ( )                 { return m_resolutionWidth; }

    /// @brief Returns the render-target height in pixels.
    int                             GetResolutionHeight     ( )                 { return m_resolutionHeight; }

    /// @brief Returns the depth-write enabled depth-stencil state.
    ID3D11DepthStencilState*        GetDepthWriteState      ( )                 { return m_depthWriteState; }

    /// @brief Returns the depth-test-only (no write) depth-stencil state.
    ID3D11DepthStencilState*        GetNoDepthWriteState    ( )                 { return m_noDepthWriteState; }

    /// @brief Returns the depth-test disabled, depth-write disabled state.
    ID3D11DepthStencilState*        GetNoDepthTestAndNoWriteState( )            { return m_noDepthTestAndNoWriteState; }

    /// @brief Returns the back-face culling rasterizer state.
    ID3D11RasterizerState*          GetCullRasterState      ( )                 { return m_cullRasterState; }

    /// @brief Returns the alpha-blending enabled blend state.
    ID3D11BlendState*               GetBlendEnabledState    ( )                 { return m_blendEnabledState; }

    /// @brief Returns the no-blending (opaque) blend state.
    ID3D11BlendState*               GetNoBlendState         ( )                 { return m_noBlendState; }

    /// @brief Returns the wrap + bilinear linear sampler state.
    ID3D11SamplerState*             GetSamWrapLinear        ( )                 { return m_samWrapLinear; }

    /// @brief Returns the wrap + anisotropic sampler state.
    ID3D11SamplerState*             GetSamWrapAniso         ( )                 { return m_samWrapAniso; }

    /// @brief Returns the clamp + bilinear linear sampler state.
    ID3D11SamplerState*             GetSamClampLinear       ( )                 { return m_samClampLinear; }

    /// @brief Returns the point (nearest-neighbour) sampler state.
    ID3D11SamplerState*             GetSamPoint             ( )                 { return m_samPoint; }

    /// @brief Returns the default vertex input layout used for 3-D objects.
    ID3D11InputLayout*              GetDefaultObjectVertexLayout( )             { return m_defaultObjectVertexLayout; }

    /// @brief Returns the full-screen quad mesh.
    Mesh*                           GetQuadMesh             ( )                 { return m_quadMesh; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Creates the constant buffer pool.
    void                            _CreateConstantBuffers  ( );

    /// @brief Releases the constant buffer pool.
    void                            _DeleteConstantBuffers  ( );

    /// @brief Creates the full-screen quad vertex and index buffers.
    void                            _CreateQuadMeshResources( );

    /// @brief Releases the full-screen quad resources.
    void                            _DeleteQuadMeshResources( );

protected:
    static RenderManager*           s_Instance;

    HWND                            m_hWnd;

    FrameTime                       m_frameTime;
    Camera*                         m_camera;
    PostProcess*                    m_postProcess;
    Sky*                            m_sky;
    Clouds*                         m_clouds;

    ID3D11Device*                   m_d3dDevice;
    ID3D11DeviceContext*            m_d3dImmediateContext;
    IDXGISwapChain*                 m_d3dSwapChain;

    ID3D11RenderTargetView*         m_backBuffer_RenderTargetView;
    ID3D11DepthStencilView*         m_backBuffer_DepthStencilView;

    ID3D11DepthStencilState*        m_depthWriteState;
    ID3D11DepthStencilState*        m_noDepthWriteState;
    ID3D11DepthStencilState*        m_noDepthTestAndNoWriteState;

    ID3D11RasterizerState*          m_cullRasterState;

    ID3D11BlendState*               m_blendEnabledState;
    ID3D11BlendState*               m_noBlendState;

    ID3D11SamplerState*             m_samWrapLinear;
    ID3D11SamplerState*             m_samWrapAniso;
    ID3D11SamplerState*             m_samClampLinear;
    ID3D11SamplerState*             m_samPoint;

    ID3D11InputLayout*              m_defaultObjectVertexLayout;

    int                             m_resolutionWidth;
    int                             m_resolutionHeight;

    ID3D11Buffer*                   m_constantBuffers[eConstantBufferType_Count];

    Mesh*                           m_quadMesh;

};

/// @brief Global convenience accessor for @ref RenderManager.
#define g_renderManager             RenderManager::Get()

#endif // __RENDERMANAGER_H__
