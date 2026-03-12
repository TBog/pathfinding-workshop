#pragma once

#ifndef __POSTPROCESS_H__
#define __POSTPROCESS_H__

class Texture;

//===================================================================
//	CLASS PostProcess
//===================================================================

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
    bool                    Load            ( const WCHAR *_szFileName );
    void                    Unload          ( );

    void                    Render          ( Texture *_HDRColorBuffer, Texture *_depthBuffer );

protected:
    WCHAR*                  m_szFileName;

};

#endif // __POSTPROCESS_H__
