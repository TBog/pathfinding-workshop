#pragma once

#ifndef __OBJECTSMANAGER_H__
#define __OBJECTSMANAGER_H__

/// @file ObjectsManager.h
/// @brief 3-D object cache singleton: loads each (file, scale) combination only once.

#include <map>

class Object;

//===================================================================
//	CLASS ObjectsManager
//===================================================================

/// @class ObjectsManager
/// @brief Singleton cache for @ref Object assets.
///
/// Objects are keyed by both filename **and** scale factor so that the same
/// OBJ file loaded at different scales produces distinct @ref Object instances
/// with correctly scaled geometry.
///
/// Access the singleton via the @c g_objectsManager macro.
class ObjectsManager
{
private:
    /// @brief Composite key for the objects map (filename + scale).
    struct ObjectsMapKey
    {
        const WCHAR*        m_szFileName;   ///< Wide-character file path.
        float               m_scale;        ///< Uniform scale factor.

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

    /// @brief Returns the singleton instance (may be @c nullptr before Create()).
    static ObjectsManager*          Get                     ( ) { return s_Instance; }

    /// @brief Creates the singleton instance.  Must be called exactly once.
    static void                     Create                  ( );

    /// @brief Destroys the singleton instance and releases all cached objects.
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads an object from a wide-character OBJ path, or returns the cached instance.
    ///
    /// @param _szFileName Wide-character path to the OBJ file.
    /// @param _scale      Uniform scale applied to all vertex positions at load time.
    /// @return Pointer to the loaded @ref Object, or @c nullptr on failure.
    Object*                         Load                    ( const WCHAR *_szFileName, float _scale );

    /// @brief Loads an object from a narrow-character OBJ path, or returns the cached instance.
    ///
    /// The narrow path is converted to a wide-character string internally.
    /// @param _szFileName Narrow-character path to the OBJ file.
    /// @param _scale      Uniform scale applied to all vertex positions at load time.
    /// @return Pointer to the loaded @ref Object, or @c nullptr on failure.
    Object*                         Load                    ( const char *_szFileName, float _scale );

protected:
    static ObjectsManager*          s_Instance;

    ObjectsMap                      m_objectsGlobalMap; ///< (filename, scale) → Object* cache.

};

/// @brief Global convenience accessor for @ref ObjectsManager.
#define g_objectsManager            ObjectsManager::Get()

#endif // __OBJECTSMANAGER_H__
