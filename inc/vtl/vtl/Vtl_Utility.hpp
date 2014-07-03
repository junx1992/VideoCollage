#ifndef __VTL_UTILITY_HPP__
#define __VTL_UTILITY_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
    VTL utility helpers

Abstract:
    --  functions for zero comparison
        AbsLessThan
        EqualsTo
        IsZero

    --  numeric helper function
        Lerp
        Square
        Sqrt


    -- MemBlock1D, wrapper of 1D array.
        Automatic free, and fast buffer swap.
        a) resize(sizt_t)       /// to allocate give size of array
        b) clear()              /// to clear all memory
        c) size_t size() const  /// to access array size
        d) ptr()                /// to access buffer pointer
        e) swap(other)          /// fast buffer swap

    -- MemBlock2D, wrapper of 2D array

Notes:


History:
    Created  on 2003 Aug 14 by oliver_liyin
    Modified on 2003 Aug 14 by oliver_liyin

\*************************************************************************/

#include "xtl/Xtl_Utility.hpp"
#include "xtl/Xtl_Memory.hpp"

namespace vtl   {

    ///  General helper functions

    /// Absolute value of a is less than b,  |a| < b
    template<class T>
    bool AbsLessThan(T a, T b)
    {
        COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsArithmetic);
        if (xtl::TypeTraits<T>::IsUnsigned)
        {
            return a < b;
        }
        else
        {
            return (a < b) && (a > -b);
        }
    }

    /// if a and b are close enough as same value
    template<class T>
    bool EqualsTo(T a, T b, T threshold = xtl::Limits<T>::denorm_min())
    {
        COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsArithmetic);
        /// NOTE: if a, b is short, a-b may be int
        return IsZero(T(a - b), threshold);
    }

    /// if value is close enough to zero
    template<class T>
    bool IsZero(T value, T threshold = xtl::Limits<T>::denorm_min())
    {
        COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsArithmetic);
        if (xtl::TypeTraits<T>::IsInteger)
        {
            return (value == 0);
        }
        else
        {
            return (-value < threshold) && (value < threshold);
        }
    }

    /// Linear interpolation.  = (1-a) * u + a * v
    template <class T, class S>
    T Lerp(const T& u, const T& v, S alpha)
    {
        if (alpha == 0) return u;
        else if (alpha == 1) return v;
        else return xtl::RealCast<T>((v - u) * alpha + u);
    }

    /// square, template version, sometimes optimized better
    template<class T>
    typename xtl::TypeTraits<T>::AccumType Square(const T& a)
    {
        COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsArithmetic);
        typedef typename xtl::TypeTraits<T>::AccumType AccumType;
        AccumType b = xtl::RealCast<AccumType>(a);
        return b * b;
    }

    /// square, template version, sometimes optimized better
    template<class T>
    typename xtl::TypeTraits<T>::LengthType Sqrt(const T& a)
    {
        typedef typename xtl::TypeTraits<T>::LengthType LengthType;
        COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsArithmetic);
        COMPILE_TIME_ASSERT(xtl::TypeTraits<LengthType>::IsReal);
        return sqrt(xtl::RealCast<LengthType>(a));
    }

    namespace impl
    {
        /// MemBlock1D,
        /// allocate 1D memory block, behave like C-like array
        /// Usage:
        ///      1. void resize(newsize);    /// resize the array
        ///      2. void clear();            /// free the memory, It'll be done when dtor
        ///      3. size_t size();           /// access the size of array
        ///      4. T* ptr()                 /// access the ptr of the buffer
        ///      5. swap(Memblock& other)    /// swap only the buffer ptr and size
        /// Note:
        ///      1. It can only be derived, e.g. vtl::VectorDynamic<T>
        ///      2. It cannot be copied
        //
        template<class T, class AllocatorPolicy = DEFAULT_ALLOCATOR>
        class MemBlock1D
        {
        protected:
            /// must derived by child class
            MemBlock1D() : m_ptr(NULL), m_size(0)
            {
            }

            ~MemBlock1D()
            {
                clear();
            }

            size_t size() const { return m_size;}

            T* ptr() { return m_ptr;}
            const T* ptr() const { return m_ptr;}

            void resize(size_t newsize)
            {
                /// if same size, ignore resize
                if (newsize == m_size) return;

                clear();
                m_size = newsize;

                /// if emptry, return without allocation
                if (m_size == 0) return;

                m_ptr = static_cast<T*>(AllocatorPolicy::Allocate(
                    m_size * sizeof(T) _DEBUG_ONLY_AB(__FILE__, __LINE__)));

                /// if allocation fail, set empty
                if (m_ptr == NULL)
                {
                    m_size = 0;
                    return;
                }
            }

            void clear()
            {
                if (m_ptr != NULL)
                {
                    AllocatorPolicy::Deallocate(m_ptr, m_size * sizeof(T));
                    m_ptr = NULL;
                    m_size = 0;
                }
            }

        private:
            T* m_ptr;
            size_t m_size;

        private: /// forbit copy constructor
            MemBlock1D(const MemBlock1D&);
            MemBlock1D& operator = (const MemBlock1D&);
        };


        /// MemBlock2D,
        /// allocate 2D memory block, behave like C-like array
        /// Usage:
        ///      1. void resize(row, col);   /// resize the array
        ///      2. void clear();            /// free the memory, It'll be done when dtor
        ///      3. size_t size();           /// access the size mem block
        ///      4. T* ptr()                 /// access the ptr of the memblock
        ///      5. T* rowptr(row)           /// access the ptr of the memblock
        ///      5. swap(Memblock& other)    /// swap only the buffer ptr and size
        /// Note:
        ///      1. It can only be derived, e.g. vtl::VectorDynamic<T>
        ///      2. It cannot be copied
        //
        template<class T, class AllocatorPolicy = DEFAULT_ALLOCATOR>
        class MemBlock2D
        {
        protected:
            typedef T* RowPtrType;

            /// must derived by custum class
            MemBlock2D()
                : m_ptr(NULL), m_ppRows(NULL), m_rows(0), m_cols(0)
            {
            }

            ~MemBlock2D()
            {
                clear();
            }

            size_t size() const { return m_rows * m_cols;}

            size_t nRows() const { return m_rows;}
            size_t nCols() const { return m_cols;}

            /// access the 1D ptr of memory block
            T* ptr() { return m_ptr;}
            const T* ptr() const { return m_ptr;}

            /// access the ptr of each row
            T* operator[](size_t row) { return m_ppRows[row];}
            const T* operator[](size_t row) const { return m_ppRows[row];}

            /// access the 2D ptr of all row pointers
            RowPtrType* ptrptr() { return m_ppRows;}
            const RowPtrType* ptrptr() const { return m_ppRows;}

            void resize(size_t rows, size_t cols)
            {
                /// if same size, ignore resize
                if (rows == m_rows && cols == m_cols) return;

                /// if emptry, return without allocation
                if (rows == 0 || rows == 0) goto clear_and_return;

                /// if same memory size, only modify row pointers
                if (rows * cols == m_rows * m_cols)
                {
                    if (m_ppRows != NULL)
                    {
                        AllocatorPolicy::Deallocate(m_ppRows, m_rows * sizeof(T*));
                    }

                    m_rows = rows;
                    m_cols = cols;
                    m_ppRows = static_cast<T**>(
                        AllocatorPolicy::Allocate(m_rows * sizeof(T*)
                        _DEBUG_ONLY_AB(__FILE__, __LINE__)));

                    if (m_ppRows == NULL) goto clear_and_return;
                }
                else
                {
                    clear();
                    m_rows = rows;
                    m_cols = cols;
                    m_ppRows = static_cast<T**>(AllocatorPolicy::Allocate(
                        m_rows * sizeof(T*) _DEBUG_ONLY_AB( __FILE__, __LINE__)));
                    if (m_ppRows == NULL) goto clear_and_return;

                    m_ptr = static_cast<T*>(AllocatorPolicy::Allocate(
                        size() * sizeof(T) _DEBUG_ONLY_AB(__FILE__, __LINE__)));
                    if (m_ptr == NULL) goto clear_and_return;
                }

                /// initialize the row pointers
                for(size_t k = 0; k < m_rows; k ++)
                {
                    m_ppRows[k] = m_ptr + k * m_cols;
                }

                return;

clear_and_return:
                clear();
                return;
            }

            void clear()
            {
                if (m_ppRows != NULL)
                {
                    AllocatorPolicy::Deallocate(m_ppRows, m_rows * sizeof(T*));
                    m_ppRows = NULL;
                }
                if (m_ptr != NULL)
                {
                    AllocatorPolicy::Deallocate(m_ptr, size() * sizeof(T));
                    m_ptr = NULL;
                    m_rows = m_cols = 0;
                }
            }

        private:
            T* m_ptr;
            T** m_ppRows;
            size_t m_rows, m_cols;

        private:    /// forbit copy constructor
            MemBlock2D(const MemBlock2D&);
            MemBlock2D& operator = (const MemBlock2D&);
        };

    } /// namespace impl


} /// namespace vtl

#endif /// __VTL_UTILITY_HPP__
