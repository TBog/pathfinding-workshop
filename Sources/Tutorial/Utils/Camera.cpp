#include "pch.h"

#include "Camera.h"

//-------------------------------------------------------------------

static const float DEFAULT_CAMERA_MOVE_SPEED = 2.f;

static const float DEFAULT_CAMERA_ROTATION_YAW_SPEED    = (float)(0.5f * D3DX_PI);
static const float DEFAULT_CAMERA_ROTATION_PITCHSPEED   = (float)(0.5f * D3DX_PI);

//-------------------------------------------------------------------

Camera::Camera( )
    : m_moveSpeed               ( DEFAULT_CAMERA_MOVE_SPEED  )
    , m_rotationYawSpeed        ( DEFAULT_CAMERA_ROTATION_YAW_SPEED  )
    , m_rotationPitchSpeed      ( DEFAULT_CAMERA_ROTATION_PITCHSPEED  )
{
    // Setup the camera's view parameters
    D3DXVECTOR3 eyePos( 0.f, 0.f, -4.f );
    D3DXVECTOR3 lookAtPos( 0.f, 0.f, 0.f );
    SetViewParams( eyePos, lookAtPos );

    // Setup the camera's projection parameters
    float fAspectRatio = 1280.f / 720.f;
    SetProjParams( (float)D3DX_PI / 4, fAspectRatio, 0.1f, 1500.0f );

    _UpdateFrustum( );
}

//-------------------------------------------------------------------

Camera::~Camera( )
{

}

//-------------------------------------------------------------------

void Camera::SetViewParams( const D3DXVECTOR3 &eyePos, const D3DXVECTOR3 &lookAtPos )
{
    // Calc the view matrix
    D3DXVECTOR3 up( 0, 1, 0 );
    D3DXMatrixLookAtLH( &m_viewMatrix, &eyePos, &lookAtPos, &up );

    // Update the view matrix
    D3DXMatrixInverse( &m_invViewMatrix, NULL, &m_viewMatrix );

    _UpdateFrustum( );
}

//-------------------------------------------------------------------

void Camera::SetProjParams( float fov, float aspect, float nearPlane, float farPlane )
{
    // Set attributes for the projection matrix
    D3DXMatrixPerspectiveFovLH( &m_projMatrix, fov, aspect, nearPlane, farPlane );

    _UpdateFrustum( );
}

//-------------------------------------------------------------------

void Camera::UpdateBasedOnInput( float dt, float moveAxisX, float moveAxisY, float lookAxisX, float lookAxisY, float speedScale )
{
    // The axis basis vectors and camera position are stored inside the 
    // position matrix in the 4 rows of the camera's world matrix.
    // To figure out the yaw/pitch of the camera, we just need the Z basis vector
    D3DXVECTOR3* ZBasis = (D3DXVECTOR3*)&m_invViewMatrix._31;

    float cameraYawAngle = atan2f( ZBasis->x, ZBasis->z );
    float length = sqrtf( ZBasis->z * ZBasis->z + ZBasis->x * ZBasis->x );
    float cameraPitchAngle = -atan2f( ZBasis->y, length );

    // If rotating the camera 
    if ( (lookAxisX != 0) || (lookAxisY != 0) )
    {
        // Update the pitch & yaw angle based on mouse movement
        float yawDelta = lookAxisX * m_rotationYawSpeed * dt;
        float pitchDelta = lookAxisY * m_rotationPitchSpeed * dt;

        cameraPitchAngle -= pitchDelta;
        cameraYawAngle += yawDelta;

        // Limit pitch to straight up or straight down
        cameraPitchAngle = __max( (float)-D3DX_PI / 2.0f + (float)D3DX_PI / 180.f, cameraPitchAngle );
        cameraPitchAngle = __min( (float)+D3DX_PI / 2.0f - (float)D3DX_PI / 180.f, cameraPitchAngle );
    }

    // Calculate position delta
    D3DXVECTOR3 velocity = speedScale * D3DXVECTOR3( moveAxisX, 0.f, moveAxisY );
    D3DXVECTOR3 posDelta = velocity * dt;

    // Make a rotation matrix based on the camera's yaw & pitch
    D3DXMATRIX cameraRotationMatrix;
    D3DXMatrixRotationYawPitchRoll( &cameraRotationMatrix, cameraYawAngle, cameraPitchAngle, 0 );

    // Transform vectors based on camera's rotation matrix
    D3DXVECTOR3 worldUp, worldAhead;
    D3DXVECTOR3 localUp = D3DXVECTOR3( 0, 1, 0 );
    D3DXVECTOR3 localAhead = D3DXVECTOR3( 0, 0, 1 );
    D3DXVec3TransformCoord( &worldUp, &localUp, &cameraRotationMatrix );
    D3DXVec3TransformCoord( &worldAhead, &localAhead, &cameraRotationMatrix );

    // Transform the position delta by the camera's rotation 
    D3DXVECTOR3 posDeltaWorld;
    D3DXVec3TransformCoord( &posDeltaWorld, &posDelta, &cameraRotationMatrix );

    // Move the eye position 
    D3DXVECTOR3 eyePos = D3DXVECTOR3( m_invViewMatrix._41, m_invViewMatrix._42, m_invViewMatrix._43 ) + posDeltaWorld;

    // Update the lookAt position based on the eye position 
    D3DXVECTOR3 lookAtPos = eyePos + worldAhead;

    // Update the view matrix
    D3DXMatrixLookAtLH( &m_viewMatrix, &eyePos, &lookAtPos, &worldUp );

    // Update the inverse view matrix
    D3DXMatrixInverse( &m_invViewMatrix, NULL, &m_viewMatrix );

    _UpdateFrustum( );
}

//-------------------------------------------------------------------

void Camera::_UpdateFrustum( )
{
    float nearPlane = GetNearPlane( );
    float farPlane = GetFarPlane( );

    m_frustum.ComputeFrustum( m_invViewMatrix, m_projMatrix._11, m_projMatrix._22, nearPlane, farPlane, 0.f );
}

//-------------------------------------------------------------------
