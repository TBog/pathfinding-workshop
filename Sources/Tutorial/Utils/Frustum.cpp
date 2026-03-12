#include "pch.h"

#include "Frustum.h"

static void ComputeFaceNormal(D3DXVECTOR4 &n, const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const D3DXVECTOR3 &c, bool normalize)
{
    D3DXVECTOR3 v1, v2;
    D3DXVec3Subtract( &v1, &a, &b );
    D3DXVec3Subtract( &v2, &a, &c );
    D3DXVec3Cross( (D3DXVECTOR3*)&n, &v2, &v1 );
    if (normalize) D3DXVec3Normalize( (D3DXVECTOR3*)&n, (D3DXVECTOR3*)&n );
    n.w = -D3DXVec3Dot( (D3DXVECTOR3*)&n, &a );
}

void sFrustum::ComputeFrustum(const D3DXMATRIX &_mxCamWorld, float sx, float sy, float _fZNear, float _fZFar, float ext)
{
    float kh = 1.f / sx;
    float kv = 1.f / sy;
    D3DXVECTOR4 camFrustum[6];
    D3DXVECTOR3 v[] = {D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(kh, kv, 1), D3DXVECTOR3(-kh, kv, 1), D3DXVECTOR3(-kh, -kv, 1), D3DXVECTOR3(kh, -kv, 1)};
    D3DXVECTOR4 q[2] = {D3DXVECTOR4(0, 0, -1, _fZNear), D3DXVECTOR4(0, 0, 1, -_fZFar)};
    camFrustum[4] = q[0];
    camFrustum[5] = q[1]; 
    for(int iFr=0; iFr<4; iFr++) ComputeFaceNormal(camFrustum[iFr], v[0], v[iFr+1], v[(iFr+1 & 3) + 1], true);


    BYTE i, j, k, l;
    frustum[1] = *(D3DXVECTOR4*)&_mxCamWorld._31;
    D3DXVec4Scale( &frustum[0], &frustum[1], -1.f );
    
    D3DXVECTOR3 &vCamPos = *(D3DXVECTOR3*)&_mxCamWorld._41;
    
    frustum[0].w = -D3DXVec3Dot( (D3DXVECTOR3*)&frustum[0], &vCamPos ) + _fZNear - ext;
    frustum[1].w = -D3DXVec3Dot( (D3DXVECTOR3*)&frustum[1], &vCamPos ) - _fZFar - ext;
    
    for (j = 0; j < 3; j++)	
    {
        k = j + 3;
        if (((float*)frustum[0])[j] >= 0.0f) {p_idx[0][j] = k; n_idx[0][j] = j;} else {p_idx[0][j] = j; n_idx[0][j] = k;}
        if (((float*)frustum[1])[j] >= 0.0f) {p_idx[1][j] = k; n_idx[1][j] = j;} else {p_idx[1][j] = j; n_idx[1][j] = k;}
    }
    
    for (i = 0; i < 4; i++) 
    {
        l = i + 2;
        D3DXVec3TransformNormal( (D3DXVECTOR3*)&frustum[l], (D3DXVECTOR3*)&camFrustum[i], &_mxCamWorld);
        frustum[l].w = -D3DXVec3Dot((D3DXVECTOR3*)&frustum[l], &vCamPos) - ext;
        for (j = 0; j < 3; j++) 
        {
            k = j + 3;
            if (((float*)frustum[l])[j] >= 0.0f) {p_idx[l][j] = k; n_idx[l][j] = j;} else {p_idx[l][j] = j; n_idx[l][j] = k;}
        }
    }
}
