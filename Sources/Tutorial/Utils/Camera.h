#pragma once

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Frustum.h"

//===================================================================
//	CLASS Camera
//===================================================================

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
    void                            SetViewParams       ( const D3DXVECTOR3 &eyePos, const D3DXVECTOR3 &lookAtPos );
    void                            SetProjParams       ( float fov, float aspect, float nearPlane, float farPlane );

    void                            UpdateBasedOnInput  ( float dt, float moveAxisX, float moveAxisY, float lookAxisX, float lookAxisY, float speedScale );

    const D3DXMATRIX&               GetViewMatrix       ( )         { return m_viewMatrix; }
    const D3DXMATRIX&               GetInvViewMatrix    ( )         { return m_invViewMatrix; }
    const D3DXMATRIX&               GetProjMatrix       ( )         { return m_projMatrix; }
    const D3DXVECTOR4               GetProjParams       ( )         { return D3DXVECTOR4( m_projMatrix._11, m_projMatrix._22, m_projMatrix._33, m_projMatrix._43 ); }
    const D3DXVECTOR3&              GetViewDir          ( )         { return *(D3DXVECTOR3*)&m_invViewMatrix._31; }
    const D3DXVECTOR3&              GetCameraPos        ( )         { return *(D3DXVECTOR3*)&m_invViewMatrix._41; }

    float                           GetNearPlane        ( )         { return - m_projMatrix._43 / m_projMatrix._33; }
    float                           GetFarPlane         ( )         { return m_projMatrix._43 / ( 1.f - m_projMatrix._33 ); }

    inline bool                     IsAABBInFrustum     ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &AABB_max, bool &_bOutIsFullyInFrustum );
    inline bool                     IsBoundingSphereInFrustum( const D3DXVECTOR3 &_boundingSphere_center, float _boundingSphere_radius, bool &_bOutIsFullyInFrustum );

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
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
