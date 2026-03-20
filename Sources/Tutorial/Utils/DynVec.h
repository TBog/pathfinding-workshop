#pragma once

#ifndef __DYNVEC_H__
#define __DYNVEC_H__

#include "Utils.h"

//===================================================================
//    CLASS Object
//===================================================================

template <typename T>
class DynVec
{
public:
    //---------------------------------------------------------------
    //    CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------
    DynVec( int size, int grow )
        : m_size        ( size )
        , m_grow        ( grow )
        , m_cursor      ( 0 )
    {
        myAssert( m_size, L"DynVec() - size must be > 0!" );
        m_vec = (T *)malloc( sizeof(T) * m_size );
    }

    ~DynVec()
    {
        m_size = 0;
        m_grow = 0;
        m_cursor = -1;
        SAFE_FREE( m_vec );
    }
    
    //---------------------------------------------------------------
    //    MAIN FUNCTIONS
    //---------------------------------------------------------------
    // Adds a element to the end of the container. If the number of elements has
    // reached the container capacity the size of the container is raised with the 
    // grow. If the specified grow is zero, the container does not resize;
    // Returns the index in the vector;
    // Note: the index can be invalid after a remove operation;
    int Add( const T &e )
    {
        if ( !CheckRealloc( 1 ) )
            return -1;

        m_vec[m_cursor] = e;
        return m_cursor ++;
    }

    // Insert an element before or after the index.
    int Insert( const T &e, int index, bool before = true )
    {
        if ( !CheckRealloc( 1 ) )
            return -1;

        myAssert( ( index >= 0 ) && ( index <= m_cursor ), L"DynVec()::Insert() - Invalid index for insert!" );

        if( !before )
            index++;

        if ( index < m_cursor )
            memmove( m_vec + index + 1, m_vec + index, (m_cursor - index) * sizeof(T) );

        m_vec[index] = e;
        m_cursor++;
        return index;
    }


    // Returns a reference to the object at the position specified by the parameter.
    T &operator[] ( int index )
    { 
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }
    const T &operator[] ( int index )     const
    { 
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }

    // removes the element at the position specified as parameter from the container.
    // Note: The indices in the container can be made invalid after a call to this method
    void Remove( int index )
    {
        myAssert((index >= 0) && (index < m_cursor), L"DynVec()::[] - Index out of bounds!");
        m_vec[index] = m_vec[--m_cursor];
    }
    
    void RemoveKeepOrder( int index )
    {
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        for( int i = index; i < m_cursor - 1; i++ )
            m_vec[i] = m_vec[i+1];
        
        --m_cursor;
    }

    // same as the operator []
    T &At( int index )
    {
        myAssert((index >= 0) && (index < m_cursor), L"DynVec()::[] - Index out of bounds!");
        return m_vec[index];
    }
    const T& AtConst( int index ) const
    {
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }

    void Clear()
    {
        m_cursor = 0;
    }

    void SetSize(int s, bool init)
    {
        myAssert( ( s >= 0) && ( s <= m_size ), L"DynVec()::SetSize() - Size out of bounds!" );
        m_cursor = s;
        if(init)
            memset( m_vec, 0, s * sizeof(T) );
    }

    int GetSize ()      const   { return m_cursor; }
    int GetMaxSize()    const   { return m_size; }

    // changes the growth params
    void SetAllocParams(int size, int grow)
    {
        if ( m_grow < grow )
            m_grow = grow;

        if ( m_size < size )
        {
            T *newVec = (T *)malloc( sizeof(T) * size );
            memmove( newVec, m_vec, sizeof(T) * m_cursor );        // copy only existing elements
            m_size = size;
            free(m_vec);
            m_vec = newVec;
        }
    }

    void FillWithValue( const T &value )
    {
        for ( int i = 0; i < m_cursor; i++ )
            m_vec[i] = value;
    }

    int Find( const T &value )
    {
        for( int i = 0; i < m_cursor; i++ )
            if ( m_vec[i] == value )
                return i;

        return -1;
    }

	bool Contains(const T& value) const
	{
		for (int i = 0; i < m_cursor; i++)
		{
			if (m_vec[i] == value)
				return true;
		}
		return false;
	}

    T *GetLast( )
    {
        if( m_cursor )
            return &m_vec[m_cursor - 1];

        return NULL;
    }

    void DeleteLast( )
    {
        if(m_cursor > 0)
            m_cursor--;
    }

    T *GetData( )       { return m_vec; }

private:
    // WARNING copy constructor and operator= should never be called for DynVec.
    DynVec( const DynVec& v )                       { }
    const DynVec& operator=( const DynVec&v )       { }


protected:
    //---------------------------------------------------------------
    //    PROTECTED FUNCTIONS
    //---------------------------------------------------------------
    bool CheckRealloc( int numNewElems )
    {
        if ( m_cursor + numNewElems <= m_size )
            return true;

        if ( m_grow == 0 )
            return false;

        int numGrows = ((m_cursor + numNewElems - m_size - 1) / m_grow) + 1;
        myAssert( numGrows > 0, L"DynVec()::CheckRealloc() - numGrows == 0" );
        m_size += m_grow * numGrows;

        m_vec = (T*)realloc( m_vec, sizeof(T) * m_size );

        return true;
    }

protected:
    T*                              m_vec;      // vector of elements
    int                             m_size;     // the total allocated size in T units;
    int                             m_grow;     // the growth of the container
    int                             m_cursor;   // the position where should be added a new element

};

#endif //__DYNVEC_H__
