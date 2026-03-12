#pragma once

#ifndef __WORLD_H__
#define __WORLD_H__

/// @file World.h
/// @brief Scene/world loader that populates entities and terrain from an XML descriptor.

#include "..\Utils\DynVec.h"

class Entity;
class Sky;
class Terrain;

//===================================================================
//	CLASS World
//===================================================================

/// @class World
/// @brief Loads and owns the scene described by an XML world descriptor.
///
/// The XML file defines:
/// - The terrain (file path, scale, position).
/// - A list of placed 3-D objects (file path, scale, world transform).
///
/// On load, @ref World creates @ref Entity instances via @ref EntitiesManager
/// and a @ref Terrain instance.  The entity list can be retrieved with
/// @ref GetEntitiesList and passed to @ref RenderManager::RenderEntities.
class World
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs the world and immediately loads it from @p _szFileName.
    /// @param _szFileName Wide-character path to the world XML descriptor.
    World                           ( const WCHAR *_szFileName );
    virtual ~World                  ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads the world from its XML descriptor.
    ///
    /// Creates entities via @ref EntitiesManager and a @ref Terrain instance.
    ///
    /// @param _szFileName Wide-character path to the world XML descriptor.
    /// @return @c true on success.
    bool                            Load                    ( const WCHAR *_szFileName );

    /// @brief Releases all entities and the terrain.
    void                            Unload                  ( );

    /// @brief Returns the list of world entities (for rendering and queries).
    const DynVec<Entity*>&          GetEntitiesList         ( ) const           { return m_entitiesList; }

    /// @brief Returns the terrain object, or @c nullptr if no terrain was loaded.
    Terrain*                        GetTerrain              ( ) const           { return m_terrain; }

protected:
    WCHAR*                          m_szFileName;

    D3DXVECTOR3                     m_center;   ///< World-space centre of the scene bounding volume.
    float                           m_size;     ///< World-space size of the scene bounding volume.

    DynVec<Entity*>                 m_entitiesList; ///< All entities instantiated by the world loader.

    Terrain*                        m_terrain;  ///< Terrain instance (may be nullptr).

};

#endif // __WORLD_H__
