#pragma once

#ifndef __CLOUDS_H__
#define __CLOUDS_H__

/// @file Clouds.h
/// @brief Temporal cloud renderer using ping-pong render targets.

class Texture;

//===================================================================
//	CLASS Clouds
//===================================================================

/// @class Clouds
/// @brief Renders volumetric clouds using a temporal accumulation technique.
///
/// Each frame the cloud simulation is advanced on a compute/pixel shader and
/// blended into a pair of alternating render targets (@c m_cloudsRenderTarget[2]).
/// The result is composited over the scene colour and depth buffers in @ref Render.
///
/// Call order per frame:
/// 1. @ref PreRender — advances the cloud simulation and updates @c m_currentCloudsRT.
/// 2. @ref Render    — composites the cloud render target onto the back buffer.
class Clouds
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Clouds                  ( );
    virtual ~Clouds         ( );

public:
    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Creates all GPU resources (render targets, blend state).
    void                    CreateResources ( );

    /// @brief Releases all GPU resources.
    void                    DestroyResources( );

    /// @brief Loads cloud configuration (textures, shader parameters) from an XML file.
    /// @param _szFileName Wide-character path to the clouds XML descriptor.
    /// @return @c true on success.
    bool                    Load            ( const WCHAR *_szFileName );

    /// @brief Releases loaded resources (textures, file path).
    void                    Unload          ( );

    /// @brief Advances the cloud simulation for the current frame.
    ///
    /// Must be called before @ref Render.  Uses the previous frame's view-projection
    /// matrix (stored in @c m_prevViewProjectionMatrix) for temporal reprojection.
    void                    PreRender       ( );

    /// @brief Composites the cloud layer onto the scene.
    ///
    /// @param _backColorBuffer Scene HDR colour buffer to composite clouds onto.
    /// @param _backDepthBuffer Scene depth buffer used for depth-compositing.
    void                    Render          ( Texture *_backColorBuffer, Texture *_backDepthBuffer );

protected:
    WCHAR*                  m_szFileName;

    ID3D11BlendState*       m_cloudsBlendState;         ///< Blend state used when compositing clouds.

    Texture*                m_cloudsRenderTarget[2];    ///< Ping-pong render targets for temporal accumulation.
    int                     m_currentCloudsRT;          ///< Index of the current (write) render target (0 or 1).

    Texture*                m_cloudsBaseMap;            ///< Base cloud density/shape texture.
    Texture*                m_cloudsDetailsMap;         ///< Detail noise texture for cloud edges.

    D3DXMATRIX              m_prevViewProjectionMatrix; ///< Previous frame's view-projection matrix for reprojection.

};

#endif // __CLOUDS_H__
