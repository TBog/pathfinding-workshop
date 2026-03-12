#pragma once

#ifndef __SKY_H__
#define __SKY_H__

//===================================================================
//	CLASS Sky
//===================================================================

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
    bool                    Load            ( const WCHAR *_szFileName );
    void                    Unload          ( );

    void                    Render          ( );

    //---------------------------------------------------------------
    //	SETTER / GETTER FUNCTIONS
    //---------------------------------------------------------------
    void                    SetLightParams( const D3DXVECTOR3 &_light_dir, const D3DXVECTOR3 &_light_color, float _light_power, const D3DXVECTOR3 &_light_ambientColor, float _light_ambientPower);
    const D3DXVECTOR3&      GetLightDir     ( )                                 { return m_light_dir; }
    const D3DXVECTOR3&      GetLightColor   ( )                                 { return m_light_color; }
    float                   GetLightPower   ( )                                 { return m_light_power; }
    const D3DXVECTOR3&      GetLightAmbientColor( )                             { return m_light_ambientColor; }
    float                   GetLightAmbientPower( )                             { return m_light_ambientPower; }

    void                    SetSkyColorParams( const D3DXVECTOR3 &_sky_color, float _sky_lightPower );
    const D3DXVECTOR3&      GetSkyColor     ( )                                 { return m_sky_color; }
    float                   GetSkyLightPower( )                                 { return m_sky_lightPower; }

    void                    SetFogParams    ( const D3DXVECTOR3 &_fog_color, float _fog_lightPower, float _fog_start, float _fog_end );
    const D3DXVECTOR3&      GetFogColor     ( )                                 { return m_fog_color; }
    float                   GetFogLightPower( )                                 { return m_fog_lightPower; }
    float                   GetFogStart     ( )                                 { return m_fog_start; }
    float                   GetFogEnd       ( )                                 { return m_fog_end; }

protected:
    WCHAR*                  m_szFileName;

    D3DXVECTOR3             m_light_dir;
    D3DXVECTOR3             m_light_color;
    float                   m_light_power;
    D3DXVECTOR3             m_light_ambientColor;
    float                   m_light_ambientPower;

    D3DXVECTOR3             m_sky_color;
    float                   m_sky_lightPower;

    D3DXVECTOR3             m_fog_color;
    float                   m_fog_lightPower;
    float                   m_fog_start;
    float                   m_fog_end;

};

#endif // __SKY_H__
