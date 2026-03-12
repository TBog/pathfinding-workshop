#pragma once

#ifndef __INPUT_H__
#define __INPUT_H__

/// @file Input.h
/// @brief Keyboard and XInput gamepad input management.

#include <xinput.h>

//===================================================================
//	struct InputGamePadState
//===================================================================

/// @struct InputGamePadState
/// @brief Snapshot of a single XInput gamepad's state for one frame.
///
/// All axis values are normalised to the range [-1, 1] (thumbsticks)
/// or [0, 1] (triggers).  The @c m_buttonStates bitmask uses the
/// standard XInput button constants (e.g. @c XINPUT_GAMEPAD_A).
struct InputGamePadState
{
    /// @brief Whether this gamepad slot is connected.
    bool                        m_isConnected;

    /// @brief Bitmask of currently pressed buttons (XInput constants).
    WORD                        m_buttonStates;

    float                       m_leftTrigger;   ///< Left trigger value [0, 1].
    float                       m_rightTrigger;  ///< Right trigger value [0, 1].
    float                       m_thumbLeftX;    ///< Left thumbstick X-axis [-1, 1].
    float                       m_thumbLeftY;    ///< Left thumbstick Y-axis [-1, 1].
    float                       m_thumbRightX;   ///< Right thumbstick X-axis [-1, 1].
    float                       m_thumbRightY;   ///< Right thumbstick Y-axis [-1, 1].

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

    /// @brief Resets all axis and button values to zero (does not change @c m_isConnected).
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

/// @class Input
/// @brief Manages keyboard state and up to @c XUSER_MAX_COUNT XInput gamepad states.
///
/// Keyboard events are captured via @ref ProcessMessage which should be called
/// from the application's @c WndProc.  Gamepad state is polled each frame
/// via @ref Update.
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

    /// @brief Processes a Windows message for keyboard input.
    ///
    /// Should be called from the application's @c WndProc for every message.
    /// Updates internal key-pressed flags on @c WM_KEYDOWN and @c WM_KEYUP.
    ///
    /// @param message Windows message identifier.
    /// @param wParam  Message word parameter.
    /// @param lParam  Message long parameter.
    void                            ProcessMessage  ( UINT message, WPARAM wParam, LPARAM lParam );

    /// @brief Polls all XInput gamepad slots and updates their state snapshots.
    ///
    /// Call once per frame before reading gamepad state.
    void                            Update          ( );

    /// @brief Returns the current state snapshot of the gamepad at slot @p idx.
    /// @param idx Gamepad slot index in [0, @c XUSER_MAX_COUNT).
    const InputGamePadState&        GetGamePadState ( int idx )     { return m_gamePadStates[idx]; }

    /// @brief Returns @c true if either Alt key is currently held.
    bool                            IsAltPressed    ( )             { return m_altKeyPressed; }

    /// @brief Returns @c true if either Ctrl key is currently held.
    bool                            IsCtrlPressed    ( )            { return m_ctrlKeyPressed; }

    /// @brief Returns @c true if the key with virtual-key code @p keyIdx is currently pressed.
    /// @param keyIdx Virtual-key code (0–255).
    bool                            IsKeyPressed    ( int keyIdx )  { return m_keysPressed[keyIdx]; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Normalises an unsigned 8-bit axis value to [0, 1].
    float                           _NormalizeInput( BYTE axisValue );

    /// @brief Normalises a signed 16-bit thumbstick axis value to [-1, 1].
    float                           _NormalizeInput( SHORT axisValue );

private:
    InputGamePadState               m_gamePadStates[XUSER_MAX_COUNT];

    static const int kMaxKeysCount = 256;
    bool                            m_keysPressed[kMaxKeysCount];   ///< Per-key pressed state (indexed by virtual-key code).
    bool                            m_altKeyPressed;
    bool                            m_ctrlKeyPressed;

};

#endif // __INPUT_H__
