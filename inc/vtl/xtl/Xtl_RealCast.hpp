#ifndef __XTL_ROUNDING_HPP__
#define __XTL_ROUNDING_HPP__

/*************************************************************************\
Oliver Liyin Copyright (c) 2003

Module Name:
     Rounding helper function

Abstract:

Notes:
    Support only X86 compatible CPU

Usage:
        /// cast from type S to type T
        /// if S is real and T is integer, use Round()
        /// otherwise, use static_cast<T>(S)

        template<class T, class S> T RealCast(S x);   /// controled by RoundingControl
        template<class T, class S> T ChopCast(S x); /// always ROUND_TO_ZERO
        template<class T, class S> T CeilCast(S x); /// always ROUND_TO_PLUS_INFINITE
        template<class T, class S> T FloorCast(S x);/// always ROUND_TO_MINUS_INFINITE
        template<class T, class S> T RoundCast(S x);/// always ROUND_TO_NEAREST

        /// Round floating point number to integer type
        /// Using X87 instruction fistp, much faster than _ftol
        int Round(float);
        int Round(double);

        /// use RoundingControl<> to control behavier of a group of RealCast
        class RoundingControl<RoundingMode ROUNDING_CONTROL_WORD>

History:
    Created  on 2004 June 6 by oliver_liyin

\*************************************************************************/

#ifndef _M_IX86
    #error VTL rounding supports only X86 compatible CPU
#endif

#include "Xtl_Utility.hpp"

namespace xtl
{
    /// const mode and masks for rounding control in x87 FPU
    enum RoundingMode
    {
        ROUND_TO_NEAREST            = 0x000, /// _RC_NEAR << 2,
        ROUND_TO_MINUS_INFINITE     = 0x400, /// _RC_DOWN << 2,
        ROUND_TO_PLUS_INFINITE      = 0x800, /// _RC_UP   << 2,
        ROUND_TO_ZERO               = 0xC00, /// _RC_CHOP << 2,

        /// the mask for control word in x87
        ROUND_CONTROL_WORD_MASK     = 0xC00, /// _MCW_RC  << 2,

        /// default rounding control, do not change x87 control word
        ROUNDING_MODE_DEFAULT       = 0xfff,
    };

#pragma region Implementation of fast cast
    namespace impl
    {
        /// Safe static cast, for short integer types
        ///  If s is out of range to type T, a run time error is raised in DEBUG mode
        ///  safe_static_cast suppress the run time error by MASK
        template<class T, class S>
        T safe_static_cast(S s)
        {
            return static_cast<T>(s);
        }

        /// can be optimized as __asm movszx eax, al
#define SAFE_STATIC_CAST(TYPE, TYPE_II, MASK)                           \
        template<>                                                      \
        inline TYPE safe_static_cast<TYPE, TYPE_II>(TYPE_II s)          \
        {                                                               \
            return static_cast<TYPE>(s & MASK);                         \
        }                                                               \

        SAFE_STATIC_CAST(CHAR,   LONG,   0xff);
        SAFE_STATIC_CAST(UCHAR,  ULONG,  0xff);
        SAFE_STATIC_CAST(SHORT,  LONG,   0xffff);
        SAFE_STATIC_CAST(USHORT, ULONG,  0xffff);

#undef SAFE_STATIC_CAST

        /// -----------------------------------------------------------------------
        /// implement fast floating point cast to integer
        /// -----------------------------------------------------------------------


        /// If fUseRounding is false, simply return static cast
        template<class T, bool fUseRounding, RoundingMode RCW>
        struct fp_cast_impl
        {
            template<class S> static T cast(const S& x)
            {
                COMPILE_TIME_ASSERT(!xtl::TypeTraits<T>::IsInteger ||
                                    !xtl::TypeTraits<S>::IsReal);
                return safe_static_cast<T, S>(x);
            }
        };

        /// with no Rounding control, using outside control
        template<class T>
        struct fp_cast_impl<T, true, ROUNDING_MODE_DEFAULT>
        {
            template<class S> static T cast(const S& x)
            {
                COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsInteger &&
                                    xtl::TypeTraits<S>::IsReal);
                return safe_static_cast<T, int>(Round(x));
            }
        };

        /// use Round function only when T is integer and S is floating point
        template<class T, RoundingMode RCW>
        struct fp_cast_impl<T, true, RCW>
        {
            template<class S> static T cast(const S& x)
            {
                COMPILE_TIME_ASSERT(xtl::TypeTraits<T>::IsInteger && 
                                    xtl::TypeTraits<S>::IsReal);
                COMPILE_TIME_ASSERT((RCW & (~ROUND_CONTROL_WORD_MASK)) == 0);

                /// inline assembly instead of class RoundingControl
                /// for better performance
                WORD oldcw, newcw = RCW;
                _asm
                {
                    fnstcw  oldcw                   /// load x87 control word in oldcw
                    mov     ax,     oldcw
                    and     ax,     ~ROUND_CONTROL_WORD_MASK
                    or      newcw,  ax              /// change to new rounding mode
                    fldcw   newcw                   /// set to new x87 control word
                }

                /// round x according to current rounding control word
                int result = Round(x);

                /// restore the backup control word
                __asm fldcw oldcw

                /// cast result from int to T
                return safe_static_cast<T, int>(result);
            }
        };

    } /// namespace impl
#pragma endregion

    /// ---------------------------------------------------------------------------
    /// RealCast and other floating point cast functions
    /// including:
    ///  1. ChopCast<T>(S x) cast x toward zero
    ///  2. RoundCast<T>(S x) cast x toward nearest integer
    ///  3. CeilCast<T>(S x) cast x toward plus infinite
    ///  4. FloorCast<T>(S x) cast x toward minus infinite
    ///  5. RealCast<T>(S x) cast x in fastest way, without changing rounding mode
    /// ---------------------------------------------------------------------------

    template<class T, class S>
    T RealCast(const S& x)
    {
        const bool TisInt = xtl::TypeTraits<T>::IsInteger;
        const bool SisFlt = xtl::TypeTraits<S>::IsReal;
        return impl::fp_cast_impl<T, TisInt && SisFlt, ROUNDING_MODE_DEFAULT>::cast(x);
    }

    template<class T, class S>
    T ChopCast(const S& x)
    {
        const bool TisInt = xtl::TypeTraits<T>::IsInteger;
        const bool SisFlt = xtl::TypeTraits<S>::IsReal;
        return impl::fp_cast_impl<T, TisInt && SisFlt, ROUND_TO_ZERO>::cast(x);
    }

    template<class T, class S>
    T RoundCast(const S& x)
    {
        const bool TisInt = xtl::TypeTraits<T>::IsInteger;
        const bool SisFlt = xtl::TypeTraits<S>::IsReal;
        return impl::fp_cast_impl<T, TisInt && SisFlt, ROUND_TO_NEAREST>::cast(x);
    }

    template<class T, class S>
    T CeilCast(const S& x)
    {
        const bool TisInt = xtl::TypeTraits<T>::IsInteger;
        const bool SisFlt = xtl::TypeTraits<S>::IsReal;
        return impl::fp_cast_impl<T, TisInt && SisFlt, ROUND_TO_PLUS_INFINITE>::cast(x);
    }

    template<class T, class S>
    T FloorCast(const S& x)
    {
        const bool TisInt = xtl::TypeTraits<T>::IsInteger;
        const bool SisFlt = xtl::TypeTraits<S>::IsReal;
        return impl::fp_cast_impl<T, TisInt && SisFlt, ROUND_TO_MINUS_INFINITE>::cast(x);
    }

    /// round float to int, which is faster than ftol
    /// Pentium's fistp instruction uses round towards even. 
    inline const int Round(const float& x)
    {
        int i;
        __asm {
            mov eax, x
            fld dword ptr [eax]
            fistp [i]
        }
        return i;
    }

    /// round double to int, which is faster than ftol
    /// Pentium's fistp instruction uses round towards even. 
    inline const int Round(const double& x)
    {
        int i;
        __asm {
            mov eax, x
            fld qword ptr [eax]
            fistp [i]
        }
        return i;
    }

    /// Control the round mode of "fistp"
    /// mode can be
    ///      ROUND_TO_NEAREST,           /// to nearest even int
    ///      ROUND_TO_MINUS_INFINITE,    /// floor
    ///      ROUND_TO_PLUS_INFINITE,     /// ceil
    ///      ROUND_TO_ZERO               /// chop decimal part
    //
    /// Usage:
    ///  void func() {
    ///      RoundingControl<ROUND_TO_ZERO> roundingmode;
    ///      for(;;) { i = Round(fp);}   /// using chop mode
    ///      RoundingControl<ROUND_TO_MINUS_INFINITE> roundingmode2;
    ///      for(;;) { i = Round(fp);}   /// using down mode
    ///  } /// rounding control will be reset automatically out of this scope
    template<RoundingMode ROUNDING_CONTROL_WORD = ROUND_TO_NEAREST>
    class RoundingControl
    {
    public:
        RoundingControl()
        {
            Set();
        }

        ~RoundingControl()
        {
            Reset();
        }

        void Set()
        {
            COMPILE_TIME_ASSERT(
                ROUNDING_CONTROL_WORD == ROUND_TO_NEAREST ||
                ROUNDING_CONTROL_WORD == ROUND_TO_MINUS_INFINITE ||
                ROUNDING_CONTROL_WORD == ROUND_TO_PLUS_INFINITE   ||
                ROUNDING_CONTROL_WORD == ROUND_TO_ZERO );

            //m_SavedStatus = _statusfp();
            //_controlfp(ROUNDING_CONTROL, _MCW_RC);
            WORD oldcw, newcw = ROUNDING_CONTROL_WORD;
            _asm {
                fnstcw  oldcw                   /// load x87 control word in oldcw
                mov     ax,     oldcw
                and     ax,     ~ROUND_CONTROL_WORD_MASK
                or      newcw,  ax              /// change to new rounding mode
                fldcw   newcw                   /// set to new x87 control word
            }
            m_SavedStatus = oldcw;
        }

        void Reset()
        {
            //_controlfp(m_SavedStatus, _MCW_RC);
            WORD oldcw = m_SavedStatus;
            __asm    fldcw   oldcw
        }

    private:
        WORD m_SavedStatus;
    };

}   /// namespace xtl


#endif//__XTL_ROUNDING_HPP__
