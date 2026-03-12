#pragma once

#ifndef __ENTITIESMANAGER_H__
#define __ENTITIESMANAGER_H__

#include "..\Utils\DynVec.h"

class Entity;
class Object;

//===================================================================
//	CLASS EntitiesManager
//===================================================================

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
    static EntitiesManager*         Get                     ( ) { return s_Instance; }

    static void                     Create                  ( );
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            DeleteAllEntities       ( );

    const DynVec<Entity*>&          GetEntitiesList         ( ) const       { return m_entitiesList; }

    Entity*                         CreateEntity            ( const char *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );
    Entity*                         CreateEntity            ( const WCHAR *_szName, Object *_object, const D3DXMATRIX &_worldMatrix );

protected:
    static EntitiesManager*         s_Instance;

    DynVec<Entity*>                 m_entitiesList;

};

#define g_entitiesManager           EntitiesManager::Get()

#endif // __ENTITIESMANAGER_H__
