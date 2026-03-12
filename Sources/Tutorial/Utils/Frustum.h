#pragma once

#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

/// @file Frustum.h
/// @brief View-frustum plane representation and visibility-test helpers.

/// @brief Initial bitmask enabling all six frustum planes (bits 0–5).
#define FRT_START_MASK          0x3F

/// @brief Maximum number of frustum planes supported.
#define FRT_MAX_PLANES          10

/// @struct sFrustumIdx
/// @brief Compact frustum-plane index pair used during hierarchical culling.
///
/// Stores the index of the most-recently-tested "scene" plane (@c scn) and
/// the "reflection" plane (@c rfl) in 3 bits each, fitting into one byte.
// 1 byte
struct sFrustumIdx
{
    BYTE scn : 3;   ///< Index of the last tested scene-side plane.
    BYTE rfl : 3;   ///< Index of the last tested reflection-side plane.
    BYTE pad : 2;   ///< Padding to fill the byte.
};

/// @enum eFrustumTest
/// @brief Result of a frustum visibility test.
enum eFrustumTest {
    FRT_OUT,    ///< Object is completely outside the frustum (invisible).
    FRT_IN_OUT, ///< Object straddles one or more frustum planes (partially visible).
    FRT_IN      ///< Object is completely inside the frustum (fully visible).
};

/// @struct sFrustum
/// @brief View frustum defined by up to @c FRT_MAX_PLANES half-spaces.
///
/// Each plane is stored as a @c D3DXVECTOR4 (normal.xyz, offset.w) in
/// world space such that a point @p P satisfies
/// @code
///   dot(plane.xyz, P) + plane.w < 0  →  inside the half-space
/// @endcode
///
/// The @c p_idx and @c n_idx tables pre-compute which AABB corner to test
/// against each plane's positive and negative normal directions, enabling the
/// fast AABB-vs-frustum test.
struct sFrustum
{
    D3DXVECTOR4	frustum[FRT_MAX_PLANES]; ///< Plane equations (normal.xyz, offset.w).
    BYTE    p_idx[FRT_MAX_PLANES][3];    ///< Positive-normal corner indices per plane.
    BYTE    n_idx[FRT_MAX_PLANES][3];    ///< Negative-normal corner indices per plane.

    /// @brief Recomputes all frustum planes from a camera world matrix and projection params.
    ///
    /// @param _mxCamWorld Camera-to-world matrix (inverse view).
    /// @param sx          Half-width of the view frustum at unit depth (tan(fovX/2)).
    /// @param sy          Half-height of the view frustum at unit depth (tan(fovY/2)).
    /// @param _fZNear     Near-plane distance.
    /// @param _fZFar      Far-plane distance.
    /// @param ext         Optional outward extent used to inflate the frustum.
    void            ComputeFrustum      (const D3DXMATRIX &_mxCamWorld, float sx, float sy, float _fZNear, float _fZFar, float ext);

    /// @brief Tests a bounding sphere against the frustum planes specified by @p in_mask.
    ///
    /// @param _vCenter  World-space centre of the sphere.
    /// @param _fRadius  Radius of the sphere.
    /// @param in_mask   Bitmask of planes to test (bit @c k → plane @c k).
    ///                  Pass @c FRT_START_MASK to test all six standard planes.
    /// @return @c FRT_OUT, @c FRT_IN_OUT, or @c FRT_IN.
    inline eFrustumTest    BSvsFrustumTest     (const D3DXVECTOR3 &_vCenter, float _fRadius, int in_mask);

    /// @brief Tests an axis-aligned bounding box against the frustum planes.
    ///
    /// Uses the p_idx/n_idx optimisation to avoid testing all eight AABB corners.
    /// The @p fr_idx hint indicates which plane to test first (typically the plane
    /// that rejected the previous object, for early-out coherence).
    ///
    /// @param vAABBv1   Minimum corner of the AABB in world space.
    /// @param vAABBv2   Maximum corner of the AABB in world space.
    /// @param fr_idx    [in/out] Index of the plane to prioritise; updated to the
    ///                  rejecting plane on @c FRT_OUT.
    /// @param in_mask   Bitmask of planes to test.
    /// @param out_mask  [out] Bitmask of planes the AABB straddles.
    /// @return @c FRT_OUT, @c FRT_IN_OUT, or @c FRT_IN.
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
