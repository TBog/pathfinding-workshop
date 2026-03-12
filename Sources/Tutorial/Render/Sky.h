#pragma once

#ifndef __SKY_H__
#define __SKY_H__

/// @file Sky.h
/// @brief Sky-dome renderer and light / fog parameter store.

//===================================================================
//	CLASS Sky
//===================================================================

/// @class Sky
/// @brief Renders a sky-dome and exposes the scene's directional light,
///        ambient light, sky colour, and atmospheric fog parameters.
///
/// These parameters are read by terrain, object, and cloud shaders via
/// the per-frame and per-pass constant buffers uploaded by
/// @ref RenderManager::SetPerFrameConstantBuffers.
class Sky
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Sky                     ( );
    virtual ~Sky            ( );

public:
    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Loads sky configuration (mesh, textures, shader parameters) from an XML file.
    /// @param _szFileName Wide-character path to the sky XML descriptor.
    /// @return @c true on success.
    bool                    Load            ( const WCHAR *_szFileName );

    /// @brief Releases all resources.
    void                    Unload          ( );

    /// @brief Renders the sky-dome around the camera position.
    void                    Render          ( );

    //---------------------------------------------------------------
    //	SETTER / GETTER FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Sets all directional light parameters in one call.
    ///
    /// @param _light_dir          Normalised world-space direction *toward* the light source.
    /// @param _light_color        RGB colour of the directional light.
    /// @param _light_power        Intensity multiplier for the directional light.
    /// @param _light_ambientColor RGB colour of the ambient (fill) light.
    /// @param _light_ambientPower Intensity multiplier for the ambient light.
    void                    SetLightParams( const D3DXVECTOR3 &_light_dir, const D3DXVECTOR3 &_light_color, float _light_power, const D3DXVECTOR3 &_light_ambientColor, float _light_ambientPower);

    /// @brief Returns the normalised world-space direction toward the directional light.
    const D3DXVECTOR3&      GetLightDir     ( )                                 { return m_light_dir; }

    /// @brief Returns the RGB colour of the directional light.
    const D3DXVECTOR3&      GetLightColor   ( )                                 { return m_light_color; }

    /// @brief Returns the intensity multiplier of the directional light.
    float                   GetLightPower   ( )                                 { return m_light_power; }

    /// @brief Returns the RGB colour of the ambient light.
    const D3DXVECTOR3&      GetLightAmbientColor( )                             { return m_light_ambientColor; }

    /// @brief Returns the intensity multiplier of the ambient light.
    float                   GetLightAmbientPower( )                             { return m_light_ambientPower; }

    /// @brief Sets the sky-dome colour and its light-power contribution.
    /// @param _sky_color      RGB colour of the sky zenith.
    /// @param _sky_lightPower Intensity of the sky's indirect contribution.
    void                    SetSkyColorParams( const D3DXVECTOR3 &_sky_color, float _sky_lightPower );

    /// @brief Returns the sky-dome zenith colour.
    const D3DXVECTOR3&      GetSkyColor     ( )                                 { return m_sky_color; }

    /// @brief Returns the sky indirect-light intensity.
    float                   GetSkyLightPower( )                                 { return m_sky_lightPower; }

    /// @brief Sets all atmospheric fog parameters.
    ///
    /// @param _fog_color      RGB colour of the fog.
    /// @param _fog_lightPower Intensity of the fog's self-illumination.
    /// @param _fog_start      World-space distance at which fog begins.
    /// @param _fog_end        World-space distance at which fog is fully opaque.
    void                    SetFogParams    ( const D3DXVECTOR3 &_fog_color, float _fog_lightPower, float _fog_start, float _fog_end );

    /// @brief Returns the fog colour.
    const D3DXVECTOR3&      GetFogColor     ( )                                 { return m_fog_color; }

    /// @brief Returns the fog self-illumination intensity.
    float                   GetFogLightPower( )                                 { return m_fog_lightPower; }

    /// @brief Returns the world-space distance at which fog begins.
    float                   GetFogStart     ( )                                 { return m_fog_start; }

    /// @brief Returns the world-space distance at which fog reaches full opacity.
    float                   GetFogEnd       ( )                                 { return m_fog_end; }

protected:
    WCHAR*                  m_szFileName;

    D3DXVECTOR3             m_light_dir;            ///< Normalised direction toward the light.
    D3DXVECTOR3             m_light_color;          ///< Directional light RGB colour.
    float                   m_light_power;          ///< Directional light intensity.
    D3DXVECTOR3             m_light_ambientColor;   ///< Ambient light RGB colour.
    float                   m_light_ambientPower;   ///< Ambient light intensity.

    D3DXVECTOR3             m_sky_color;            ///< Sky zenith colour.
    float                   m_sky_lightPower;       ///< Sky indirect-light intensity.

    D3DXVECTOR3             m_fog_color;            ///< Fog colour.
    float                   m_fog_lightPower;       ///< Fog self-illumination intensity.
    float                   m_fog_start;            ///< Distance at which fog begins.
    float                   m_fog_end;              ///< Distance at which fog is fully opaque.

};

#endif // __SKY_H__
