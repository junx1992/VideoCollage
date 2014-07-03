#ifndef __VTL_VECTOR_HPP__
#define __VTL_VECTOR_HPP__


/*************************************************************************\
Oliver Liyin Copyright(c) 2003

Module Name:
  VTL Vector defination and implementation
  
Abstract:
    1. Vector can be accessed as vec[k], vec.at(k), vec.ptr()[k].
    2. VectorStatic is shallow copied type, as an extension of C type array.
       It can be allocated in stack. 
    5. VectorDynamic is heap allocated array, similar to std::vector
       Can support geometry and numerical operations
    
Notes:

Usage:
    1. Use vectorStatic<n, T> and vectorDynamic<T>
       VectorDynamic<T> use resize() to allocate memory
    2. Use Vector2<> Vector3<> Vector4<> for fixed size operation
        
History:
    Created  on 2003 Feb 14 by oliver_liyin
          
\*************************************************************************/

#include "Vtl_Operator.hpp"

namespace vtl{

///  The macro for cross type ctor
///  To make copy constructor from different type of vectors
///  to use this macro, you must also define cross type Copy();
///  NOTE: these template function does not replace default copy ctor and assignment operator
///  i.e. DST(const DST& dst); and DST& operator=(const DST& dst); is still member-wise copy
#define CTOR_CROSS_TYPE_COPY(DST, SRC)                          \
        template <class TYPE, class ACCESS>                     \
        DST(const SRC<TYPE, ACCESS>& src)                       \
        {                                                       \
            Copy(src);                                          \
        }                                                       \
        template <class TYPE, class ACCESS>                     \
        DST& operator = (const SRC<TYPE, ACCESS>& src)          \
        {                                                       \
            Copy(src);                                          \
            return *this;                                       \
        }                                                       \
        DST(const DST& src)                                     \
        {                                                       \
            Copy(src);                                          \
        }                                                       \
        DST& operator = (const DST& src)                        \
        {                                                       \
            Copy(src);                                          \
            return *this;                                       \
        }

/// Enable or disable implicit conversion from vector to pointer type
/// 1. Enabled.
///    The vector can be implicitly converted to T*, which implies operator []
/// 2. Disabled:
///    Access the elements by operator [].
///    Access the pointer by ptr() function explicitly.
#define ENABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T)                    \
    operator T* () { return ptr();}                                 \
    operator const T* () const { return ptr();}                     \

#define DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T)                   \
    T& operator[](size_t nPos) { return ptr()[nPos];}               \
    const T& operator[](size_t nPos) const { return ptr()[nPos];}   \

/// [MACRO for all static access policy]
/// [Add this macro in your own static access policy ].
#define DEFINE_VECTOR_ACCESS_POLICY(T, N, PTR)              \
    const static size_t NDIMS = N;                          \
    size_t  size() const { return N;}                       \
    void clear () {}                                        \
    T* ptr() { return (PTR);}                               \
    const T* ptr() const { return (PTR);}                   \
    void resize(size_t _DEBUG_ONLY(newsize)) { assert(N == newsize);}

/// Macro for all vector access policy
/// define the Rebind from A<T, N>, to new access policy B<S, M>
/// if given S and M is same as T, N, return self
#define DEFINE_VECTOR_ACCESS_REBIND(A, T, N, B, S, M)                       \
    template<class S, size_t M> struct Rebind { typedef B<S, M> Result;};   \
    template<> struct Rebind<T, N> { typedef A Result;}                     \

/// -----------------------------------------------------------------
/// General Vector Interface
///  1. typedef for value, reference, and pointer
///  2. typedef for accumulation and length
///  3. memory function: including size, resize, clear, empty
///  4. access 1D array by ptr(), at(int), operator[](int)
///  5. iterator support, compatible to STL
///  6. default operator, including basic, arithmetic, and geometry  
/// -----------------------------------------------------------------

    /// [Concrete class for Vector implementation]
    /// [Including std::vector compatible access]
    template<class T, class VectorAccessPolicy>
    class Vector : public VectorAccessPolicy
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

    public:
        /// Import VectorAccess Policy
        typedef VectorAccessPolicy AccessType;
        using VectorAccessPolicy::NDIMS;
        using VectorAccessPolicy::size;
        using VectorAccessPolicy::resize;
        using VectorAccessPolicy::clear;
        using VectorAccessPolicy::ptr;
        using VectorAccessPolicy::operator[];

        /// copy resize, resize according to other.size();
        template<class X, class A>
        void resize(const Vector<X, A>& another)
        {
            resize(another.size());
        }

        /// Query if the vector is zero
        bool empty() const { return size() == 0;}

        /// access the memory block by 1D index
        ReferenceType at(size_t nPos) { return ptr()[nPos];}
        ConstReference at(size_t nPos) const { return ptr()[nPos];}

        /// access by iterators
        typedef PointerType iterator;
        iterator begin() { return ptr();}
        iterator end()   { return ptr() + size();}
        iterator rbegin(){ return ptr() + size() - 1;}
        iterator rend()  { return ptr() - 1;}

        /// access by const_iterators
        typedef ConstPointer const_iterator;
        const_iterator begin()  const { return ptr();}
        const_iterator end()    const { return ptr() + size();}
        const_iterator rbegin() const { return ptr() + size() - 1;}
        const_iterator rend()   const { return ptr() - 1;}

    public:
        Vector() {}
        CTOR_CROSS_TYPE_COPY   (Vector, Vector);
        VECTOR_OP_BASIC        (Vector, 0)
        VECTOR_OP_ARITHMATIC   (Vector, 0)
        VECTOR_OP_GEOMETRIC    (Vector, 0)
        VECTOR_OP_COMPARISON   (Vector, 0)

        /// Copy content from one vector to another,
        /// Using RealCast to convert type if necessary
        template<class S, class ANY_ACCESS>
        Vector<T, AccessType>& Copy(const Vector<S, ANY_ACCESS>& src)
        {
            resize(src);
            if (size() > 0)
            {
                T* pDst = ptr();
                const S* pSrc = src.ptr();
                for(size_t k = 0; k < size(); k ++)
                {
                    (*pDst++) = xtl::RealCast<T>(*pSrc++);
                }
            }
            return *this;
        }

        /// Copy from same type vector, using memcpy
        template<class ANY_ACCESS>
        Vector<T, AccessType>& Copy(const Vector<T, ANY_ACCESS>& src)
        {
            resize(src);
            if (size() > 0)
            {
                memcpy(ptr(), src.ptr(), size() * sizeof(T));
            }
            return *this;
        }
    };

/// -----------------------------------------------------------------------------------
/// VectorAccess Policies
/// To define a custom VectorAccess Policy for Vector, you should define
///     1. HRESULT resize(size_t);   /// to resize the vector
///     2. void clear();             /// to deallocate the memory buffer if possible
///     3. size_t size() const;      /// to access the number of items in vector
///     4. T& operator[](size_t);    /// to access the member by index
///     5. T* ptr();                 /// to access memory block pointer
///     6. either DISABLE or ENABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR
///     7. Define Rebind to rebind the access policy to another size
/// -----------------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

    /// general static vector access with dimension of N
    /// Size is fixed at compile time
    template<class T, size_t N>
    class VectorAccess
    {
    public:
        DEFINE_VECTOR_ACCESS_POLICY(T, N, m_pData);
        DEFINE_VECTOR_ACCESS_REBIND(VectorAccess, T, N, VectorAccess, S, M);
        DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T);

    private:
        T m_pData[N];
    };

    /// Dynamic size vector, runtime re-sizable
    template<class T>
    class VectorAccess<T, 0> : protected impl::MemBlock1D<T>
    {
        typedef impl::MemBlock1D<T> super;

    public:
        const static size_t NDIMS = 0;
        using super::resize;
        using super::clear;
        using super::size;
        using super::ptr;

        DEFINE_VECTOR_ACCESS_REBIND(VectorAccess, T, 0, VectorAccess, S, M);
        DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T);
    };

    /// [vector2, VectorAccess by .x and .y]
    template<class T>
    struct VectorAccessXY
    {
        DEFINE_VECTOR_ACCESS_POLICY(T, 2, &x);
        DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T);
        DEFINE_VECTOR_ACCESS_REBIND(VectorAccessXY, T, 2, VectorAccess, S, M);
        T x, y;
    };

    /// [vector3, access by .x, .y, .z]
    template<class T>
    struct VectorAccessXYZ
    {
        DEFINE_VECTOR_ACCESS_POLICY(T, 3, &x);
        DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T);
        DEFINE_VECTOR_ACCESS_REBIND(VectorAccessXYZ, T, 3, VectorAccess, S, M);
        T x, y, z;
    };

    /// [vector4, access by .x, .y, .z, .w]
    template<class T>
    struct VectorAccessXYZW
    {
        DEFINE_VECTOR_ACCESS_POLICY(T, 4, &x);
        DISABLE_IMPLICIT_CONVERT_VECTOR_TO_PTR(T);
        DEFINE_VECTOR_ACCESS_REBIND(VectorAccessXYZW, T, 4, VectorAccess, S, M);
        T x, y, z, w;
    };

#pragma pack(pop)


/// -----------------------------------------------------------------------------------
/// Vector Concrete implementation
///      1. VectorDynamic, a warper from std::vector
///      2. VectorStatic, a warper from C array
///      3. Vector2, 3, 4, using x,y,[z],[w] to access members
///      4. All of them have operator of basic, geometry, and arithmatic
/// -----------------------------------------------------------------------------------

    /// [Concrete VectorDynamic]
    template<class T, class VectorAccessPolicy = VectorAccess<T, 0> >
    class VectorDynamic : public Vector<T, VectorAccessPolicy>
    {
    public:
        VectorDynamic()
        {
            COMPILE_TIME_ASSERT(sizeof(VectorDynamic) == sizeof(VectorAccessPolicy));
        }
        CTOR_CROSS_TYPE_COPY   (VectorDynamic, Vector);

        VectorDynamic(size_t newsize)
        {
            resize(newsize);
        }
    };

    /// [Concrete VectorStatic]
    template<class T, size_t N, class VectorAccessPolicy = VectorAccess<T, N> >
    class VectorStatic : public Vector<T, VectorAccessPolicy>
    {
    public:
        VectorStatic()
        {
            COMPILE_TIME_ASSERT(sizeof(VectorStatic) == sizeof(VectorAccessPolicy));
        }
        CTOR_CROSS_TYPE_COPY   (VectorStatic, Vector);
    };

    /// [Concrete Vector2, including specific 2D vector operation]
    template<class T, class VectorAccessPolicy = VectorAccessXY<T> >
    class Vector2 : public Vector<T, VectorAccessPolicy>
    {
    public:
        Vector2()
        {
            COMPILE_TIME_ASSERT(sizeof(Vector2) == sizeof(VectorAccessPolicy));
        }

        CTOR_CROSS_TYPE_COPY   (Vector2, Vector);
        VECTOR_OP_BASIC        (Vector2, 2)
        VECTOR_OP_ARITHMATIC   (Vector2, 2)
        VECTOR_OP_GEOMETRIC    (Vector2, 2)
        VECTOR_OP_COMPARISON   (Vector2, 2)

        Vector2(T v0, T v1)
        {
            at(0) = v0;   at(1) = v1;
        }

        Vector2& Set(T v0, T v1)
        {
            at(0) = v0;   at(1) = v1;
            return *this;
        }

        /// perpendicular vector, counter clockwise
        const Vector2 Perpendicular() const
        {
            return Vector2(-at(1), at(0));
        }
    };

    typedef Vector2<float>  Vector2F;
    typedef Vector2<double> Vector2D;

    /// [Concrete Vector3, including specific 3D vector operation)
    template<class T, class VectorAccessPolicy = VectorAccessXYZ<T> >
    class Vector3 : public Vector<T, VectorAccessPolicy>
    {
    public:
        Vector3()
        {
            COMPILE_TIME_ASSERT(sizeof(Vector3) == sizeof(VectorAccessPolicy));
        }

        CTOR_CROSS_TYPE_COPY   (Vector3, Vector);
        VECTOR_OP_BASIC        (Vector3, 3)
        VECTOR_OP_ARITHMATIC   (Vector3, 3)
        VECTOR_OP_GEOMETRIC    (Vector3, 3)
        VECTOR_OP_COMPARISON   (Vector3, 3)

        Vector3(T v0, T v1, T v2)
        {
            at(0) = v0;   at(1) = v1;   at(2) = v2;
        }

        Vector3& Set(T v0, T v1, T v2)
        {
            at(0) = v0;   at(1) = v1;   at(2) = v2;
            return *this;
        }

        /// return a cross product
        Vector3 Cross(const Vector3 & v) const
        {
            Vector3 r;
            CrossProduct (r, *this, v);
            return r;
        }
    };

    typedef Vector3<float>  Vector3F;
    typedef Vector3<double> Vector3D;

    /// [Concrete Vector4, including specific 4D vector operation)
    template<class T, class VectorAccessPolicy = VectorAccessXYZW<T> >
    class Vector4 : public Vector<T, VectorAccessPolicy>
    {
    public:
        Vector4()
        {
            COMPILE_TIME_ASSERT(sizeof(Vector4) == sizeof(VectorAccessPolicy));
        }

        CTOR_CROSS_TYPE_COPY   (Vector4, Vector);
        VECTOR_OP_BASIC        (Vector4, 4)
        VECTOR_OP_ARITHMATIC   (Vector4, 4)
        VECTOR_OP_GEOMETRIC    (Vector4, 4)
        VECTOR_OP_COMPARISON   (Vector4, 4)

        Vector4(T v0, T v1, T v2, T v3)
        {
            at(0) = v0;   at(1) = v1;   at(2) = v2;   at(3) = v3;
        }

        Vector4& Set(T v0, T v1, T v2, T v3)
        {
            at(0) = v0;   at(1) = v1;   at(2) = v2;   at(3) = v3;
            return *this;
        }
    };

    typedef Vector4<float>  Vector4F;
    typedef Vector4<double> Vector4D;

    /// Dot product of two vector, return scalar
    template <class T, class AT>
    typename Vector<T, AT>::LengthType
        DotProduct (const Vector<T, AT>& u, const Vector<T, AT>& v)
    {
        typename Vector<T, AT>::LengthType sum = 0;
        assert(u.size() == v.size());
        for(size_t k = 0; k < u.size(); k ++)
        {
            sum += u[k] * v[k];
        }
        return sum;
    }

    /// cross product of two 3D vector, return 3D vector
    template <class T, class AT>
    void CrossProduct (Vector<T, AT>& r, const Vector<T, AT>& u, const Vector<T, AT>& v)
    {
        assert(u.size() == 3 && v.size() == 3);
        r.resize(3);
        r[0] = u[1] * v[2] - u[2] * v[1];
        r[1] = u[2] * v[0] - u[0] * v[2];
        r[2] = u[0] * v[1] - u[1] * v[0];
    }

} /// namespace vtl

#endif//__VTL_VECTOR_HPP__