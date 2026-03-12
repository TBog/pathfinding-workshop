#pragma once

#ifndef __INPUT_H__
#define __INPUT_H__

#include <xinput.h>

//===================================================================
//	struct InputGamePadState
//===================================================================

struct InputGamePadState
{
    bool                        m_isConnected;

    WORD                        m_buttonStates;

    float                       m_leftTrigger;
    float                       m_rightTrigger;
    float                       m_thumbLeftX;
    float                       m_thumbLeftY;
    float                       m_thumbRightX;
    float                       m_thumbRightY;

    InputGamePadState( )
        : m_isConnected         ( false )
        , m_buttonStates        ( 0 )
        , m_leftTrigger         ( 0.f )
        , m_rightTrigger        ( 0.f )
        , m_thumbLeftX          ( 0.f )
        , m_thumbLeftY          ( 0.f )
        , m_thumbRightX         ( 0.f )
        , m_thumbRightY         ( 0.f )
    {}

    void Reset( )
    {
        m_buttonStates = 0;
        m_leftTrigger = 0.f;
        m_rightTrigger = 0.f;
        m_thumbLeftX = 0.f;
        m_thumbLeftY = 0.f;
        m_thumbRightX = 0.f;
        m_thumbRightY = 0.f;
    }

};

//===================================================================
//	CLASS Input
//===================================================================

class Input
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Input( );
    virtual ~Input( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            ProcessMessage  ( UINT message, WPARAM wParam, LPARAM lParam );

    void                            Update          ( );
    
    const InputGamePadState&        GetGamePadState ( int idx )     { return m_gamePadStates[idx]; }
    
    bool                            IsAltPressed    ( )             { return m_altKeyPressed; }
    bool                            IsCtrlPressed    ( )            { return m_ctrlKeyPressed; }
    bool                            IsKeyPressed    ( int keyIdx )  { return m_keysPressed[keyIdx]; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    float                           _NormalizeInput( BYTE axisValue );
    float                           _NormalizeInput( SHORT axisValue );

private:
    InputGamePadState               m_gamePadStates[XUSER_MAX_COUNT];

    static const int kMaxKeysCount = 256;
    bool                            m_keysPressed[kMaxKeysCount];
    bool                            m_altKeyPressed;
    bool                            m_ctrlKeyPressed;

};

#endif // __INPUT_H__
