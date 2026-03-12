#pragma once

#ifndef __AUTOLIST_H__
#define __AUTOLIST_H__

/// @file AutoList.h
/// @brief Intrusive doubly-linked list that self-manages membership via CRTP.

//===================================================================
//	CLASS AutoList
//===================================================================

/// @class AutoList
/// @brief Intrusive doubly-linked list using the Curiously Recurring Template
///        Pattern (CRTP).
///
/// Any class @c T that inherits @c AutoList<T> is automatically inserted into
/// a class-wide global linked list at construction time and removed at
/// destruction time.
///
/// This is primarily used by the @ref Shader hierarchy so that
/// @c Shader::ReloadAll() can iterate every live shader instance without
/// requiring an explicit registration step.
///
/// Example usage:
/// @code
/// class MySystem : public AutoList<MySystem> { ... };
///
/// // Iterate all live instances:
/// for (MySystem* p = MySystem::GetGlobalListHead(); p; p = p->GetNext())
///     p->DoSomething();
/// @endcode
///
/// @warning The list is not thread-safe.  All instances must be created and
///          destroyed on the same thread.
template <class T>
class AutoList
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Inserts @c this into the global list tail.
    AutoList( )
        : m_next( NULL )
        , m_prev( NULL )
    {
        _Add( (T*)this );
    }

    /// @brief Removes @c this from the global list.
    virtual ~AutoList( )
    {
        _Remove( (T*)this );
    }

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Returns the head of the global list, or @c nullptr if empty.
    static T*                       GetGlobalListHead       ( )     { return s_GlobalListHead; }

    /// @brief Returns the next node in the list, or @c nullptr at the tail.
    T*                              GetNext                 ( )     { return m_next; }

private:
    //---------------------------------------------------------------
    //	PRIVATE FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Appends @p _node to the global list tail.
    static void                     _Add                    ( T *_node )
    {
        if ( !s_GlobalListHead )
            s_GlobalListHead = s_GlobalListTail = _node;
        else
        {
            s_GlobalListTail->m_next = _node;
            _node->m_prev = s_GlobalListTail;
            s_GlobalListTail = _node;
        }
    }

    /// @brief Removes @p _node from the global list, repairing the links.
    static void                     _Remove                 ( T *_node )
    {
        if ( _node == s_GlobalListHead )
        {
            s_GlobalListHead = _node->m_next;
            if ( s_GlobalListHead )
                s_GlobalListHead->m_prev = NULL;
            else
                s_GlobalListTail = NULL;
        }
        else if ( _node == s_GlobalListTail )
        {
            s_GlobalListTail = _node->m_prev;
            s_GlobalListTail->m_next = NULL;
        }
        else
        {
            T *tmpNext = _node->m_next;
            _node->m_prev->m_next = tmpNext;
            tmpNext->m_prev = _node->m_prev;
            _node->m_prev = _node->m_next = NULL;
        }
    }

private:
    static T*                       s_GlobalListHead;
    static T*                       s_GlobalListTail;

    T*                              m_next;
    T*                              m_prev;

};

 template<class T> T* AutoList<T>::s_GlobalListHead = NULL;
 template<class T> T* AutoList<T>::s_GlobalListTail = NULL;


#endif // __AUTOLIST_H__
