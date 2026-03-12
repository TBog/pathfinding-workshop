#pragma once

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "RefCounted.h"
#include "DynVec.h"

class Texture;

//===================================================================
//	CLASS Material
//===================================================================

class Material : public RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Material                        ( );
    virtual ~Material               ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            SetTexture              ( int slot, Texture* texture );
    Texture*                        GetTexture              ( int slot )
    {
        if ( slot >= m_texturesList.GetSize() )
            return NULL;

        return m_texturesList[slot];
    }

protected:
    DynVec<Texture*>                m_texturesList;

};

#endif // __MATERIAL_H__
