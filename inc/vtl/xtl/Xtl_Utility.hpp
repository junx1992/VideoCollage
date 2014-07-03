#ifndef __XTL_UTILITY_HPP__
#define __XTL_UTILITY_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2004

Module Name:

Abstract:

Notes:

Usage:
    1. COMPILE_TIME_ASSERT

    2. IS_SAME_TYPE

    3. TypeTraits<T>
        1. typedef ValueType, wrap T
        2. typedef ConstValue, and UnconstValue
        3. typedef reference, and ConstReference
        4. typedef pointer, and ConstPointer
        5. typedef accume_type and LengthType
        6. bool IsSigned, IsUnsigned, IsInteger
        7. bool IsReal, IsArithmetic
        9. bool IsVoid
        10.bool IsConst
        11.bool IsPointer, IsReference, IsArray
        12.bool IsConstValue, IsConstPtr, IsConstRef
        13.bool IsMemFunPtr

    4. Limits<T> --  wrapper from std::numeric_limits<T>
        1. denorm_min   /// min value that 1/x will not be overflow.
        2. epsilon      /// min value that 1 + x == x
        3. max_value    /// for_all b, b <= a
        4. min_value    /// for_all b, b >= a


History:
    Created  on 2004 Nov 17 by t-yinli@microsoft.com

\*************************************************************************/

#pragma once

#include "xtl/Xtl_Platform.hpp"

#include <limits>

#define MACRO_CAT0(A, B) A##B
#define MACRO_CAT(A, B) MACRO_CAT0(A, B)

/// [compile time assertion].
/// Error Message: error C2466: cannot allocate an array of constant size 0
/// Usage: COMPILE_TIME_ASSERT(a == b);
/// This is better than C_ASSERT in "winnt.h" because it can append a counter after name
/// which enable multiple COMPILE_TIME_ASSERT() in same scope, where C_ASSERT will fail.
#define COMPILE_TIME_ASSERT(expr) \
    typedef char MACRO_CAT(COMPILE_TIME_ASSERT_ERROR, __COUNTER__)[((expr) ? 1 : 0)]

/// Compare two types, if they are same type
#define IS_SAME_TYPE(T, S) ::xtl::impl::SameType<T, S >::Result

namespace xtl {

    template<class T>
    void swap(T& a, T&b)
    {
        T c = a;
        a = b;
        b = c;
    }

    namespace impl {

        /// class template SameType
        /// Return true iff two given types are the same
        /// Invocation: SameType<T, U>::value
        /// where:
        /// T and U are types
        /// Result evaluates to true iff U == T (types equal)
        template <typename T, typename U>
        struct SameType
        {
        private:
            template<typename> struct Inner     { enum { Result = false }; };
            template<>         struct Inner<T>  { enum { Result = true  }; };

        public:
            enum { Result = Inner<U>::Result };
        };

        /// class template Select
        /// Selects one of two types based upon a boolean constant
        /// Invocation: Select<flag, T, U>::Result
        /// where:
        /// flag is a compile-time boolean constant
        /// T and U are types
        /// Result evaluates to T if flag is true, and to U otherwise.
        template <bool flag, typename T, typename U>
        struct Select
        {
        private:
            template<bool>  struct Inner        { typedef T Result; };
            template<>      struct Inner<false> { typedef U Result; };

        public:
            typedef typename Inner<flag>::Result Result;
        };

        /// a class and macro to now if it's integeral type
        template<class T> struct IsSignedInt_T { enum { Result = 0}; };
        #define IT_IS_SIGNED(T) template<> struct IsSignedInt_T <T> { enum { Result = 1}; }

        IT_IS_SIGNED(signed __int8);        /// __int8,  signed char
        IT_IS_SIGNED(signed __int16);       /// __int16, signed short, short
        IT_IS_SIGNED(signed __int32);       /// __int32, signed int, int
        IT_IS_SIGNED(signed __int64);       /// __int64, signed hyper
        IT_IS_SIGNED(signed long);          /// signed long, long

        template<class T> struct IsUnSignedInt_T { enum { Result = 0}; };
        #define IT_IS_UNSIGNED(T) template<> struct IsUnSignedInt_T <T> { enum { Result = 1}; }

        IT_IS_UNSIGNED(unsigned __int8);    /// unsigned __int8,  unsigned char
        IT_IS_UNSIGNED(unsigned __int16);   /// unsigned __int16, unsigned short
        IT_IS_UNSIGNED(unsigned __int32);   /// unsigned __int32, unsigned int
        IT_IS_UNSIGNED(unsigned __int64);   /// unsigned __int64, unsigned hyper
        IT_IS_UNSIGNED(unsigned long);      /// unsigned long

        #ifdef _CHAR_UNSIGNED
            IT_IS_UNSIGNED(char);
        #else
            IT_IS_SIGNED(char);
        #endif

        #ifdef __BOOL_DEFINED
            IT_IS_UNSIGNED(bool);
        #endif

        #ifdef _NATIVE_WCHAR_T_DEFINED 
            IT_IS_UNSIGNED(wchar_t);
        #endif

        template<class T> struct IsReal_T { enum { Result = 0}; };

        #define IT_IS_REAL(T) template<> struct IsReal_T <T> { enum { Result = 1}; }

        IT_IS_REAL(float);
        IT_IS_REAL(double);
        IT_IS_REAL(long double);

        #undef IT_IS_UNSIGNED
        #undef IT_IS_SIGNED
        #undef IT_IS_REAL

    }   /// namespace impl

    using impl::IsReal_T;
    using impl::IsSignedInt_T;
    using impl::IsUnSignedInt_T;
    using impl::Select;

#define CONST_BOOL(A, B) const static bool A = B

    /// [Type Traits]
    /// NOTE: We do not want to include large boots template lib
    /// NOTE: reference type is always const, cannot be unconst,
    ///       e.g. unconst<int&>::IsConst == true
    /// All implementations are in "xtl_impl.hpp"
    template<class T>
    class TypeTraits
    {
    public:
        /// the plain type, same as T
        typedef T ValueType;

        CONST_BOOL(IsSigned     , IsSignedInt_T<T>::Result);
        CONST_BOOL(IsUnsigned   , IsUnSignedInt_T<T>::Result);
        CONST_BOOL(IsReal       , IsReal_T<T>::Result);
        CONST_BOOL(IsInteger    , IsSigned || IsUnsigned);
        CONST_BOOL(IsArithmetic , IsInteger || IsReal);

        /// can store accumulation, e.g. int for T=char
        typedef typename Select<IsInteger, int, T>::Result AccumType; 

        /// can store the value from sqrt(),
        typedef typename Select<IsInteger, float, T>::Result LengthType;
    };

#undef CONST_BOOL

#pragma push_macro("max")
#pragma push_macro("min")
#undef min
#undef max

    /// wrap std::numeric_limits, to avoid stupid push_macro("max")
    /// Known problem:
    ///  1. does not work for __int64, unsigned __int64
    ///  2. unknown result for non arithmetic type, e.g. PointF
    template<class T>
    class Limits
    {
        typedef ::std::numeric_limits<T> limits_type;
        COMPILE_TIME_ASSERT(TypeTraits<T>::IsArithmetic);

    public:
        /// Usage: if (x <= denorm_min()) { Donot use 1/x;}
        /// remember, denorm_min<int>() == 0;
        static const T denorm_min() { return limits_type::denorm_min();}

        /// Usage: if (x <= epsilon()) { 1 == (1+x);}
        static const T epsilon()    { return limits_type::epsilon();}

        /// Usage: any T x => x < max_value<T>()
        static const T max_value()  { return limits_type::max();}

        /// Usage: any T x => x < max_value<T>()
        static const T min_value()  { return limits_type::min();}

        /// Ininit large number
        static const T infinite() { return limits_type::infinity();}

    };

#pragma pop_macro("min")
#pragma pop_macro("max")

}   /// namespace xtl

#endif//__XTL_UTILITY_HPP__
