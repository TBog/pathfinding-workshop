#pragma once

#ifndef __REFCOUNTED_H__
#define __REFCOUNTED_H__

/// @file RefCounted.h
/// @brief Base class for objects with manual COM-style reference counting.

//===================================================================
//	CLASS RefCounted
//===================================================================

/// @class RefCounted
/// @brief Provides manual reference counting (COM-style).
///
/// Objects that derive from @c RefCounted start with a reference count of
/// zero.  The first @ref AddRef() increments it to 1.  When the count falls
/// back to zero via @ref Release(), the object deletes itself.
///
/// Typical usage:
/// @code
/// MyObject* obj = new MyObject();
/// obj->AddRef();    // ref count = 1
/// // ... use obj ...
/// obj->Release();   // ref count = 0 → object is deleted
/// @endcode
///
/// The helper macro @c SAFE_RELEASE (defined in Utils.h) performs a null
/// check before calling @c Release() and then sets the pointer to @c nullptr.
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

    /// @brief Increments the reference count by one.
    void                            AddRef                  ( )     { m_refCount++; }

    /// @brief Decrements the reference count and deletes the object when it reaches zero.
    ///
    /// If the resulting count is ≤ 0, @c delete @c this is called.  Do not
    /// access the object after a @c Release() that may have destroyed it.
    void                            Release                 ( );

    /// @brief Returns the current reference count.
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
