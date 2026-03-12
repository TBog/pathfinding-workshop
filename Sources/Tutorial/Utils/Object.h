#pragma once

#ifndef __OBJECT_H__
#define __OBJECT_H__

/// @file Object.h
/// @brief A renderable 3-D asset: a collection of meshes and materials loaded from an OBJ file.

#include "RefCounted.h"
#include "DynVec.h"

class Material;
class Mesh;

/// @enum EObjectTextureSlotType
/// @brief Identifies the semantic role of a texture slot within a @ref Material.
enum EObjectTextureSlotType
{
    eObjectTextureSlotType_Diffuse = 0, ///< Base colour / albedo map.
    eObjectTextureSlotType_Normal,      ///< Tangent-space normal map.
    eObjectTextureSlotType_Roughness,   ///< PBR roughness map.
    eObjectTextureSlotType_Specular,    ///< Specular/reflectance map.
    eObjectTextureSlotType_AmbientOcclusion ///< Ambient occlusion map.
};

//===================================================================
//	CLASS Object
//===================================================================

/// @class Object
/// @brief A reference-counted 3-D asset loaded from an OBJ file.
///
/// An @c Object contains one or more @ref Mesh objects, each with an
/// associated @ref Material.  Local-space bounding volumes (sphere and AABB)
/// are computed during loading and used by @ref Entity to derive world-space bounds.
///
/// Objects are shared: the @ref ObjectsManager cache may return the same
/// @c Object* for multiple @ref Entity instances that use the same file and scale.
/// Use @ref AddRef / @ref Release to manage shared ownership.
class Object : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs an object and immediately loads it from @p _szFileName.
    /// @param _szFileName Wide-character path to the OBJ file.
    /// @param _scale      Uniform scale factor applied to all vertex positions at load time.
    Object                        ( const WCHAR *_szFileName, float _scale );
    virtual ~Object               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads the OBJ file and creates all GPU resources.
    ///
    /// Materials and textures referenced by the MTL file are loaded via
    /// @ref TexturesManager.
    ///
    /// @param _szFileName Wide-character path to the OBJ file.
    /// @param _scale      Uniform scale applied to all vertex positions.
    /// @return @c true on success.
    bool                            Load                    ( const WCHAR *_szFileName, float _scale );

    /// @brief Releases all GPU resources and destroys the mesh/material lists.
    void                            Unload                  ( );

    /// @brief Returns the list of meshes that make up this object.
    const DynVec<Mesh*>&            GetMeshList             ( ) const               { return  m_meshesList; }

    /// @brief Returns the local-space bounding sphere centre.
    const D3DXVECTOR3&              GetLocalBoundingSphere_center( ) const               { return m_local_boundingSphere_center; }

    /// @brief Returns the local-space bounding sphere radius.
    float                           GetLocalBoundingSphere_radius( ) const               { return m_local_boundingSphere_radius; }

    /// @brief Returns the minimum corner of the local-space AABB.
    const D3DXVECTOR3&              GetLocalBoundingAABB_min     ( ) const               { return m_local_boundingAABB_min; }

    /// @brief Returns the maximum corner of the local-space AABB.
    const D3DXVECTOR3&              GetLocalBoundingAABB_max     ( ) const               { return m_local_boundingAABB_max; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Expands the local AABB to encompass the given bounds, then recomputes the bounding sphere.
    void                            _UpdateLocalBoundingAABB    ( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max );

protected:
    WCHAR*                          m_szFileName;

    DynVec<Material*>               m_materialsList;    ///< Materials indexed by mesh.
    DynVec<Mesh*>                   m_meshesList;       ///< Sub-meshes of this object.

    D3DXVECTOR3                     m_local_boundingSphere_center;  ///< Local-space sphere centre.
    float                           m_local_boundingSphere_radius;  ///< Local-space sphere radius.

    D3DXVECTOR3                     m_local_boundingAABB_min;   ///< Local-space AABB minimum corner.
    D3DXVECTOR3                     m_local_boundingAABB_max;   ///< Local-space AABB maximum corner.

};

#endif // __OBJECT_H__
