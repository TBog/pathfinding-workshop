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
private:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    Input( );
    virtual ~Input( );

public:
    //---------------------------------------------------------------
	//	SINGLETON FUNCTIONS
	//---------------------------------------------------------------
	static Input*                   Get() { return s_Instance; }

	static void                     Create();
	static void                     Destroy();

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            ProcessMessage  ( UINT message, WPARAM wParam, LPARAM lParam );

    void                            Update          ( );
    
    const InputGamePadState&        GetGamePadState ( int idx )     { return m_gamePadStates[idx]; }
    
    bool                            IsAltPressed    ( )             { return m_altKeyPressed; }
    bool                            IsCtrlPressed    ( )            { return m_ctrlKeyPressed; }
    bool                            IsKeyPressed    ( int keyIdx )  { return m_keysPressed[keyIdx]; }

    int                             GetMouseDeltaX  ( ) const       { return m_mouseDeltaX; }
    int                             GetMouseDeltaY  ( ) const       { return m_mouseDeltaY; }
    bool                            IsMouseButtonDown( int btn ) const { return (btn >= 0 && btn < 3) ? m_mouseButtons[btn] : false; }
    void                            ResetMousePosition( int x, int y ) { m_mousePosX = x; m_mousePosY = y; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    float                           _NormalizeInput( BYTE axisValue );
    float                           _NormalizeInput( SHORT axisValue );

	static Input*                   s_Instance;

private:
    InputGamePadState               m_gamePadStates[XUSER_MAX_COUNT];

    static const int kMaxKeysCount = 256;
    bool                            m_keysPressed[kMaxKeysCount];
    bool                            m_altKeyPressed;
    bool                            m_ctrlKeyPressed;

    int                             m_mousePosX;
    int                             m_mousePosY;
    int                             m_mouseDeltaXAccum;
    int                             m_mouseDeltaYAccum;
    int                             m_mouseDeltaX;
    int                             m_mouseDeltaY;
    bool                            m_mouseButtons[3];  // 0=left, 1=right, 2=middle

};

#define g_input Input::Get()

#endif // __INPUT_H__
