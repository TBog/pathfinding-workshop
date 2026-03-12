#pragma once

#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

#define FRT_START_MASK          0x3F
#define FRT_MAX_PLANES          10

// 1 byte
struct sFrustumIdx
{
    BYTE scn : 3;
    BYTE rfl : 3;
    BYTE pad : 2;
};

enum eFrustumTest {FRT_OUT, FRT_IN_OUT, FRT_IN};

struct sFrustum
{
    D3DXVECTOR4	frustum[FRT_MAX_PLANES];
    BYTE    p_idx[FRT_MAX_PLANES][3];
    BYTE    n_idx[FRT_MAX_PLANES][3];

    void            ComputeFrustum      (const D3DXMATRIX &_mxCamWorld, float sx, float sy, float _fZNear, float _fZFar, float ext);

    inline eFrustumTest    BSvsFrustumTest     (const D3DXVECTOR3 &_vCenter, float _fRadius, int in_mask);
    inline eFrustumTest    AABBvsFrustumTest   (const D3DXVECTOR3 &vAABBv1, const D3DXVECTOR3 &vAABBv2, BYTE &fr_idx, int in_mask, int &out_mask);

};

eFrustumTest sFrustum::BSvsFrustumTest(const D3DXVECTOR3 &_vCenter, float _fRadius, int in_mask)
{
    bool bIn = true;
    float d;
    for (int i = 0, k = 1; k <= in_mask; i++, k <<= 1) if (in_mask & k) 
        if ((d = D3DXVec3Dot((D3DXVECTOR3*)&frustum[i], &_vCenter) + frustum[i].w) >= 0) if (d >= _fRadius) return FRT_OUT; else bIn = false; else bIn &= (-d >= _fRadius);
    if (bIn) return FRT_IN; else return FRT_IN_OUT;
}

eFrustumTest sFrustum::AABBvsFrustumTest(const D3DXVECTOR3 &vAABBv1, const D3DXVECTOR3 &vAABBv2, BYTE &fr_idx, int in_mask, int &out_mask)
{
    D3DXVECTOR3 bb[2];
    bb[0] = vAABBv1;
    bb[1] = vAABBv2;
    float *fbb = (float*)bb; 
    BYTE i, k = 1 << fr_idx;
    eFrustumTest res = FRT_IN;

    D3DXVECTOR4 *fr = frustum + fr_idx;
    BYTE *nidx = n_idx[fr_idx], *pidx = p_idx[fr_idx];
    out_mask = 0;

    if (in_mask & k) 
    {
        if (((fr->x * fbb[nidx[0]]) + (fr->y * fbb[nidx[1]]) + (fr->z * fbb[nidx[2]]) + fr->w) >= 0) return FRT_OUT;
        if (((fr->x * fbb[pidx[0]]) + (fr->y * fbb[pidx[1]]) + (fr->z * fbb[pidx[2]]) + fr->w) > 0) {out_mask |= k; res = FRT_IN_OUT;}
    }

    for (i = 0, k = 1; k <= in_mask; i++, k <<= 1) if ((i != fr_idx) && (in_mask & k)) 
    { 
        fr = frustum + i; nidx = n_idx[i]; pidx = p_idx[i];

        if (((fr->x * fbb[nidx[0]]) + (fr->y * fbb[nidx[1]]) + (fr->z * fbb[nidx[2]]) + fr->w) >= 0) {fr_idx = i; return FRT_OUT;}
        if (((fr->x * fbb[pidx[0]]) + (fr->y * fbb[pidx[1]]) + (fr->z * fbb[pidx[2]]) + fr->w) > 0) {out_mask |= k; res = FRT_IN_OUT;}
    } 

    return res;
}

#endif // __FRUSTUM_H__
