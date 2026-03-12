#pragma once

#ifndef __TEXTURESMANAGER_H__
#define __TEXTURESMANAGER_H__

#include <map>

class Texture;

//===================================================================
//	CLASS TexturesManager
//===================================================================

class TexturesManager
{
private:
    struct cmp_str
    {
        bool operator()(const WCHAR *a, const WCHAR *b) const
        {
            return wcscmp(a, b) < 0;
        }
    };

    typedef std::map< const WCHAR*, Texture*, cmp_str > TexturesMap;

private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    TexturesManager         ( );
    virtual ~TexturesManager( );

public:
    //---------------------------------------------------------------
    //	SINGLETON FUNCTIONS
    //---------------------------------------------------------------
    static TexturesManager*         Get                     ( ) { return s_Instance; }

    static void                     Create                  ( );
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    Texture*                        Load                    ( const WCHAR *_szFileName );
    Texture*                        Load                    ( const char *_szFileName );

protected:
    static TexturesManager*         s_Instance;

    TexturesMap                     m_texturesGlobalMap;

};

#define g_texturesManager           TexturesManager::Get()

#endif // __TEXTURESMANAGER_H__
