#include "pch.h"

#include "Material.h"

#include "Texture.h"

//-------------------------------------------------------------------

Material::Material( )
    : m_texturesList( 8, 8 )
{
}

//-------------------------------------------------------------------

Material::~Material( )
{
    for ( int i = 0; i < m_texturesList.GetSize(); i++ )
    {
        SAFE_RELEASE( m_texturesList[i] );
    }
    m_texturesList.Clear( );
}

//-------------------------------------------------------------------

void Material::SetTexture( int slot, Texture* texture )
{
    if ( slot >= m_texturesList.GetSize() )
    {
        int slotsToAdd = slot - m_texturesList.GetSize() + 1;
        for ( int i = 0; i < slotsToAdd; i++ )
        {
            m_texturesList.Add( NULL );
        }
    }

    m_texturesList[slot] = texture;
    if ( texture )
    {
        texture->AddRef( );
    }
}

//-------------------------------------------------------------------
