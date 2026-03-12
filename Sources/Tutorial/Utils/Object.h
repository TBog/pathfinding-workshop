#pragma once

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "RefCounted.h"
#include "DynVec.h"

class Material;
class Mesh;

enum EObjectTextureSlotType
{
    eObjectTextureSlotType_Diffuse = 0,
    eObjectTextureSlotType_Normal,
    eObjectTextureSlotType_Roughness,
    eObjectTextureSlotType_Specular,
    eObjectTextureSlotType_AmbientOcclusion
};

//===================================================================
//	CLASS Object
//===================================================================

class Object : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Object                        ( const WCHAR *_szFileName, float _scale );
    virtual ~Object               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    bool                            Load                    ( const WCHAR *_szFileName, float _scale );
    void                            Unload                  ( );

    const DynVec<Mesh*>&            GetMeshList             ( ) const               { return  m_meshesList; }

    const D3DXVECTOR3&              GetLocalBoundingSphere_center( ) const               { return m_local_boundingSphere_center; }
    float                           GetLocalBoundingSphere_radius( ) const               { return m_local_boundingSphere_radius; }

    const D3DXVECTOR3&              GetLocalBoundingAABB_min     ( ) const               { return m_local_boundingAABB_min; }
    const D3DXVECTOR3&              GetLocalBoundingAABB_max     ( ) const               { return m_local_boundingAABB_max; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _UpdateLocalBoundingAABB    ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max );

protected:
    WCHAR*                          m_szFileName;

    DynVec<Material*>               m_materialsList;
    DynVec<Mesh*>                   m_meshesList;

    D3DXVECTOR3                     m_local_boundingSphere_center;
    float                           m_local_boundingSphere_radius;

    D3DXVECTOR3                     m_local_boundingAABB_min;
    D3DXVECTOR3                     m_local_boundingAABB_max;

};

#endif // __OBJECT_H__
