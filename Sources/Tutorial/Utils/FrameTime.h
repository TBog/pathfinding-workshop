#pragma once

#ifndef __FRAMETIME_H__
#define __FRAMETIME_H__

//===================================================================
//	CLASS FrameTime
//===================================================================

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
    void                            Init            ( );
    void                            Update          ( );

    float                           GetDt           ( )     { return m_dt; }
    double                          GetTime         ( )     { return m_time; }

protected:
    //---------------------------------------------------------------
    //	PROTECTED FUNCTIONS
    //---------------------------------------------------------------

private:
    float                           m_dt;
    double                          m_time;
    LONGLONG                        m_lastTime;
    LONGLONG                        m_baseTime;
    LONGLONG                        m_QPFTicksPerSec;

};

#endif // __FRAMETIME_H__
