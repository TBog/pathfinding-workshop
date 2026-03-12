#pragma once

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "RefCounted.h"

class Object;

//===================================================================
//	CLASS Entity
//===================================================================

class Entity : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Entity                          ( const WCHAR *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );
    virtual ~Entity                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    const Object*                   GetObject               ( ) const   { return m_object; }
    const D3DXMATRIX&               GetWorldMatrix          ( ) const   { return m_worldMatrix; }

    const D3DXVECTOR3&              GetBoundingSphere_center( ) const   { return m_boundingSphere_center; }
    float                           GetBoundingSphere_radius( ) const   { return m_boundingSphere_radius; }

    const D3DXVECTOR3&              GetBoundingAABB_min     ( ) const   { return m_boundingAABB_min; }
    const D3DXVECTOR3&              GetBoundingAABB_max     ( ) const   { return m_boundingAABB_max; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    void                            _UpdateBoundingAABB     ( );

protected:
    WCHAR*                          m_szName;

    Object*                         m_object;
    D3DXMATRIX                      m_worldMatrix;

    D3DXVECTOR3                     m_boundingSphere_center;
    float                           m_boundingSphere_radius;

    D3DXVECTOR3                     m_boundingAABB_min;
    D3DXVECTOR3                     m_boundingAABB_max;

};

#endif // __ENTITY_H__
