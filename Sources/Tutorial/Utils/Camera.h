#pragma once

#ifndef __CAMERA_H__
#define __CAMERA_H__

/// @file Camera.h
/// @brief First-person perspective camera with frustum-culling support.

#include "Frustum.h"

//===================================================================
//	CLASS Camera
//===================================================================

/// @class Camera
/// @brief First-person perspective camera.
///
/// Maintains a view matrix and a projection matrix, derives an inverse
/// view matrix for easy world-space position / direction queries, and
/// wraps a @ref sFrustum for per-frame visibility culling.
class Camera
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Camera( );
    virtual ~Camera( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Sets the view matrix from eye position and look-at target.
    /// @param eyePos    World-space position of the camera.
    /// @param lookAtPos World-space point the camera looks toward.
    void                            SetViewParams       ( const D3DXVECTOR3 &eyePos, const D3DXVECTOR3 &lookAtPos );

    /// @brief Sets the projection matrix from standard perspective parameters.
    /// @param fov        Vertical field-of-view in radians.
    /// @param aspect     Viewport aspect ratio (width / height).
    /// @param nearPlane  Distance to the near clip plane.
    /// @param farPlane   Distance to the far clip plane.
    void                            SetProjParams       ( float fov, float aspect, float nearPlane, float farPlane );

    /// @brief Translates and rotates the camera based on analogue axis input.
    ///
    /// Intended to be driven by gamepad or keyboard/mouse axes each frame.
    ///
    /// @param dt         Delta-time in seconds.
    /// @param moveAxisX  Lateral movement axis value [-1, 1] (strafe).
    /// @param moveAxisY  Forward/backward movement axis value [-1, 1].
    /// @param lookAxisX  Yaw (horizontal look) axis value [-1, 1].
    /// @param lookAxisY  Pitch (vertical look) axis value [-1, 1].
    /// @param speedScale Multiplier applied to the base movement speed.
    void                            UpdateBasedOnInput  ( float dt, float moveAxisX, float moveAxisY, float lookAxisX, float lookAxisY, float speedScale );

    /// @brief Returns the current view matrix (world → camera space).
    const D3DXMATRIX&               GetViewMatrix       ( )         { return m_viewMatrix; }

    /// @brief Returns the inverse view matrix (camera → world space).
    const D3DXMATRIX&               GetInvViewMatrix    ( )         { return m_invViewMatrix; }

    /// @brief Returns the current projection matrix.
    const D3DXMATRIX&               GetProjMatrix       ( )         { return m_projMatrix; }

    /// @brief Returns the four projection parameters packed into a @c D3DXVECTOR4.
    ///
    /// The components correspond to @c m_projMatrix._11, @c ._22, @c ._33,
    /// and @c ._43, which encode the X/Y scale and Z mapping coefficients.
    const D3DXVECTOR4               GetProjParams       ( )         { return D3DXVECTOR4( m_projMatrix._11, m_projMatrix._22, m_projMatrix._33, m_projMatrix._43 ); }

    /// @brief Returns the world-space camera forward direction (from the inverse view matrix row 3).
    const D3DXVECTOR3&              GetViewDir          ( )         { return *(D3DXVECTOR3*)&m_invViewMatrix._31; }

    /// @brief Returns the world-space camera position (from the inverse view matrix row 4).
    const D3DXVECTOR3&              GetCameraPos        ( )         { return *(D3DXVECTOR3*)&m_invViewMatrix._41; }

    /// @brief Derives the near-plane distance from the projection matrix.
    float                           GetNearPlane        ( )         { return - m_projMatrix._43 / m_projMatrix._33; }

    /// @brief Derives the far-plane distance from the projection matrix.
    float                           GetFarPlane         ( )         { return m_projMatrix._43 / ( 1.f - m_projMatrix._33 ); }

    /// @brief Tests whether an AABB intersects the view frustum.
    ///
    /// @param _AABB_min               Minimum corner of the AABB in world space.
    /// @param AABB_max                Maximum corner of the AABB in world space.
    /// @param _bOutIsFullyInFrustum   Set to @c true if the AABB is completely inside.
    /// @return @c true if the AABB is at least partially inside the frustum.
    inline bool                     IsAABBInFrustum     ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &AABB_max, bool &_bOutIsFullyInFrustum );

    /// @brief Tests whether a bounding sphere intersects the view frustum.
    ///
    /// @param _boundingSphere_center   World-space centre of the sphere.
    /// @param _boundingSphere_radius   Radius of the sphere.
    /// @param _bOutIsFullyInFrustum    Set to @c true if the sphere is completely inside.
    /// @return @c true if the sphere is at least partially inside the frustum.
    inline bool                     IsBoundingSphereInFrustum( const D3DXVECTOR3 &_boundingSphere_center, float _boundingSphere_radius, bool &_bOutIsFullyInFrustum );

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Rebuilds the view frustum from the current view and projection matrices.
    void                            _UpdateFrustum      ( );

private:
    D3DXMATRIX                      m_viewMatrix;           // View matrix 
    D3DXMATRIX                      m_projMatrix;           // Projection matrix

    D3DXMATRIX                      m_invViewMatrix;        // Inverse view matrix 
    float                           m_moveSpeed;
    float                           m_rotationYawSpeed;
    float                           m_rotationPitchSpeed;

    sFrustum                        m_frustum;

};

//-------------------------------------------------------------------

inline bool Camera::IsAABBInFrustum( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &AABB_max, bool &_bOutIsFullyInFrustum )
{
    BYTE fr_idx = 0;
    int in_mask = FRT_START_MASK, out_mask = 0;
    eFrustumTest isInFrustum = m_frustum.AABBvsFrustumTest( _AABB_min, AABB_max, fr_idx, in_mask, out_mask );

    _bOutIsFullyInFrustum = ( isInFrustum == FRT_IN );

    return ( isInFrustum != FRT_OUT );
}

//-------------------------------------------------------------------

inline bool Camera::IsBoundingSphereInFrustum( const D3DXVECTOR3 &_boundingSphere_center, float _boundingSphere_radius, bool &_bOutIsFullyInFrustum )
{
    int in_mask = FRT_START_MASK;
    eFrustumTest isInFrustum = m_frustum.BSvsFrustumTest( _boundingSphere_center, _boundingSphere_radius, in_mask );

    _bOutIsFullyInFrustum = ( isInFrustum == FRT_IN );

    return ( isInFrustum != FRT_OUT );
}

#endif // __CAMERA_H__
