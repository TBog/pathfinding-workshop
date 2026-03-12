#pragma once

#ifndef __WORLD_H__
#define __WORLD_H__

#include "..\Utils\DynVec.h"

class Entity;
class Sky;
class Terrain;

//===================================================================
//	CLASS World
//===================================================================

class World
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    World                           ( const WCHAR *_szFileName );
    virtual ~World                  ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    bool                            Load                    ( const WCHAR *_szFileName );
    void                            Unload                  ( );

    const DynVec<Entity*>&          GetEntitiesList         ( ) const           { return m_entitiesList; }

    Terrain*                        GetTerrain              ( ) const           { return m_terrain; }

protected:
    WCHAR*                          m_szFileName;

    D3DXVECTOR3                     m_center;
    float                           m_size;

    DynVec<Entity*>                 m_entitiesList;

    Terrain*                        m_terrain;

};

#endif // __WORLD_H__
