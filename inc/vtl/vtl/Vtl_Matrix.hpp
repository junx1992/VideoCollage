#ifndef __VTL_MATRIX_HPP__
#define __VTL_MATRIX_HPP__


/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
  VTL Matrix defination and implementation
  
Abstract:
    1. Matrix can be accessed as mat(row, col).
    2. A number of matrix types are defined in uniform interface
    3. All C++ operators defined on Matrix are applied on each element
    4. MatrixStatic is trivial copied type, as an extension of C type 2D array.
       It can be allocated in stack. 
    5. MatrixDynamic is complex copied type, as C type pointer of pointer,
    
Notes:
     
Usage:
        
History:
    Created  on 2003 Feb 14 by oliver_liyin

\*************************************************************************/

#include "xtl/Xtl_Pointer.hpp"
#include "xtl/Xtl_RealCast.hpp"
#include "vtl/Vtl_Utility.hpp"
#include "vtl/Vtl_Operator.hpp"
#include "vtl/Vtl_Vector.hpp"

#include <vector>
#include <numeric>

namespace vtl{

    template<class T, class A> class Matrix;
    template<class T, size_t M, size_t N> class MatrixAccess;
    template<class T, size_t M, size_t N> class MatrixAccessRef;

    /// Matrix access static, compile time fixed size
    template<class T, size_t M, size_t N>
    class MatrixAccess
    {
        COMPILE_TIME_ASSERT(M > 0 && N > 0);
    public:
        const static size_t NROWS = M;
        const static size_t NCOLS = N;

        typedef T RowPtrType[NCOLS];

        template<class S, size_t MM, size_t NN>
        struct Rebind
        {
            typedef MatrixAccess<S, MM, NN> Result;
        };

    public:
        size_t nRows() const { return NROWS;}
        size_t nCols() const { return NCOLS;}

        /// resize the matrix, for static matrix, it always do nothing
        void resize(size_t _DEBUG_ONLY(rows), size_t _DEBUG_ONLY(cols))
        {
            assert(rows == NROWS && cols == NCOLS);
        }

        /// number of elements in mem-block
        size_t size() const { return NROWS * NCOLS;}

        /// pointer to the mem-block
        T* ptr()       { return(T*) m_pData;}
        const T* ptr() const { return(T*) m_pData;}

        /// access the static matrix by mat[row][col]
        T* operator [](size_t row)       { return m_pData[row];}
        const T* operator [](size_t row) const { return m_pData[row];}

        /// access matrix by 2 level pointer, a = ptrptr(); a[i][j];
        RowPtrType* ptrptr() { return m_pData;}
        const RowPtrType* ptrptr() const { return m_pData;}

    private:
        /// C standard, 2D array is stored in row major continuous mem-block
        T m_pData[NROWS][NCOLS];
    };

    /// Matrix access dynamic, dynamic allocated matrix in heap
    template<class T>
    class MatrixAccess<T, 0, 0> : protected impl::MemBlock2D<T>
    {
        typedef impl::MemBlock2D<T> super;

    public:
        const static size_t NROWS = 0;
        const static size_t NCOLS = 0;

        template<class S, size_t MM, size_t NN>
        struct Rebind
        {
            typedef MatrixAccess<S, MM, NN> Result;
        };

    public:
        using super::nCols;
        using super::nRows;
        using super::resize;
        using super::size;
        using super::ptr;
        using super::ptrptr;
        using super::RowPtrType;
        using super::operator [];
    };

    template<class T, size_t M, size_t N>
    class MatrixAccessRef
    {
        typedef MatrixAccess<T, M, N> NativeAccessType;

    public:
        const static size_t NROWS = M;
        const static size_t NCOLS = N;

        typedef typename NativeAccessType::RowPtrType RowPtrType;

        template<class S, size_t MM, size_t NN>
        struct Rebind
        {
            typedef MatrixAccessRef<S, MM, NN> Result;
        };

    public:
        MatrixAccessRef() : m_pRef(new NativeAccessType) {}

        void ResetRef() { m_pRef = new NativeAccessType;}

        size_t nRows() const { return (*m_pRef).nRows();}
        size_t nCols() const { return (*m_pRef).nCols();}

        /// resize the matrix, for static matrix, it always do nothing
        void resize(size_t crows, size_t ccols) { (*m_pRef).resize(crows, ccols);}

        /// number of elements in mem-block
        size_t size() const { return (*m_pRef).size();}

        /// pointer to the mem-block
        T* ptr()                { return (*m_pRef).ptr();}
        const T* ptr() const    { return (*m_pRef).ptr();}

        /// access the static matrix by mat[row][col]
        T* operator [](size_t row)              { return (*m_pRef)[row];}
        const T* operator [](size_t row) const  { return (*m_pRef)[row];}

        /// access matrix by 2 level pointer, a = ptrptr(); a[i][j];
        RowPtrType* ptrptr() { return (*m_pRef).ptrptr();}
        const RowPtrType* ptrptr() const { return (*m_pRef).ptrptr();}

    private:
        xtl::SharedPtr<NativeAccessType> m_pRef;
    };

    /// a matrix access policy must define:
    ///  1. nRows() const and nCols() const, for dimension
    ///  3. resize(row, col);    /// resize the matrix
    ///  4. size() const, number of elements in mem-block
    ///  5. ptr(), pointer to the mem-block
    ///  6. operator[], access row pointer, mat[row][col]
    ///  7. ptrptr(), access by 2-level pointer, mat[row][col]
    ///  8. typedef RowPtrType;  the pointer type to access a row of data

    /// This is a general interface for matrix access
    template<class T, class MatrixAccessPolicy>
    class Matrix : public MatrixAccessPolicy
    {
    public:
        typedef typename xtl::TypeTraits<T>::ValueType      ValueType;
        typedef typename xtl::TypeTraits<T>::AccumType      AccumType;
        typedef typename xtl::TypeTraits<T>::LengthType     LengthType;

#ifdef ENABLE_TYPE_TRAITS_EX
        typedef typename xtl::TypeTraitsEx<T>::ConstValue     ConstValue;
        typedef typename xtl::TypeTraitsEx<T>::ReferenceType  ReferenceType;
        typedef typename xtl::TypeTraitsEx<T>::ConstReference ConstReference;
        typedef typename xtl::TypeTraitsEx<T>::PointerType    PointerType;
        typedef typename xtl::TypeTraitsEx<T>::ConstPointer   ConstPointer;
#else
        typedef const T     ConstValue;
        typedef T&          ReferenceType;
        typedef const T&    ConstReference;
        typedef T*          PointerType;
        typedef const T*    ConstPointer;
#endif

        typedef MatrixAccessPolicy AccessType;
        using AccessType::NROWS;
        using AccessType::NCOLS;

        typedef Matrix<T, typename AccessType::Rebind<T, NCOLS, NROWS>::Result> TransposedMatrix;

        template<class T, class MA, class MB>
        struct ProductMatrixType
        {
            COMPILE_TIME_ASSERT(MA::NCOLS == MB::NROWS);
            typedef Matrix<T, typename MA::Rebind<T, MA::NROWS, MB::NCOLS>::Result> ResultType;
        };

        template<class T, class MA, class VA>
        struct ProductVectorType
        {
            COMPILE_TIME_ASSERT(MA::NCOLS == VA::NDIMS);
            typedef Vector<T, typename VA::Rebind<T, MA::NROWS>::Result> ResultType;
        };

    public:
        using AccessType::nRows;          /// matrix height
        using AccessType::nCols;          /// matrix width
        using AccessType::resize;         /// resize and allocate
        using AccessType::size;           /// access mem block size,
        using AccessType::ptr;            /// access mem block pointer
        using AccessType::operator[];     /// access row pointer, mat[row][col]
        using AccessType::ptrptr;         /// access by 2-level pointer, mat[row][col]
        using AccessType::RowPtrType;     /// the pointer type to access a row of data

        Matrix() {}
        CTOR_CROSS_TYPE_COPY   (Matrix, Matrix);
        VECTOR_OP_BASIC        (Matrix, 0);
        VECTOR_OP_ARITHMATIC   (Matrix, 0);
        VECTOR_OP_COMPARISON   (Matrix, 0);


        /// access by operator() as 2D array
        T& operator()(size_t row, size_t col)             { return (*this)[row][col];}
        const T& operator()(size_t row, size_t col) const { return (*this)[row][col];}

        /// access mem-block as 1D block
        T& at(size_t nPos)       { return *(ptr() + nPos);}
        const T& at(size_t nPos) const { return *(ptr() + nPos);}

        template<class S, class ANY_ACCESS>
        Matrix<T, AccessType>& Copy(const Matrix<S, ANY_ACCESS>& src)
        {
            T* pDst = ptr();
            if(size() > 0)
            {
                const S* pSrc = src.ptr();
                for(size_t k = 0; k < size(); k ++)
                {
                    (*pDst++) = xtl::RealCast<T>(*pSrc++);
                }
            }
            return *this;
        }

        template<class ANY_ACCESS>
        Matrix<T, AccessType>& Copy(const Matrix<T, ANY_ACCESS>& src)
        {
            resize(src);
            if(size() > 0)
            {
                memcpy(ptr(), src.ptr(), size() * sizeof(T));
            }
            return *this;
        }

        /// resize according to another matrix, maybe another type
        template<class S, class A>
        void resize(const Matrix<S, A> & other)
        {
            resize(other.nRows(), other.nCols());
        }

        bool IsEmpty() const { return nRows() == 0 || nCols() == 0;}
        bool IsSquare() const { return nRows() == nCols();}

        /// make an identity matrix
        void SetOne()
        {
            SetZero();
            for (size_t n = 0; n < min(nRows(), nCols()); n ++)
            {
                (*this)(n, n) = 1;
            }
        }

        const TransposedMatrix Transposed() const
        {
            TransposedMatrix result;
            MatrixTranspose(result, *this);
            return result;
        }

        /// get the invert matrix, must not be singular
        void Invert()
        {
            /// Defined in vtl_Lud.hpp
            MatrixInvert(*this);
        }

        const Matrix Inverted() const
        {
            Matrix result;
            result.Copy(*this);
            result.Invert();
            return result;
        }

        /// multiply another matrix
        template<class RhsMatrixAccess>
        const typename ProductMatrixType<T, AccessType, RhsMatrixAccess>::ResultType
            Multiply(const Matrix<T, RhsMatrixAccess>& Rhs) const
        {
            typename ProductMatrixType<T, AccessType, RhsMatrixAccess>::ResultType result;
            MatrixMultiply(result, *this, Rhs);
            return result;
        }

        /// multiply another vector
        template<class RhsVectorAccess>
        const typename ProductVectorType<T, AccessType, RhsVectorAccess>::ResultType
            Multiply(const vtl::Vector<T, RhsVectorAccess>& Rhs) const
        {
            typename ProductVectorType<T, AccessType, RhsVectorAccess>::ResultType result;
            MatrixMultiply(result, *this, Rhs);
            return result;
        }

        /// Determinant of matrix, using LU decomposition
        T Determinant() const
        {
            /// Defined in vtl_Lud.hpp
            return MatrixDeterminant(*this);
        }

        /// transpose the dynamic matrix
        void Transpose()
        {
            const static bool fSquare = (NROWS == NCOLS);
            const static bool fStatic = (NROWS > 0) && (NCOLS > 0);
            TransposeMatrix_T<Matrix, fSquare, fStatic>::Transpose(*this);
        }

    private:
        template<class MatrixType, bool fSquare, bool fStatic>
        struct TransposeMatrix_T
        {
            /// Dynamic matrix
            static void Transpose (MatrixType& mat)
            {
                if (mat.IsSquare())
                {
                    MatrixTranspose(mat, mat);
                }
                else
                {
                    typename MatrixType::TransposedMatrix result;
                    MatrixTranspose(result, mat);
                    mat.Swap(result);
                }
            }
        };

        template<class MatrixType>
        struct TransposeMatrix_T<MatrixType, false, true>
        {
            /// non-square, static matrix
            static void Transpose (MatrixType& mat)
            {
                /// We cannot transpose a non-square static matrix
                /// You have to store the result in another matrix
                assert(false);
            }
        };

        template<class MatrixType>
        struct TransposeMatrix_T<MatrixType, true, true>
        {
            /// square, static matrix
            static void Transpose (MatrixType& mat)
            {
                assert(mat.IsSquare());
                MatrixTranspose(mat, mat);
            }
        };
    };


    /// MatrixDynamic for resizable matrix
    template<class T>
    class MatrixDynamic : public Matrix<T, MatrixAccess<T, 0, 0> >
    {
    public:
        MatrixDynamic() {}
        CTOR_CROSS_TYPE_COPY   (MatrixDynamic, Matrix);
    };

    template<class T>
    class MatrixDynamicRef : public Matrix<T, MatrixAccessRef<T, 0, 0 > >
    {
    public:
        MatrixDynamicRef() {}
        CTOR_CROSS_TYPE_COPY   (MatrixDynamicRef, Matrix);
    };

    /// This is the static sized matrix
    ///  can not resize, and may be allocated in stack frame
    template<class T, size_t NROWS, size_t NCOLS>
    class MatrixStatic : public Matrix<T, MatrixAccess<T, NROWS, NCOLS> > 
    {
    public:
        MatrixStatic() {};
        CTOR_CROSS_TYPE_COPY   (MatrixStatic, Matrix);
    };


    template<class T, size_t NROWS, size_t NCOLS>
    class MatrixStaticRef : public Matrix<T, MatrixAccessRef<T, NROWS, NCOLS> >
    {
    public:
        MatrixStaticRef() {};
        CTOR_CROSS_TYPE_COPY   (MatrixStaticRef, Matrix);
    };

    /// This is the square matrix
    /// which as a lot helper functions, like transpose, invert, reset
    template<class T, size_t NDIMS>
    class MatrixSquare : public Matrix<T, MatrixAccess<T, NDIMS, NDIMS> >
    {
    public:
        MatrixSquare() {};
        CTOR_CROSS_TYPE_COPY   (MatrixSquare, Matrix);
    };

    template<class T, size_t NDIMS>
    class MatrixSquareRef : public Matrix<T, MatrixAccessRef<T, NDIMS, NDIMS> >
    {
    public:
        MatrixSquareRef() {};
        CTOR_CROSS_TYPE_COPY   (MatrixSquareRef, Matrix);
    };


    /// typical usage of matrix square
    typedef MatrixSquare<float , 2> Matrix2x2F;
    typedef MatrixSquare<float , 3> Matrix3x3F;
    typedef MatrixSquare<float , 4> Matrix4x4F;

    typedef MatrixSquare<double, 2> Matrix2x2D;
    typedef MatrixSquare<double, 3> Matrix3x3D;
    typedef MatrixSquare<double, 4> Matrix4x4D;

    /// Matrix multiply a vector
    /// Only process same element type vectors and matrix
    template<class T, class TA, class L, class LA, class R, class RA>
    void MatrixMultiply( vtl::Vector<T, TA>& result,
                         const Matrix<L, LA>& lhs,
                         const vtl::Vector<R, RA>& rhs)
    {
        const size_t M = lhs.nRows();
        const size_t N = lhs.nCols();
        assert(N == rhs.size());
        if (N != rhs.size()) return;

		result.resize(M);
        for (size_t m = 0; m < M; m++)
        {
            T& one = result[m] = 0;
            for (size_t n = 0; n < N; n++)
            {
                // one += lhs(m, n) * rhs[n];
                const L& vl = lhs(m, n);
                const R& vr = rhs[n];
                if (vl != 0 && vr != 0)
                {
                    one += xtl::RealCast<T>(vl * vr);
                }
            }
        }
    }

    /// a vector multiply a matrix
    template<class T, class TA, class L, class LA, class R, class RA>
    void MatrixMultiply( vtl::Vector<T, TA>& result,
                         const vtl::Vector<L, LA>& lhs,
                         const Matrix<R, RA>& rhs)
    {
        const size_t M = rhs.nRows();
        const size_t N = rhs.nCols();
		assert(M == lhs.size());
        if (M != lhs.size()) return;
		
        result.resize(N);
        for (size_t n = 0; n < N; n++)
        {
            T& one = result[n] = 0;
            for (size_t m = 0; m < M; m++)
            {
                // one += xtl::RealCast<T>(lhs[m] * rhs(m, n));
                const L& vl = lhs[m];
                const R& vr = rhs(m, n);
                if (vl != 0 && vr != 0)
                {
                    one += xtl::RealCast<T>(vl * vr);
                }
            }
        }
    }

    /// matrix multiply matrix
    template<class T, class TA, class L, class LA, class R, class RA>
    void MatrixMultiply(Matrix<T, TA>& result,
                        const Matrix<L, LA>& lhs,
                        const Matrix<R, RA>& rhs)
    {
        const size_t M = lhs.nRows();
        const size_t N = rhs.nCols();
		assert(lhs.nCols() == rhs.nRows());
        if (lhs.nCols() != rhs.nRows()) return;
		
        const size_t K = rhs.nRows();
        result.resize(M, N);
        for (size_t m = 0; m < M; m++)
        {
            for (size_t n = 0; n < N; n ++) 
            {
                T& one = result(m, n) = 0;
                for (size_t l = 0; l < K; l++)
                {
                    //one += xtl::RealCast<T>((lhs)(m, l) * rhs(l, n));
                    const L& vl = lhs(m, l);
                    const R& vr = rhs(l, n);
                    if (vl != 0 && vr != 0)
                    {
                        one += xtl::RealCast<T>(vl * vr);
                    }
                }
            }
        }
    }

    /// three matrix multiply sequencially
    template<class T, class TA, 
             class L, class LA, class M, class MA, class R, class RA>
    void MatrixMultiply( Matrix<T, TA>& result,
                        const Matrix<L, LA>& lhs,
                        const Matrix<M, MA>& mhs,
                        const Matrix<R, RA>& rhs)
    {
        MatrixDynamic<R> tmp;
        MatrixMultiply(tmp, lhs, mhs);
        MatrixMultiply(result, tmp, rhs);
    }

    /// matrix transpose, the square matrix condition can be done in compile time
    template<class T, class A>
    void MatrixTranspose(typename Matrix<T, A>::TransposedMatrix& dst, const Matrix<T, A>& src)
    {
        const size_t nRows = src.nRows();
        const size_t nCols = src.nCols();

        /// inplace transpose require square matrix
        if (dst.IsSquare() && ((INT_PTR)(&dst) == (INT_PTR)(&src)))
        {
            for (size_t r = 0; r < nRows; r ++)
            {
                for (size_t c = r+1; c < nCols; c ++)
                {
                    std::swap(dst(r, c), dst(c, r));
                }
            }
        }
        else
        {
            /// if you get assert here, and dst is MatrixStatic
            /// then you have used incorrect matrix dimension
            dst.resize(nCols, nRows);
            for (size_t r = 0; r < nRows; r ++)
            {
                for (size_t c = 0; c < nCols; c ++)
                {
                    dst(c, r) = src(r, c);
                }
            }
        }
    }

} /// namespace vtl


#endif//__VTL_MATRIX_HPP__