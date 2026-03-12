#include "pch.h"

#include "FrameTime.h"

//-------------------------------------------------------------------

FrameTime::FrameTime( )
    : m_dt                  ( 0 )
    , m_time                ( 0 )
    , m_lastTime            ( 0 )
    , m_baseTime            ( 0 )
{
    // Use QueryPerformanceFrequency to get the frequency of the counter
    LARGE_INTEGER qwTicksPerSec = { 0 };
    QueryPerformanceFrequency( &qwTicksPerSec );
    m_QPFTicksPerSec = qwTicksPerSec.QuadPart;
}

//-------------------------------------------------------------------

FrameTime::~FrameTime( )
{

}

//-------------------------------------------------------------------

void FrameTime::Init( )
{
    LARGE_INTEGER qwTime;
    QueryPerformanceCounter( &qwTime );

    m_lastTime = qwTime.QuadPart;
    m_baseTime = qwTime.QuadPart;
}

//-------------------------------------------------------------------

void FrameTime::Update( )
{
    LARGE_INTEGER qwTime;
    QueryPerformanceCounter( &qwTime );

    m_dt = (float)( (double)( qwTime.QuadPart - m_lastTime ) / (double)m_QPFTicksPerSec );
    m_lastTime = qwTime.QuadPart;
    m_time = (double)( qwTime.QuadPart - m_baseTime ) / (double)m_QPFTicksPerSec;

    if ( m_dt < 0.f )
        m_dt = 0.f;

    if ( m_time < 0.f )
        m_time = 0.f;
}

//-------------------------------------------------------------------
