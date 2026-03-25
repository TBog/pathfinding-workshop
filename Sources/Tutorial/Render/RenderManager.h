#pragma once

#ifndef __RENDERMANAGER_H__
#define __RENDERMANAGER_H__

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
    static RenderManager*           Get                     ( ) { return s_Instance; }

    static void                     Create                  ( );
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    HRESULT                         CreateDevice            ( HWND hWnd );
    void                            DestroyDevice           ( );
    void                            ResizeSwapChain         ( int width, int height );

    void                            Update                  ( float dt );
    void                            Present                 ( );

    void                            ClearBackBufferRenderTarget( const D3DXCOLOR &_ClearColor, float _fClearDepth );
    void                            SetBackBufferRenderTarget( );
    void                            SetRenderTargets        ( Texture *_textureRT, Texture *_textureDepth );
    void                            SetRenderTargets        ( Texture *_textureRT0, Texture *_textureRT1, Texture *_textureRT2, Texture *_textureRT3, Texture *_textureDepth );

    float*                          LockConstantBuffer      ( EConstantBufferType _eType );
    void                            UnlockConstantBuffer    ( EConstantBufferType _eType );

    void                            SetPerFrameConstantBuffers( );

    void                            RenderQuadMesh          ( const Texture *_texture );

    void                            RenderObject            ( const Object *_object, const D3DXMATRIX &_worldMatrix );

    void                            RenderEntities          ( const DynVec<Entity*> &_entitiesList );

    //---------------------------------------------------------------
    //	GETTER FUNCTIONS
    //---------------------------------------------------------------
    ID3D11Device*                   GetDevice               ( )                 { return m_d3dDevice; }
    ID3D11DeviceContext*            GetDeviceContext        ( )                 { return m_d3dImmediateContext; }

    FrameTime&                      GetFrameTime            ( )                 { return m_frameTime; }
    Camera*                         GetCamera               ( )                 { return m_camera; }
    PostProcess*                    GetPostProcess          ( )                 { return m_postProcess; }
    Sky*                            GetSky                  ( )                 { return m_sky; }
    Clouds*                         GetClouds               ( )                 { return m_clouds; }

    int                             GetResolutionWidth      ( )                 { return m_resolutionWidth; }
    int                             GetResolutionHeight     ( )                 { return m_resolutionHeight; }

    ID3D11DepthStencilState*        GetDepthWriteState      ( )                 { return m_depthWriteState; }
    ID3D11DepthStencilState*        GetNoDepthWriteState    ( )                 { return m_noDepthWriteState; }
    ID3D11DepthStencilState*        GetNoDepthTestAndNoWriteState( )            { return m_noDepthTestAndNoWriteState; }

    ID3D11RasterizerState*          GetCullRasterState      ( )                 { return m_cullRasterState; }

    ID3D11BlendState*               GetBlendEnabledState    ( )                 { return m_blendEnabledState; }
    ID3D11BlendState*               GetNoBlendState         ( )                 { return m_noBlendState; }

    ID3D11SamplerState*             GetSamWrapLinear        ( )                 { return m_samWrapLinear; }
    ID3D11SamplerState*             GetSamWrapAniso         ( )                 { return m_samWrapAniso; }
    ID3D11SamplerState*             GetSamClampLinear       ( )                 { return m_samClampLinear; }
    ID3D11SamplerState*             GetSamPoint             ( )                 { return m_samPoint; }

    ID3D11InputLayout*              GetDefaultObjectVertexLayout( )             { return m_defaultObjectVertexLayout; }

    Mesh*                           GetQuadMesh             ( )                 { return m_quadMesh; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _CreateConstantBuffers  ( );
    void                            _DeleteConstantBuffers  ( );

    void                            _CreateQuadMeshResources( );
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

#define g_renderManager             RenderManager::Get()

#endif // __RENDERMANAGER_H__
