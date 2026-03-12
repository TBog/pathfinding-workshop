#include "pch.h"

#include "ObjectsManager.h"

#include "..\Utils\Utils.h"
#include "..\Utils\Object.h"

//-------------------------------------------------------------------

ObjectsManager* ObjectsManager::s_Instance = NULL;

ObjectsManager::ObjectsManager( )
{
}

//-------------------------------------------------------------------

ObjectsManager::~ObjectsManager( )
{
    for ( ObjectsMap::iterator it = m_objectsGlobalMap.begin(); it != m_objectsGlobalMap.end(); it++ )
    {
        it->second->Release();
    }

    m_objectsGlobalMap.clear();
}

//-------------------------------------------------------------------

void ObjectsManager::Create( )
{
    if ( s_Instance )
    {
        myAssert( false, L"ObjectsManager::Create() already called !" );
        return;
    }

    s_Instance = new ObjectsManager();
}

//-------------------------------------------------------------------

void ObjectsManager::Destroy( )
{
    SAFE_DELETE( s_Instance );
}

//-------------------------------------------------------------------

Object* ObjectsManager::Load( const WCHAR *_szFileName, float _scale )
{
    if ( _szFileName == NULL )
        return NULL;

    Object *object = NULL;

    ObjectsMapKey key( _szFileName, _scale );
    ObjectsMap::iterator it = m_objectsGlobalMap.find( key );
    if ( it != m_objectsGlobalMap.end() )
    {
        object = it->second;
    }
    else
    {
        object = new Object( _szFileName, _scale );

        WCHAR *szFileNameKey = _wcsdup( _szFileName );
        ObjectsMapKey keyToAdd( szFileNameKey, _scale );
        
        m_objectsGlobalMap.insert( std::make_pair( keyToAdd, object ) );

        object->AddRef();  // add ref here - because we're keeping one ref in the map
    }

    object->AddRef();  //  add ref for the caller which will be responsible to also release it

    return object;
}

//-------------------------------------------------------------------

Object* ObjectsManager::Load( const char *_szFileName, float _scale )
{
    if ( _szFileName == NULL )
        return NULL;

    WCHAR wszFileName[256];
    ConvertStringToWideString( _szFileName, wszFileName );

    return Load( wszFileName, _scale );
}

//-------------------------------------------------------------------

