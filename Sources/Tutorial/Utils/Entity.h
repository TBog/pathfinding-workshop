#pragma once

#ifndef __ENTITY_H__
#define __ENTITY_H__

/// @file Entity.h
/// @brief A scene entity: a shared 3-D object placed in the world with a transform
///        and pre-computed bounding volumes.

#include "RefCounted.h"

class Object;

//===================================================================
//	CLASS Entity
//===================================================================

/// @class Entity
/// @brief A placed instance of a shared @ref Object in the scene.
///
/// Combines a reference to a shared (ref-counted) @ref Object with a
/// world-space transform matrix and pre-computed bounding volumes (both
/// sphere and AABB) in world space.  Multiple entities may share the same
/// @ref Object.
///
/// Entities are ref-counted (@ref RefCounted) so that managers can hold
/// shared ownership via @ref AddRef / @ref Release.
class Entity : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs an entity with the given name, object, and world transform.
    ///
    /// The bounding volumes are derived from the object's local bounds
    /// transformed by @p _worldMatrix.
    ///
    /// @param _szName       Human-readable name for debugging.
    /// @param _object       Shared 3-D object (must not be @c nullptr).
    /// @param _worldMatrix  Initial world-space transform.
    Entity                          ( const WCHAR *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );
    virtual ~Entity                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Returns the shared object associated with this entity.
    const Object*                   GetObject               ( ) const   { return m_object; }

    /// @brief Returns the world-space transform matrix.
    const D3DXMATRIX&               GetWorldMatrix          ( ) const   { return m_worldMatrix; }

    /// @brief Returns the world-space bounding sphere centre.
    const D3DXVECTOR3&              GetBoundingSphere_center( ) const   { return m_boundingSphere_center; }

    /// @brief Returns the world-space bounding sphere radius.
    float                           GetBoundingSphere_radius( ) const   { return m_boundingSphere_radius; }

    /// @brief Returns the minimum corner of the world-space AABB.
    const D3DXVECTOR3&              GetBoundingAABB_min     ( ) const   { return m_boundingAABB_min; }

    /// @brief Returns the maximum corner of the world-space AABB.
    const D3DXVECTOR3&              GetBoundingAABB_max     ( ) const   { return m_boundingAABB_max; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Recomputes the world-space AABB from the object's local AABB and the world matrix.
    void                            _UpdateBoundingAABB     ( );

protected:
    WCHAR*                          m_szName;

    Object*                         m_object;
    D3DXMATRIX                      m_worldMatrix;

    D3DXVECTOR3                     m_boundingSphere_center;    ///< World-space bounding sphere centre.
    float                           m_boundingSphere_radius;    ///< World-space bounding sphere radius.

    D3DXVECTOR3                     m_boundingAABB_min;         ///< World-space AABB minimum corner.
    D3DXVECTOR3                     m_boundingAABB_max;         ///< World-space AABB maximum corner.

};

#endif // __ENTITY_H__
