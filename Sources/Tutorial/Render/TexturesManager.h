#pragma once

#ifndef __TEXTURESMANAGER_H__
#define __TEXTURESMANAGER_H__

/// @file TexturesManager.h
/// @brief Texture cache singleton: loads each texture file only once.

#include <map>

class Texture;

//===================================================================
//	CLASS TexturesManager
//===================================================================

/// @class TexturesManager
/// @brief Singleton texture cache.
///
/// Maintains a @c std::map keyed by filename.  Calling @ref Load more than
/// once with the same filename returns the already-loaded @ref Texture pointer
/// instead of loading the file again.
///
/// Access the singleton via the @c g_texturesManager macro.
class TexturesManager
{
private:
    /// @brief Comparator for wide-character string keys.
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

    /// @brief Returns the singleton instance (may be @c nullptr before Create()).
    static TexturesManager*         Get                     ( ) { return s_Instance; }

    /// @brief Creates the singleton instance.  Must be called exactly once.
    static void                     Create                  ( );

    /// @brief Destroys the singleton instance and releases all cached textures.
    static void                     Destroy                 ( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads a texture from a wide-character file path, or returns the cached instance.
    /// @param _szFileName Wide-character path to the image file.
    /// @return Pointer to the loaded @ref Texture, or @c nullptr on failure.
    Texture*                        Load                    ( const WCHAR *_szFileName );

    /// @brief Loads a texture from a narrow-character file path, or returns the cached instance.
    ///
    /// The narrow path is converted to a wide-character string internally.
    /// @param _szFileName Narrow-character path to the image file.
    /// @return Pointer to the loaded @ref Texture, or @c nullptr on failure.
    Texture*                        Load                    ( const char *_szFileName );

protected:
    static TexturesManager*         s_Instance;

    TexturesMap                     m_texturesGlobalMap;    ///< Filename → Texture* cache.

};

/// @brief Global convenience accessor for @ref TexturesManager.
#define g_texturesManager           TexturesManager::Get()

#endif // __TEXTURESMANAGER_H__
