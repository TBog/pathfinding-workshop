#pragma once

#ifndef __POSTPROCESS_H__
#define __POSTPROCESS_H__

/// @file PostProcess.h
/// @brief HDR tone-mapping post-process pass.

class Texture;

//===================================================================
//	CLASS PostProcess
//===================================================================

/// @class PostProcess
/// @brief Applies HDR tone-mapping (and any other post-process effects) to produce
///        the final LDR back-buffer image.
///
/// Reads an HDR floating-point colour buffer and a depth buffer, and writes a
/// tone-mapped image ready for display.  Configuration is loaded from an XML file.
class PostProcess
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    PostProcess             ( );
    virtual ~PostProcess    ( );

public:
    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads post-process configuration from an XML file.
    /// @param _szFileName Wide-character path to the post-process XML descriptor.
    /// @return @c true on success.
    bool                    Load            ( const WCHAR *_szFileName );

    /// @brief Releases all resources.
    void                    Unload          ( );

    /// @brief Executes the post-process pass.
    ///
    /// Binds @p _HDRColorBuffer and @p _depthBuffer as shader inputs and renders
    /// a full-screen quad to the currently set render target.
    ///
    /// @param _HDRColorBuffer Floating-point HDR colour buffer to tone-map.
    /// @param _depthBuffer    Scene depth buffer (used for depth-of-field, fog, etc.).
    void                    Render          ( Texture *_HDRColorBuffer, Texture *_depthBuffer );

protected:
    WCHAR*                  m_szFileName;   ///< Path to the post-process XML descriptor.

};

#endif // __POSTPROCESS_H__
