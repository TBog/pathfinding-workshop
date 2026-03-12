#pragma once

#ifndef __CLOUDS_H__
#define __CLOUDS_H__

class Texture;

//===================================================================
//	CLASS Clouds
//===================================================================

class Clouds
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Clouds                  ( );
    virtual ~Clouds         ( );

public:
    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                    CreateResources ( );
    void                    DestroyResources( );

    bool                    Load            ( const WCHAR *_szFileName );
    void                    Unload          ( );

    void                    PreRender       ( );
    void                    Render          ( Texture *_backColorBuffer, Texture *_backDepthBuffer );

protected:
    WCHAR*                  m_szFileName;

    ID3D11BlendState*       m_cloudsBlendState;

    Texture*                m_cloudsRenderTarget[2];
    int                     m_currentCloudsRT;

    Texture*                m_cloudsBaseMap;
    Texture*                m_cloudsDetailsMap;

    D3DXMATRIX              m_prevViewProjectionMatrix;

};

#endif // __CLOUDS_H__
