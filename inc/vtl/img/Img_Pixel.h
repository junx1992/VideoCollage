/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib: pixels defination, and implementation
  
Abstract:
    Defines various pixel types, including BYTE, FLOAT, RGBA, RGB, etc.

Notes:
     TODO: use derive instead of typedef

     #define DEFINE_PIXEL_RGB(PixelName, ChannelName, AccessName)
     class PixelName : public vtl::Vector<ChannelName, AccessName>
     {
     };


Usage:
        
History:
    Created  on 2003 Aug. 16 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "Img_Utility.h"
#include <windows.h>

namespace img
{
    /// Simple pixel type, from basic types
    typedef BYTE     PixelByte;        /// unsigned __int8
    typedef WORD     PixelWord;        /// unsigned __int16
    typedef short    PixelShort;       /// signed __int16
    typedef int      PixelInt;         /// signed __int32
    typedef float    PixelFloat;
    typedef double   PixelDouble;

    /// PixelTraits<T>
    /// 1. PixelSize         /// bytes per pixel
    /// 2. PixelType         /// the type of pixel
    /// 3. ChannelSize       /// bytes per channel
    /// 4. ChannelType       /// the type of channel item
    /// 5. ChannelNum        /// the number of channels, include alpha
    /// 6. ColorChannelNum   /// the number of color channels, exclude alpha
    /// 7. HasAlpha          /// the pixel has alpha channel or not
    template<class T>
    class PixelTraits
    {
    public:
        typedef T PixelType;
        typedef T ChannelType;
        typedef typename xtl::TypeTraits<ChannelType>::LengthType DiffType;

        const static size_t PixelSize = sizeof(T);
        const static size_t ChannelSize = sizeof(T);
        const static size_t ChannelNum = 1;
        const static size_t ColorChannelNum = 1;    /// exclude alpha channel
        const static bool HasAlpha = false;

        const static PixelType White, Black;

        /// Pixel invert, the negative value of pixel
        /// Invert(Invert(p)) == p; is always true
        /// Should be dst = White - src + Black; usually black = 0
        static void Invert(T& dst, const T& src)
        {
            dst = White - src;
        }

        /// Pixel difference between two pixels
        /// return value contains the LengthType of ChannelType
        /// which can keep the value of Length() function
        static DiffType Diff(const T& p, const T& q)
        {
            return xtl::RealCast<DiffType>(p - q);
        }
    };

#pragma region Template version of RGB colors

    #pragma pack(push)
    #pragma pack(1)

    template<class T, bool hasAlpha> class PixelRgb_T;

    template<class T>
    class PixelRgb_T<T, false>
    {
    public:
        T b, g, r;

        PixelRgb_T()
        {
            COMPILE_TIME_ASSERT(sizeof(*this) == 3 * sizeof(T));
        }

        PixelRgb_T(T _r, T _g, T _b)
            : b(_b), g(_g), r(_r)
        {
        }

        T& operator[] (size_t idx)
        {
            assert(idx < 3);
            return (&b)[idx];
        }

        const T& operator[] (size_t idx) const
        {
            assert(idx < 3);
            return (&b)[idx];
        }

        void Set(T _r, T _g, T _b)
        {
            r = _r; g = _g; b = _b;
        }

        T R() const { return r;}
        T G() const { return g;}
        T B() const { return b;}
        void SetR(T _r) { r = _r;}
        void SetG(T _g) { g = _g;}
        void SetB(T _b) { b = _b;}
    };

    /// Access pixel using bgra structure, each element is type T
    template<class T>
    class PixelRgb_T<T, true>
    {
    public:
        T b, g, r, a;

        PixelRgb_T()
        {
            COMPILE_TIME_ASSERT(sizeof(*this) == 4 * sizeof(T));
        }

        PixelRgb_T(T _r, T _g, T _b, T _a)
            : b(_b), g(_g), r(_r), a(_a)
        {
        }

        T& operator[] (size_t idx)
        {
            assert(idx < 4);
            return (&b)[idx];
        }

        const T& operator[] (size_t idx) const
        {
            assert(idx < 4);
            return (&b)[idx];
        }

        void Set(T _r, T _g, T _b, T _a = 255)
        {
            r = _r; g = _g; b = _b; a = _a;
        }

        T R() const { return r;}
        T G() const { return g;}
        T B() const { return b;}
        T A() const { return a;}
        void SetR(T _r) { r = _r;}
        void SetG(T _g) { g = _g;}
        void SetB(T _b) { b = _b;}
        void SetA(T _a) { a = _a;}
    };

    template<>
    class PixelRgb_T<PixelByte, true>
    {
    public:
        PixelByte b, g, r, a;

        PixelRgb_T()
        {
            COMPILE_TIME_ASSERT(sizeof(*this) == 4 * sizeof(PixelByte));
        }

        PixelRgb_T(const PixelRgb_T& other)
        {
            Set(other.Value());
        }

        PixelRgb_T& operator = (const PixelRgb_T& other)
        {
            Set(other.Value());
            return *this;
        }

        PixelRgb_T(PixelByte _r, PixelByte _g, PixelByte _b, PixelByte _a = 0xff)
            : b(_b), g(_g), r(_r), a(_a)
        {
        }

        PixelRgb_T(DWORD dwValue)
        {
            Set(dwValue);
        }

        PixelByte& operator[] (size_t idx)
        {
            assert(idx < 4);
            return (&b)[idx];
        }

        const PixelByte& operator[] (size_t idx) const
        {
            assert(idx < 4);
            return (&b)[idx];
        }

        void Set(PixelByte _r, PixelByte _g, PixelByte _b, PixelByte _a = 0xff)
        {
            r = _r; g = _g; b = _b; a = _a;
        }

        void Set(DWORD dwValue)
        {
            *(DWORD*)(void*)(&b) = dwValue;
        }

        const DWORD& Value() const
        {
            return *(const DWORD*)(const void*)(&b);
        }

        PixelByte R() const { return r;}
        PixelByte G() const { return g;}
        PixelByte B() const { return b;}
        PixelByte A() const { return a;}
        void SetR(PixelByte _r) { r = _r;}
        void SetG(PixelByte _g) { g = _g;}
        void SetB(PixelByte _b) { b = _b;}
        void SetA(PixelByte _a) { a = _a;}
    };


    #pragma pack(pop)

#pragma endregion

    template<class T, bool hasAlpha>
    class PixelTraits<PixelRgb_T<T, hasAlpha> >
    {
    public:
        typedef PixelRgb_T<T, hasAlpha> PixelType;

        typedef T ChannelType;

        const static size_t PixelSize = sizeof(PixelType);

        const static size_t ChannelSize = sizeof(T);

        const static size_t ChannelNum = PixelSize / ChannelSize;

        const static size_t ColorChannelNum = 3;

        const static bool HasAlpha = (ChannelNum > ColorChannelNum);

        typedef typename xtl::TypeTraits<T>::LengthType DiffType;

        const static PixelType White, Black;

        static void Invert(PixelType& dst, const PixelType& src)
        {
            dst.r = White.r - src.r;
            dst.g = White.g - src.g;
            dst.b = White.b - src.b;
        }

        static DiffType Diff(const PixelType& p, const PixelType& q)
        {
            DiffType dr = (DiffType) p.r - (DiffType) q.r;
            DiffType dg = (DiffType) p.g - (DiffType) q.g;
            DiffType db = (DiffType) p.b - (DiffType) q.b;
            return (DiffType) sqrt(dr*dr + dg*dg + db*db);
        }

        COMPILE_TIME_ASSERT(PixelSize == ChannelNum * ChannelSize);
    };

    typedef PixelRgb_T<PixelByte,  false>  PixelRgbByte;
    typedef PixelRgb_T<PixelWord,  false>  PixelRgbWord;
    typedef PixelRgb_T<PixelShort, false>  PixelRgbShort;
    typedef PixelRgb_T<PixelInt,   false>  PixelRgbInt;
    typedef PixelRgb_T<PixelFloat, false>  PixelRgbFloat;
    typedef PixelRgb_T<PixelDouble,false>  PixelRgbDouble;

    typedef PixelRgb_T<PixelByte,  true>  PixelArgbByte;
    typedef PixelRgb_T<PixelWord,  true>  PixelArgbWord;
    typedef PixelRgb_T<PixelShort, true>  PixelArgbShort;
    typedef PixelRgb_T<PixelInt,   true>  PixelArgbInt;
    typedef PixelRgb_T<PixelFloat, true>  PixelArgbFloat;
    typedef PixelRgb_T<PixelDouble,true>  PixelArgbDouble;

    /// BYTE is the default name of both Argb and Rgb
    typedef PixelArgbByte PixelArgb;
    typedef PixelRgbByte  PixelRgb;

    /// Define arithmatic operators for PixelRgb_T
    /// They are applied on each channel, including alpha if it exists
#define RGB_OP_ARITHMATIC(OP)                                                   \
    template<class T, bool A>                                                   \
    void operator OP##= (PixelRgb_T<T, A>& dst, const PixelRgb_T<T, A>& src)    \
    {                                                                           \
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;            \
        for (int k = 0; k < K; k ++) dst[k] OP##= src[k];                       \
    }                                                                           \
    template<class T, bool A>                                                   \
    PixelRgb_T<T, A> operator OP (PixelRgb_T<T, A> a, const PixelRgb_T<T, A>& b)\
    {                                                                           \
        a OP##= b;                                                              \
        return a;                                                               \
    }                                                                           \
    template<class T, bool A>                                                   \
    void operator OP##= (PixelRgb_T<T, A>& dst, T scalar)                       \
    {                                                                           \
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;            \
        for (int k = 0; k < K; k ++) dst[k] OP##= scalar;                       \
    }                                                                           \
    template<class T, bool A>                                                   \
    PixelRgb_T<T, A> operator OP (PixelRgb_T<T, A> a, T scalar)                 \
    {                                                                           \
        a OP##= scalar;                                                         \
        return a;                                                               \
    }                                                                           \
    template<class T, bool A>                                                   \
        PixelRgb_T<T, A> operator OP (T scalar, const PixelRgb_T<T, A>& b)      \
    {                                                                           \
        PixelRgb_T<T, A> r;                                                     \
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;            \
        for (int k = 0; k < K; k ++) r[k] = scalar OP b[k];                     \
		return r;																\
    }                                                                           \

    RGB_OP_ARITHMATIC(+);
    RGB_OP_ARITHMATIC(-);
    RGB_OP_ARITHMATIC(*);
    RGB_OP_ARITHMATIC(/);
    RGB_OP_ARITHMATIC(&);
    RGB_OP_ARITHMATIC(|);
    RGB_OP_ARITHMATIC(^);
    RGB_OP_ARITHMATIC(>>);
    RGB_OP_ARITHMATIC(<<);

#undef RGB_OP_ARITHMATIC

    /// Define comparison, so that pixel can be put into Associative Containers
    /// such as map<>, set<>
#define RGB_OP_ALPHABETA_COMPARE(OP, OPS, EQUAL)                                \
    template<class T, bool A>                                                   \
    bool operator OP (const PixelRgb_T<T, A>& a, const PixelRgb_T<T, A>& b)     \
    {                                                                           \
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;            \
        for (size_t k = 0; k < K; k ++)                                         \
        {                                                                       \
            if (a[k] OPS b[k])    return true;                                  \
            if (b[k] OPS a[k])    return false;                                 \
        }                                                                       \
        return EQUAL;                                                           \
    }                                                                           \

    RGB_OP_ALPHABETA_COMPARE(<=, <, true);
    RGB_OP_ALPHABETA_COMPARE(>=, >, true);
    RGB_OP_ALPHABETA_COMPARE(< , <, false);
    RGB_OP_ALPHABETA_COMPARE(> , >, false);

#undef RGB_OP_ALPHABETA_COMPARE

    template<class T, bool A>
    bool operator == (const PixelRgb_T<T, A>& a, const PixelRgb_T<T, A>& b)
    {
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;
        for (size_t k = 0; k < K; k ++)
        {
            if (a[k] != b[k]) return false;
        }
        return true;
    }

    template<class T, bool A>
    bool operator != (const PixelRgb_T<T, A>& a, const PixelRgb_T<T, A>& b)
    {
        const size_t K = PixelTraits<PixelRgb_T<T, A> >::ChannelNum;
        for (size_t k = 0; k < K; k ++)
        {
            if (a[k] == b[k]) return false;
        }
        return true;
    }

    template<>
    inline bool operator == (const PixelArgb& a, const PixelArgb& b)
    {
        return a.Value() == b.Value();
    }

    template<>
    inline bool operator != (const PixelArgb& a, const PixelArgb& b)
    {
        return a.Value() != b.Value();
    }

    /// Pixel for HSV color
#pragma pack(push)
#pragma pack(1)
    struct PixelHsv
    {
    public:
        PixelByte h, s, v;

        PixelHsv() {}

        PixelHsv(PixelByte _h, PixelByte _s, PixelByte _v)
        {
            h = _h; s = _s; v = _v;
        }

        void Set(PixelByte _h, PixelByte _s, PixelByte _v)
        {
            h = _h; s = _s; v = _v;
        }
    };
#pragma pack(pop)

    /// Pixel Traits for general Hsv colors
    template<>
    class PixelTraits<PixelHsv>
    {
    public:
        const static size_t PixelSize = sizeof(PixelHsv);

        typedef PixelHsv PixelType;
        typedef PixelByte ChannelType;
        typedef xtl::TypeTraits<ChannelType>::LengthType difference;

        const static size_t ChannelSize = sizeof(PixelByte);
        const static size_t ChannelNum = 3;
        const static size_t ColorChannelNum = 3;    /// exclude alpha channel
        const static bool HasAlpha = false;

        COMPILE_TIME_ASSERT(PixelSize == ChannelNum * ChannelSize);
    };


    /// Pixel for CIE La*b* color space
    /// This space is uniform for human eyes.
    /// Therefore, we can use simple Eucledian distance
    /// Pixel range:  L  \in [0, 100]    * 256
    ///               a* \in [-128, 127] * 256
    ///               b* \in [-127, 127] * 256
    /// The higher byte is for CIE Lab color, integer part
    /// The lower byte is decimal part of the color
#pragma pack(push)
#pragma pack(1)
    struct PixelLab
    {
    public:
        short l, a, b;

        PixelLab() {}

        PixelLab(short _l, short _a, short _b)
        {
            l = _l; a = _a; b = _b;
        }

        void Set(short _l, short _a, short _b)
        {
            l = _l; a = _a; b = _b;
        }
    };
#pragma pack(pop)

    /// Pixel Traits for general Lab colors
    template<>
    class PixelTraits<PixelLab>
    {
    public:
        const static size_t PixelSize = sizeof(PixelLab);

        typedef PixelLab PixelType;
        typedef short ChannelType;
        typedef xtl::TypeTraits<ChannelType>::LengthType DiffType;

        const static size_t ChannelSize = sizeof(short);
        const static size_t ChannelNum = 3;
        const static size_t ColorChannelNum = 3;    /// exclude alpha channel
        const static bool HasAlpha = false;

        COMPILE_TIME_ASSERT(PixelSize == ChannelNum * ChannelSize);


        /// Pixel invert, the negative value of pixel
        /// Invert(Invert(p)) == p; is always true
        static void Invert(PixelType& dst, const PixelType& src)
        {
            dst.l = (100 << 8) - src.l;
            dst.a = - src.a;
            dst.b = - src.b;
        }

        /// Pixel difference between two pixels
        /// return value contains the LengthType of ChannelType
        /// which can keep the value of Length() function
        static DiffType Difference(const PixelType& p, const PixelType& q)
        {
            const DiffType dl = xtl::RealCast<DiffType>(p.l) - xtl::RealCast<DiffType>(q.l);
            const DiffType da = xtl::RealCast<DiffType>(p.a) - xtl::RealCast<DiffType>(q.a);
            const DiffType db = xtl::RealCast<DiffType>(p.b) - xtl::RealCast<DiffType>(q.b);
            return sqrt(dl * dl + da * da + db * db);
        }
    };

#pragma region -- Implementation of PixelAssignment

    namespace impl
    {
        /// Pixel assignment, from dst type to src type
        /// Generally, use RealCast to cast the pixel type
        template<class DST, class SRC>
        struct PixelAssignment
        {
            static void Assign(DST& dst, const SRC& src)
            {
                dst = xtl::RealCast<DST>(src);
            }
        };

        /// Pixel assignment between pixels of same type
        template<class SameType>
        struct PixelAssignment<SameType, SameType>
        {
            static void Assign(SameType& dst, const SameType& src)
            {
                dst = src;
            }
        };

        /// Translate pixel type between 8bits and 16bits
        template<>
        struct PixelAssignment<PixelByte, PixelWord>
        {
            static void Assign(PixelByte& dst, const PixelWord& src)
            {
                dst = xtl::RealCast<PixelByte>(src >> 6);
            }
        };

        template<>
        struct PixelAssignment<PixelWord, PixelByte>
        {
            static void Assign(PixelWord& dst, const PixelByte& src)
            {
                dst = xtl::RealCast<PixelWord>(src << 6);
            }
        };

        /// ------- Pixel assignment between rgb and gray level

        template<class GRAY, class T, bool hasAlpha>
        struct PixelAssignment<GRAY, PixelRgb_T<T, hasAlpha> >
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;

            static void Assign(GRAY& dst, const RgbType& src)
            {
                typedef PixelTraits<RgbType>::ChannelType ChannelType;
                typedef xtl::TypeTraits<ChannelType>::AccumType Type;
                Rgb2Gray<Type, xtl::TypeTraits<Type>::IsInteger> converter;
                dst = xtl::RealCast<GRAY>(converter(src.r, src.g, src.b));
            }

        private:
            /// integer version: (14549 * R + 46334 * G + 4653 * B) >> 16
            template<class Type, bool TypeIsInteger> struct Rgb2Gray;

            /// ITU standard.    .222 * R + .707 * G + .071 * B
            template<class Type>
            struct Rgb2Gray<Type, false>
            {
                Type operator () (Type r, Type g, Type b)
                {
                    return r * Type(0.222) + g * Type(0.707) + b * Type(0.071);
                }
            };

            template<class Type>
            struct Rgb2Gray<Type, true>
            {
                Type operator () (Type r, Type g, Type b)
                {
                    return (r * 14549 + g * 46334 + b * 4653) >> 16;
                }
            };
        };

        template<class GRAY, class T, bool hasAlpha>
        struct PixelAssignment<PixelRgb_T<T, hasAlpha>, GRAY>
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;
            static void Assign(RgbType& dst, const GRAY& src)
            {
                typedef PixelTraits<RgbType>::ChannelType ChannelType;
                ChannelType tmp = xtl::RealCast<ChannelType>(src);
                dst.Set(tmp, tmp, tmp);
            }
        };

        /// ------- Pixel assignment between rgb and hsv

        void ConvertRgb2Hsv(BYTE& h, BYTE& s, BYTE& v, BYTE r, BYTE g, BYTE b);
        void ConvertHsv2Rgb(BYTE& r, BYTE& g, BYTE& b, BYTE h, BYTE s, BYTE v);

        template<class T, bool hasAlpha>
        struct PixelAssignment<PixelHsv, PixelRgb_T<T, hasAlpha> >
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;
            static void Assign(PixelHsv& dst, const RgbType& src)
            {
                BYTE r = xtl::RealCast<BYTE>(src.r);
                BYTE g = xtl::RealCast<BYTE>(src.g);
                BYTE b = xtl::RealCast<BYTE>(src.b);
                ConvertRgb2Hsv(dst.h, dst.s, dst.v, r, g, b);
            };
        };

        template<class T, bool hasAlpha>
        struct PixelAssignment<PixelRgb_T<T, hasAlpha>, PixelHsv>
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;
            static void Assign(RgbType& dst, const PixelHsv& src)
            {
                BYTE b, g, r;
                ConvertHsv2Rgb(b, g, r, src.h, src.s, src.v);
                dst.r = xtl::RealCast<BYTE>(r);
                dst.g = xtl::RealCast<BYTE>(g);
                dst.b = xtl::RealCast<BYTE>(b);
            };
        };

        /// ------- Pixel assignment between rgb and lab

        void ConvertRgb2Lab(PixelLab& dst, BYTE r, BYTE g, BYTE b);
        void ConvertLab2Rgb(BYTE& r, BYTE& g, BYTE& b, const PixelLab& src);

        template<class T, bool hasAlpha>
        struct PixelAssignment<PixelLab, PixelRgb_T<T, hasAlpha> >
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;
            static void Assign(PixelLab& dst, const RgbType& src)
            {
                BYTE r = xtl::RealCast<BYTE>(src.r);
                BYTE g = xtl::RealCast<BYTE>(src.g);
                BYTE b = xtl::RealCast<BYTE>(src.b);
                ConvertRgb2Lab(dst, r, g, b);
            };
        };

        template<class T, bool hasAlpha>
        struct PixelAssignment<PixelRgb_T<T, hasAlpha>, PixelLab>
        {
            typedef PixelRgb_T<T, hasAlpha> RgbType;
            static void Assign(RgbType& dst, const PixelLab& src)
            {
                BYTE b, g, r;
                ConvertLab2Rgb(b, g, r, src);
                dst.r = xtl::RealCast<BYTE>(r);
                dst.g = xtl::RealCast<BYTE>(g);
                dst.b = xtl::RealCast<BYTE>(b);
            };
        };

        /// ----- Cross type pixel assignment for RGB or ARGB colors
        template<class T, bool ThasAlpha, class S, bool ShasAlpha>
        struct PixelAssignment<PixelRgb_T<T, ThasAlpha>, PixelRgb_T<S, ShasAlpha> >
        {
            typedef PixelRgb_T<T, ThasAlpha> PixelDst;
            typedef PixelRgb_T<S, ShasAlpha> PixelSrc;
            typedef PixelTraits<PixelDst> TraitsDst;
            typedef PixelTraits<PixelSrc> TraitsSrc;
            typedef typename TraitsDst::ChannelType ChannelDst;
            typedef typename TraitsSrc::ChannelType ChannelSrc;
            const static size_t copyNum = min(TraitsDst::ChannelNum, TraitsSrc::ChannelNum);

            static void Assign(PixelDst& dst, const PixelSrc& src)
            {
                for (size_t k = 0; k < copyNum; k ++)
                {
                    PixelAssignment<ChannelDst, ChannelSrc>::Assign(dst[k], src[k]);
                }
            }
        };

    } /// namespace impl

#pragma endregion

    /// Index of channels when using pixel as array of channels
    enum ChannelIndex
    {
        ChannelBlue     = 0,
        ChannelGreen    = 1,
        ChannelRed      = 2,
        ChannelAlpha    = 3,

        ChannelLumi     = 0,
        ChannelAstar    = 1,
        ChannelBstar    = 2,

        ChannelHue      = 0,
        ChannelSaturate = 1,
        ChannelValue    = 2,

        ChannelGray     = 0,
    };

    /// Pixel assignment between two types, function version
    template<class DST, class SRC>
    void PixelAssign(DST& dst, const SRC& src)
    {
        impl::PixelAssignment<DST, SRC>::Assign(dst, src);
    }

    template<class PixelType>
    void PixelInvert(PixelType& dst, const PixelType& src)
    {
        PixelTraits<PixelType>::Invert(dst, src);
    }

    template<>
    inline void PixelInvert<PixelArgb>(PixelArgb& dst, const PixelArgb& src)
    {
        (*(DWORD*)(void*)&dst) = 0x00ffffff ^ (*(DWORD*)(void*)&src);
    }

    template<>
    inline void PixelInvert<PixelByte>(PixelByte& dst, const PixelByte& src)
    {
        (*(BYTE*)(void*)&dst) = 0xff ^ (*(BYTE*)(void*)&src);
    }

    template<class PixelType>
    typename PixelTraits<PixelType>::DiffType
        PixelDifference(const PixelType& p, const PixelType& q)
    {
        return PixelTraits<PixelType>::Difference(p, q);
    }

} /// namespace img
