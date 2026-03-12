#pragma once

#ifndef __DYNVEC_H__
#define __DYNVEC_H__

/// @file DynVec.h
/// @brief Dynamic (resizable) array template backed by malloc/realloc.

#include "Utils.h"

//===================================================================
//    CLASS DynVec
//===================================================================

/// @class DynVec
/// @brief Resizable array of plain-data elements backed by @c malloc / @c realloc.
///
/// Designed for use with trivially copyable types (e.g. pointers, POD structs).
/// Elements are stored in contiguous memory; no constructors or destructors are
/// invoked on the elements themselves.
///
/// The container has two capacity parameters:
/// - **size**  — initial (and minimum) allocated capacity in elements.
/// - **grow**  — number of elements by which to expand when the current
///               capacity is exhausted.  A grow value of @c 0 disables
///               automatic resizing (Add/Insert will fail silently).
///
/// @warning The copy constructor and assignment operator are intentionally
///          declared private and not implemented.  Do not copy @c DynVec
///          instances.
template <typename T>
class DynVec
{
public:
    //---------------------------------------------------------------
    //    CONSTRUCTOR / DESTRUCTOR
    //---------------------------------------------------------------

    /// @brief Constructs a @c DynVec with the given initial capacity and grow step.
    /// @param size Initial capacity in elements (must be > 0).
    /// @param grow Number of elements to add per reallocation (0 = no resize).
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

    /// @brief Appends an element to the end of the container.
    ///
    /// If the container is full and @c grow > 0, the internal buffer is
    /// reallocated.  If @c grow == 0 and the buffer is full, the element is
    /// silently discarded and @c -1 is returned.
    ///
    /// @note Returned indices may become invalid after a subsequent @ref Remove.
    /// @param e Element to append.
    /// @return The index of the newly added element, or @c -1 on failure.
    int Add( const T &e )
    {
        if ( !CheckRealloc( 1 ) )
            return -1;

        m_vec[m_cursor] = e;
        return m_cursor ++;
    }

    /// @brief Inserts an element before (or after) the specified index.
    ///
    /// All elements from @p index onward are shifted by one position using
    /// @c memmove.
    ///
    /// @param e      Element to insert.
    /// @param index  Position at which to insert.
    /// @param before If @c true (default), insert *before* @p index; if
    ///               @c false, insert *after* @p index.
    /// @return The index at which the element was inserted, or @c -1 on failure.
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


    /// @brief Returns a reference to the element at @p index.
    /// @param index Zero-based element index (must be < @ref GetSize()).
    T &operator[] ( int index )
    { 
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }

    /// @brief Returns a const reference to the element at @p index.
    /// @param index Zero-based element index (must be < @ref GetSize()).
    const T &operator[] ( int index )     const
    { 
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }

    /// @brief Removes the element at @p index by swapping it with the last element.
    ///
    /// This is an O(1) operation but does **not** preserve element order.
    /// Use @ref RemoveKeepOrder when insertion order must be maintained.
    ///
    /// @note Stored indices obtained from previous @ref Add / @ref Insert calls
    ///       may become invalid after this call.
    /// @param index Zero-based index of the element to remove.
    void Remove( int index )
    {
        myAssert((index >= 0) && (index < m_cursor), L"DynVec()::[] - Index out of bounds!");
        m_vec[index] = m_vec[--m_cursor];
    }

    /// @brief Removes the element at @p index while preserving the order of remaining elements.
    ///
    /// Shifts all elements after @p index one position to the left (O(n)).
    /// @param index Zero-based index of the element to remove.
    void RemoveKeepOrder( int index )
    {
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        for( int i = index; i < m_cursor - 1; i++ )
            m_vec[i] = m_vec[i+1];
        
        --m_cursor;
    }

    /// @brief Returns a reference to the element at @p index (same as @c operator[]).
    /// @param index Zero-based element index (must be < @ref GetSize()).
    T &At( int index )
    {
        myAssert((index >= 0) && (index < m_cursor), L"DynVec()::[] - Index out of bounds!");
        return m_vec[index];
    }

    /// @brief Returns a const reference to the element at @p index.
    /// @param index Zero-based element index (must be < @ref GetSize()).
    const T& AtConst( int index ) const
    {
        myAssert( ( index >= 0 ) && ( index < m_cursor ), L"DynVec()::[] - Index out of bounds!" );
        return m_vec[index];
    }

    /// @brief Resets the element count to zero without freeing memory.
    void Clear()
    {
        m_cursor = 0;
    }

    /// @brief Sets the logical size of the container to @p s.
    ///
    /// @p s must be within [0, @ref GetMaxSize()].  If @p init is @c true the
    /// first @p s elements are zero-initialised.
    /// @param s    New element count.
    /// @param init If @c true, zero-initialises the first @p s elements.
    void SetSize(int s, bool init)
    {
        myAssert( ( s >= 0) && ( s <= m_size ), L"DynVec()::SetSize() - Size out of bounds!" );
        m_cursor = s;
        if(init)
            memset( m_vec, 0, s * sizeof(T) );
    }

    /// @brief Returns the current number of elements (logical size).
    int GetSize ()      const   { return m_cursor; }

    /// @brief Returns the total allocated capacity in elements.
    int GetMaxSize()    const   { return m_size; }

    /// @brief Updates the allocation parameters.
    ///
    /// If @p grow is larger than the current grow step it is updated.
    /// If @p size is larger than the current capacity, the internal buffer is
    /// reallocated to hold @p size elements (existing elements are preserved).
    ///
    /// @param size New minimum capacity in elements.
    /// @param grow New minimum grow step in elements.
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

    /// @brief Sets every element in the container to @p value.
    /// @param value Value to assign to each element.
    void FillWithValue( const T &value )
    {
        for ( int i = 0; i < m_cursor; i++ )
            m_vec[i] = value;
    }

    /// @brief Searches for the first element equal to @p value using @c operator==.
    /// @param value Value to search for.
    /// @return Zero-based index of the first matching element, or @c -1 if not found.
    int Find( const T &value )
    {
        for( int i = 0; i < m_cursor; i++ )
            if ( m_vec[i] == value )
                return i;

        return -1;
    }

    /// @brief Returns a pointer to the last element, or @c nullptr if the container is empty.
    T *GetLast( )
    {
        if( m_cursor )
            return &m_vec[m_cursor - 1];

        return NULL;
    }

    /// @brief Decrements the element count by one (removes the last element).
    /// No-op if the container is already empty.
    void DeleteLast( )
    {
        if(m_cursor > 0)
            m_cursor--;
    }

    /// @brief Returns a raw pointer to the underlying element array.
    T *GetData( )       { return m_vec; }

private:
    // WARNING copy constructor and operator= should never be called for DynVec.
    DynVec( const DynVec& v )                       { }
    const DynVec& operator=( const DynVec&v )       { }


protected:
    //---------------------------------------------------------------
    //    PROTECTED FUNCTIONS
    //---------------------------------------------------------------

    /// @brief Ensures the buffer has room for @p numNewElems additional elements.
    ///
    /// If there is insufficient space and @c m_grow > 0, the buffer is
    /// reallocated in multiples of @c m_grow until sufficient capacity exists.
    ///
    /// @param numNewElems Number of elements that must fit after the call.
    /// @return @c true if there is sufficient capacity; @c false if
    ///         @c m_grow == 0 and the buffer is full.
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
