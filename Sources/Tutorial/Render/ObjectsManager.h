#pragma once

#ifndef __OBJECTSMANAGER_H__
#define __OBJECTSMANAGER_H__

#include <map>

class Object;

//===================================================================
//	CLASS ObjectsManager
//===================================================================

class ObjectsManager
{
private:
    struct ObjectsMapKey
    {
        const WCHAR*        m_szFileName;
        float               m_scale;

        ObjectsMapKey( const WCHAR* _szFileName, float _scale ) : m_szFileName( _szFileName ), m_scale( _scale ) {}

        bool const operator==( const ObjectsMapKey &o ) const
        {
            int cmp = wcscmp( m_szFileName, o.m_szFileName );
            return ( (cmp == 0) && (m_scale == o.m_scale) );
        }

        bool const operator<( const ObjectsMapKey &o ) const
        {
            int cmp = wcscmp( m_szFileName, o.m_szFileName );
            return ( (cmp < 0) || ((cmp == 0) && (m_scale < o.m_scale)));
        }
    };

    typedef std::map< ObjectsMapKey, Object* > ObjectsMap;

private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    ObjectsManager          ( );
    virtual ~ObjectsManager ( );

public:
    //---------------------------------------------------------------
    //	SINGLETON FUNCTIONS
    //---------------------------------------------------------------
    static ObjectsManager*          Get                     ( ) { return s_Instance; }

    static void                     Create                  ( );
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    Object*                         Load                    ( const WCHAR *_szFileName, float _scale );
    Object*                         Load                    ( const char *_szFileName, float _scale );

protected:
    static ObjectsManager*          s_Instance;

    ObjectsMap                      m_objectsGlobalMap;

};

#define g_objectsManager            ObjectsManager::Get()

#endif // __OBJECTSMANAGER_H__
