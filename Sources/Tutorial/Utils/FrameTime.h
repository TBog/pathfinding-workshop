#pragma once

#ifndef __FRAMETIME_H__
#define __FRAMETIME_H__

/// @file FrameTime.h
/// @brief High-resolution per-frame timer using QueryPerformanceCounter.

//===================================================================
//	CLASS FrameTime
//===================================================================

/// @class FrameTime
/// @brief Measures elapsed time between frames using the Windows
///        high-resolution performance counter.
///
/// Typical usage:
/// @code
/// FrameTime ft;
/// ft.Init();
/// // each frame:
/// ft.Update();
/// float dt = ft.GetDt();   // seconds since last frame
/// @endcode
class FrameTime
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    FrameTime( );
    virtual ~FrameTime( );

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Initialises the timer and records the base timestamp.
    ///
    /// Must be called once before the first @ref Update().  Queries
    /// @c QueryPerformanceFrequency to determine the counter resolution.
    void                            Init            ( );

    /// @brief Advances the timer by one frame.
    ///
    /// Computes @ref GetDt (delta-time) and @ref GetTime (total elapsed time)
    /// from the current performance-counter value.  Call once per frame.
    void                            Update          ( );

    /// @brief Returns the elapsed time (in seconds) since the last @ref Update().
    float                           GetDt           ( )     { return m_dt; }

    /// @brief Returns the total elapsed time (in seconds) since @ref Init().
    double                          GetTime         ( )     { return m_time; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

private:
    float                           m_dt;               ///< Delta-time for the current frame (seconds).
    double                          m_time;             ///< Total elapsed time since Init() (seconds).
    LONGLONG                        m_lastTime;         ///< Performance counter value at the last Update().
    LONGLONG                        m_baseTime;         ///< Performance counter value at Init().
    LONGLONG                        m_QPFTicksPerSec;   ///< Counter frequency in ticks per second.

};

#endif // __FRAMETIME_H__
