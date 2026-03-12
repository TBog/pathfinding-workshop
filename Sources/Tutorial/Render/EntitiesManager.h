#pragma once

#ifndef __ENTITIESMANAGER_H__
#define __ENTITIESMANAGER_H__

/// @file EntitiesManager.h
/// @brief Factory and owner of all scene @ref Entity instances.

#include "..\Utils\DynVec.h"

class Entity;
class Object;

//===================================================================
//	CLASS EntitiesManager
//===================================================================

/// @class EntitiesManager
/// @brief Singleton factory that creates and owns all @ref Entity instances.
///
/// Created entities are ref-counted.  The manager holds one reference per
/// entity; additional owners should call @ref Entity::AddRef.  When
/// @ref DeleteAllEntities (or the destructor) is called, the manager
/// releases its reference to every entity.
///
/// Access the singleton via the @c g_entitiesManager macro.
class EntitiesManager
{
private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    EntitiesManager         ( );
    virtual ~EntitiesManager( );

public:
    //---------------------------------------------------------------
    //	SINGLETON FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Returns the singleton instance (may be @c nullptr before Create()).
    static EntitiesManager*         Get                     ( ) { return s_Instance; }

    /// @brief Creates the singleton instance.  Must be called exactly once.
    static void                     Create                  ( );

    /// @brief Destroys the singleton instance and releases all entities.
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Releases all entities and clears the entity list.
    void                            DeleteAllEntities       ( );

    /// @brief Returns the full list of managed entities.
    const DynVec<Entity*>&          GetEntitiesList         ( ) const       { return m_entitiesList; }

    /// @brief Creates a new entity from a narrow-character name, object, and world transform.
    ///
    /// The returned entity is owned by the manager (ref count 1).  Call
    /// @ref Entity::AddRef if additional owners need to keep a reference.
    ///
    /// @param _szName      Narrow-character display name (converted internally).
    /// @param _object      Shared 3-D object.
    /// @param _worldMatrix World-space transform.
    /// @return Pointer to the new @ref Entity, or @c nullptr on failure.
    Entity*                         CreateEntity            ( const char *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );

    /// @brief Creates a new entity from a wide-character name, object, and world transform.
    ///
    /// @param _szName      Wide-character display name.
    /// @param _object      Shared 3-D object.
    /// @param _worldMatrix World-space transform.
    /// @return Pointer to the new @ref Entity, or @c nullptr on failure.
    Entity*                         CreateEntity            ( const WCHAR *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );

protected:
    static EntitiesManager*         s_Instance;

    DynVec<Entity*>                 m_entitiesList; ///< All managed entity instances.

};

/// @brief Global convenience accessor for @ref EntitiesManager.
#define g_entitiesManager           EntitiesManager::Get()

#endif // __ENTITIESMANAGER_H__
