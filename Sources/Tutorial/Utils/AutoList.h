#pragma once

#ifndef __AUTOLIST_H__
#define __AUTOLIST_H__

//===================================================================
//	CLASS AutoList
//===================================================================

template <class T>
class AutoList
{
public:
    //---------------------------------------------------------------
    //	CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    AutoList( )
        : m_next( NULL )
        , m_prev( NULL )
    {
        _Add( (T*)this );
    }

    virtual ~AutoList( )
    {
        _Remove( (T*)this );
    }

    //---------------------------------------------------------------
    //	MAIN FUNCTIONS
    //---------------------------------------------------------------
    static T*                       GetGlobalListHead       ( )     { return s_GlobalListHead; }
    T*                              GetNext                 ( )     { return m_next; }

private:
    //---------------------------------------------------------------
    //	PRIVATE FUNCTIONS
    //---------------------------------------------------------------
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
