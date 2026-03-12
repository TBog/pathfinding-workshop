#pragma once

#ifndef __REFCOUNTED_H__
#define __REFCOUNTED_H__

//===================================================================
//	CLASS RefCounted
//===================================================================

class RefCounted
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    RefCounted( ) : m_refCount(0) { }
    virtual ~RefCounted( ) { }

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    void                            AddRef                  ( )     { m_refCount++; }
    void                            Release                 ( );

    int                             GetRefCount             ( )     { return m_refCount; }

private:
    int                             m_refCount;

};

inline void RefCounted::Release( )
{
    m_refCount--;

    if ( m_refCount <= 0 )
    {
        delete this;
    }
}

#endif // __REFCOUNTED_H__
