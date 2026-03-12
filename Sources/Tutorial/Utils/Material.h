#pragma once

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

/// @file Material.h
/// @brief Material definition: a set of texture slots used during rendering.

#include "RefCounted.h"
#include "DynVec.h"

class Texture;

//===================================================================
//	CLASS Material
//===================================================================

/// @class Material
/// @brief Holds the texture slots used when rendering a @ref Mesh.
///
/// Slots are indexed by the @c EObjectTextureSlotType enum defined in
/// @ref Object.h (diffuse, normal, roughness, specular, ambient occlusion).
///
/// Inherits @ref RefCounted.
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

    /// @brief Assigns a texture to the given slot, replacing any previous texture.
    ///
    /// The material takes no ownership of the texture pointer; callers are
    /// responsible for keeping the @ref Texture alive as long as the material
    /// uses it.
    ///
    /// @param slot    Texture slot index (see @c EObjectTextureSlotType).
    /// @param texture Pointer to the texture to assign.
    void                            SetTexture              ( int slot, Texture* texture );

    /// @brief Returns the texture assigned to @p slot, or @c nullptr if the slot is empty.
    /// @param slot Texture slot index.
    Texture*                        GetTexture              ( int slot )
    {
        if ( slot >= m_texturesList.GetSize() )
            return NULL;

        return m_texturesList[slot];
    }

protected:
    DynVec<Texture*>                m_texturesList; ///< Per-slot texture list; indexed by EObjectTextureSlotType.

};

#endif // __MATERIAL_H__
