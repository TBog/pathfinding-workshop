#include "pch.h"

#include "TexturesManager.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Texture.h"

//-------------------------------------------------------------------

TexturesManager* TexturesManager::s_Instance = NULL;

TexturesManager::TexturesManager( )
{
}

//-------------------------------------------------------------------

TexturesManager::~TexturesManager( )
{
    for ( TexturesMap::iterator it = m_texturesGlobalMap.begin(); it != m_texturesGlobalMap.end(); it++ )
    {
        it->second->Release();
    }

    m_texturesGlobalMap.clear();
}

//-------------------------------------------------------------------

void TexturesManager::Create( )
{
    if ( s_Instance )
    {
        myAssert( false, L"TexturesManager::Create() already called !" );
        return;
    }

    s_Instance = new TexturesManager();
}

//-------------------------------------------------------------------

void TexturesManager::Destroy( )
{
    SAFE_DELETE( s_Instance );
}

//-------------------------------------------------------------------

Texture* TexturesManager::Load( const WCHAR *_szFileName )
{
    if ( _szFileName == NULL )
        return NULL;

    Texture *texture = NULL;

    TexturesMap::iterator it = m_texturesGlobalMap.find( _szFileName );
    if ( it != m_texturesGlobalMap.end() )
    {
        texture = it->second;
    }
    else
    {
        texture = new Texture( _szFileName );

        WCHAR *szFileNameKey = _wcsdup( _szFileName );
        m_texturesGlobalMap.insert( std::make_pair( szFileNameKey, texture ) );

        texture->AddRef();  // add ref here - because we're keeping one ref in the map
    }

    texture->AddRef();  //  add ref for the caller which will be responsible to also release it

    return texture;
}

//-------------------------------------------------------------------

Texture* TexturesManager::Load( const char *_szFileName )
{
    if ( _szFileName == NULL )
        return NULL;

    WCHAR wszFileName[256];
    ConvertStringToWideString( _szFileName, wszFileName );

    return Load( wszFileName );
}

//-------------------------------------------------------------------

