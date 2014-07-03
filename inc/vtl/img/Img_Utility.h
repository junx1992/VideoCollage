/*************************************************************************\
    Microsoft Research Asia
    Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib: basic utility and helper functions
  
Notes:

    The Gdiplus namespace is used in this header.
    However, all functions can be replaced without requiring Gdiplus.DLL
    For those requiring Gdiplus.DLL, look at Img_Gdiplus.h
     
Usage:
        
History:
    Created  on 2003 Aug. 16 by oliver_liyin
                  
\*************************************************************************/

#pragma once

#include "xtl/Xtl_Utility.hpp"
#include "xtl/Xtl_Realcast.hpp"

#include <unknwn.h>
#include <gdiplus.h>


#pragma region -- Additional operators for Gdiplus::Size, Point, and PointF

namespace Gdiplus
{

#define TO_INT_64(x) (*(__int64*)(void*)(&x))

    COMPILE_TIME_ASSERT(sizeof(__int64) == 2 * sizeof(INT));
    COMPILE_TIME_ASSERT(sizeof(__int64) == 2 * sizeof(REAL));

#define POINT_OP_COMPARISON(PointType, OP)                                      \
    inline bool operator OP (const PointType& a, const PointType& b)            \
    {                                                                           \
        return TO_INT_64(a) OP TO_INT_64(b);                                    \
    }

    POINT_OP_COMPARISON(PointF, ==);
    POINT_OP_COMPARISON(PointF, !=);
    POINT_OP_COMPARISON(PointF, < );
    POINT_OP_COMPARISON(PointF, > );
    POINT_OP_COMPARISON(PointF, <=);
    POINT_OP_COMPARISON(PointF, >=);

    POINT_OP_COMPARISON(Point, ==);
    POINT_OP_COMPARISON(Point, !=);
    POINT_OP_COMPARISON(Point, < );
    POINT_OP_COMPARISON(Point, > );
    POINT_OP_COMPARISON(Point, <=);
    POINT_OP_COMPARISON(Point, >=);

    POINT_OP_COMPARISON(SizeF, ==);
    POINT_OP_COMPARISON(SizeF, !=);
    POINT_OP_COMPARISON(SizeF, < );
    POINT_OP_COMPARISON(SizeF, > );
    POINT_OP_COMPARISON(SizeF, <=);
    POINT_OP_COMPARISON(SizeF, >=);

    POINT_OP_COMPARISON(Size, ==);
    POINT_OP_COMPARISON(Size, !=);
    POINT_OP_COMPARISON(Size, < );
    POINT_OP_COMPARISON(Size, > );
    POINT_OP_COMPARISON(Size, <=);
    POINT_OP_COMPARISON(Size, >=);

#undef POINT_OP_COMPARISON
#undef TO_INT_64

#define POINT_OP_POINT(PointType, OP)                                           \
    inline PointType operator OP (const PointType& lhs, const PointType& rhs)   \
    {                                                                           \
        return PointType(lhs.X OP rhs.X, lhs.Y OP rhs.Y);                       \
    }

    POINT_OP_POINT(PointF, *);
    POINT_OP_POINT(PointF, /);

    POINT_OP_POINT(Point, *);
    POINT_OP_POINT(Point, /);
    POINT_OP_POINT(Point, |);
    POINT_OP_POINT(Point, &);
    POINT_OP_POINT(Point, ^);
    POINT_OP_POINT(Point, >>);
    POINT_OP_POINT(Point, <<);

#undef POINT_OP_POINT

#define POINT_OP_EQ_POINT(PointType, OP)                                        \
    inline void operator OP##= (PointType& lhs, const PointType& rhs)           \
    {                                                                           \
        lhs.X OP##= rhs.X;                                                      \
        lhs.Y OP##= rhs.Y;                                                      \
    }

    POINT_OP_EQ_POINT(PointF, +);
    POINT_OP_EQ_POINT(PointF, -);
    POINT_OP_EQ_POINT(PointF, *);
    POINT_OP_EQ_POINT(PointF, /);

    POINT_OP_EQ_POINT(Point, +);
    POINT_OP_EQ_POINT(Point, -);
    POINT_OP_EQ_POINT(Point, *);
    POINT_OP_EQ_POINT(Point, /);
    POINT_OP_EQ_POINT(Point, |);
    POINT_OP_EQ_POINT(Point, &);
    POINT_OP_EQ_POINT(Point, ^);
    POINT_OP_EQ_POINT(Point, >>);
    POINT_OP_EQ_POINT(Point, <<);

#undef POINT_OP_EQ_POINT

#define POINT_OP_SCALE(PointType, OP, ScaleType)                            \
    inline void operator OP##= (PointType& pt, ScaleType scale)             \
    {                                                                       \
        pt.X OP##= scale;                                                   \
        pt.Y OP##= scale;                                                   \
    }                                                                       \
    inline PointType operator OP (const PointType& pt, ScaleType scale)     \
    {                                                                       \
        return PointType(pt.X OP scale, pt.Y OP scale);                     \
    }                                                                       \
    inline PointType operator OP (ScaleType scale, const PointType& pt)     \
    {                                                                       \
        return PointType(scale OP pt.X, scale OP pt.Y);                     \
    }                                                                       \

    POINT_OP_SCALE(PointF, +, float);
    POINT_OP_SCALE(PointF, -, float);
    POINT_OP_SCALE(PointF, *, float);
    POINT_OP_SCALE(PointF, /, float);

    POINT_OP_SCALE(Point, +, int);
    POINT_OP_SCALE(Point, -, int);
    POINT_OP_SCALE(Point, *, int);
    POINT_OP_SCALE(Point, /, int);
    POINT_OP_SCALE(Point, |, int);
    POINT_OP_SCALE(Point, &, int);
    POINT_OP_SCALE(Point, ^, int);
    POINT_OP_SCALE(Point, >>, int);
    POINT_OP_SCALE(Point, <<, int);

#undef POINT_OP_SCALE
}

#pragma endregion

namespace img
{

    /// Convert Gdiplus::Status to HRESULT
    HRESULT ResultFromStatus(Gdiplus::Status status);

    /// Get the value that's in range of [min, max]
    /// if value is out of range, replace it by min or max
    template<class T>
    T ForceInRange(const T& value, const T& min_value, const T& max_value)
    {
        if(value < min_value) return min_value;
        else if(value > max_value) return max_value;
        else return value;
    }

#pragma region -- Convert to Win32 POINT

    /// [ Convert windows mouse message to mouse position, POINT ].
    inline POINT GetPOINT(LPARAM lParam)
    {
        POINT pt;
        pt.x = (short)(lParam&0xffff);
        pt.y = (short)(lParam>>16);
        return pt;
    }

#pragma endregion

#pragma region -- Convert to Gdiplus::Point

    /// [ Convert windows mouse message to int position, Point ].
    inline Gdiplus::Point GetPoint(LPARAM lParam)
    {
        return Gdiplus::Point((short)(lParam&0xffff), (short)(lParam>>16));
    }

    /// [ Convert windows POINT to Point ].
    inline Gdiplus::Point GetPoint(POINT pt)
    {
        return Gdiplus::Point(pt.x, pt.y);
    }

    /// [ Convert PointF to Point ].
    inline Gdiplus::Point GetPoint(const Gdiplus::PointF& pt)
    {
        return Gdiplus::Point(xtl::RealCast<INT>(pt.X), xtl::RealCast<INT>(pt.Y));
    }

#pragma endregion

#pragma region -- Convert to Gdiplus::PointF

    /// Convert windows mouse message to mouse position, PointF ].
    inline Gdiplus::PointF GetPointF(LPARAM lParam)
    {
        float x = (short)(lParam&0xffff);
        float y = (short)(lParam>>16);
        return Gdiplus::PointF(x, y);
    }

    /// convert int coordinate to gdiplus
    inline Gdiplus::PointF GetPointF(INT x, INT y)
    {
        return Gdiplus::PointF(float(x), float(y));
    }

    /// convert windows point to gdiplus
    inline Gdiplus::PointF GetPointF(const POINT& pt)
    {
        return Gdiplus::PointF(float(pt.x), float(pt.y));
    }

    /// convert windows point to gdiplus
    inline Gdiplus::PointF GetPointF(const Gdiplus::Point& pt)
    {
        return Gdiplus::PointF(float(pt.X), float(pt.Y));
    }

    /// convert windows point to gdiplus
    inline Gdiplus::PointF GetPointF(const Gdiplus::PointF& pt)
    {
        return pt;
    }

#pragma endregion

#pragma region -- Convert to Gdiplus::RectF

    /// convert windows rect into gdiplus
    inline Gdiplus::RectF GetRectF(const RECT& rc)
    {
        return Gdiplus::RectF(
            float(rc.left), float(rc.top), 
            float(rc.right - rc.left), float(rc.bottom - rc.top));
    }

    /// convert Gdiplus int Rect to float RectF
    inline Gdiplus::RectF GetRectF(const Gdiplus::Rect& rc)
    {
        return Gdiplus::RectF(
            float(rc.X), float(rc.Y), float(rc.Width), float(rc.Height));
    }
#pragma endregion

#pragma region -- Convert to Gdiplus::Rect

    /// convert from GDI structur RECT to GdiplusRect
    inline Gdiplus::Rect GetRect(const RECT& rc)
    {
        return Gdiplus::Rect(
            (rc.left), (rc.top), (rc.right - rc.left), (rc.bottom - rc.top));
    }

    /// convert from Gdiplus float RectF to Rect
    inline Gdiplus::Rect GetRect(const Gdiplus::RectF& rc)
    {
        return Gdiplus::Rect(
            xtl::RealCast<INT>(rc.X), xtl::RealCast<INT>(rc.Y),
            xtl::RealCast<INT>(rc.Width), xtl::RealCast<INT>(rc.Height));
    }
#pragma endregion

#pragma region -- Convert to Win32 RECT

    inline RECT GetRECT(const Gdiplus::Rect& rc)
    {
        RECT result = { rc.X, rc.Y, rc.GetRight(), rc.GetBottom()};
        return result;
    }

#pragma endregion

#pragma region -- Point geometry helper functions

    template<class PT>
    float PointLength(const PT& p)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, img::ShortXY) ||
            IS_SAME_TYPE(PT, Gdiplus::Point) ||
            IS_SAME_TYPE(PT, Gdiplus::PointF) );

        return sqrtf((float)(p.X * p.X + p.Y * p.Y));
    }

    template<class PT>
    int PointDot(const PT& p, const PT& q)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, img::ShortXY) ||
            IS_SAME_TYPE(PT, Gdiplus::Point) );

        return p.X * q.X + p.Y * q.Y;
    }

    inline float PointDot(const Gdiplus::PointF& p, const Gdiplus::PointF& q)
    {
        return p.X * q.X + p.Y * q.Y;
    }

    template<class PT>
    PT PointPerpendicular(const PT& p)
    {
        return PT(p.Y, -p.X);
    }

    inline Gdiplus::PointF PointNormalized(const Gdiplus::PointF& p)
    {
        float length = PointLength(p);
        if (length == 0)
        {
            return Gdiplus::PointF(0, 0);
        }
        else
        {
            return p / length;
        }
    }

#pragma endregion

    class ShortXY
    {
    public:
        INT16 X, Y;

    public:
        ShortXY()
        {
            COMPILE_TIME_ASSERT(sizeof(ShortXY) == sizeof(DWORD));
        }
        
        ShortXY(const ShortXY& xy)
        {
            DW() = xy.DW();
        }

        ShortXY& operator = (const ShortXY& xy)
        {
            DW() = xy.DW();
            return *this;
        }

        ShortXY(int x, int y)
        {
            X = xtl::RealCast<short>(x);
            Y = xtl::RealCast<short>(y);
        }

        /// convert this to DWORD, unsigned int32
        /// This is useful to use ShortXY in stdext::hash_xxx
        /// Because stdext::hash_value is evaluated in size_t type
        operator DWORD () const { return DW();}

        /// access as array index, 0 is X, 1 is Y
        short& operator[] (size_t index)             { assert(index < 2); return (&X)[index];}
        const short& operator[] (size_t index) const { assert(index < 2); return (&X)[index];}

        /// Comparison using DWORD representation
        bool operator == (ShortXY in) const { return DW() == in.DW();}
        bool operator != (ShortXY in) const { return DW() != in.DW();}
        bool operator <  (ShortXY in) const { return DW() <  in.DW();}
        bool operator >  (ShortXY in) const { return DW() >  in.DW();}
        bool operator <= (ShortXY in) const { return DW() <= in.DW();}
        bool operator >= (ShortXY in) const { return DW() >= in.DW();}

    private:
        DWORD& DW() { return *(DWORD*)(void*)(&X);}
        const DWORD& DW() const { return *(const DWORD*)(const void*)(&X);}
    };

#define DEFINE_SHORTXY_OP(OP)                                       \
    inline void operator OP##= (ShortXY& lhs, const ShortXY& rhs)   \
    {                                                               \
        lhs.X = lhs.X OP rhs.X;                                     \
        lhs.Y = lhs.Y OP rhs.Y;                                     \
    }                                                               \
    inline ShortXY operator OP (ShortXY lhs, const ShortXY& rhs)    \
    {                                                               \
        lhs.X = lhs.X OP rhs.X;                                     \
        lhs.Y = lhs.Y OP rhs.Y;                                     \
        return lhs;                                                 \
    }                                                               \
    inline void operator OP##= (ShortXY& lhs, short scale)          \
    {                                                               \
        lhs.X = lhs.X OP scale;                                     \
        lhs.Y = lhs.Y OP scale;                                     \
    }                                                               \
    inline ShortXY operator OP (ShortXY lhs, short scale)           \
    {                                                               \
        lhs.X = lhs.X OP scale;                                     \
        lhs.Y = lhs.Y OP scale;                                     \
        return lhs;                                                 \
    }                                                               \
    inline ShortXY operator OP (short scale, ShortXY rhs)           \
    {                                                               \
        rhs.X = scale OP rhs.X;                                     \
        rhs.Y = scale OP rhs.Y;                                     \
        return rhs;                                                 \
    }

    DEFINE_SHORTXY_OP(+);
    DEFINE_SHORTXY_OP(-);
    DEFINE_SHORTXY_OP(*);
    DEFINE_SHORTXY_OP(/);
    DEFINE_SHORTXY_OP(>>);
    DEFINE_SHORTXY_OP(<<);

#undef DEFINE_SHORTXY_OP

#pragma region -- Convert to img::ShortXY

    /// Convert windows mouse message to mouse position, ShortXY.
    inline img::ShortXY GetShortXY(LPARAM lParam)
    {
        short x = (short)(lParam&0xffff);
        short y = (short)(lParam>>16);
        return img::ShortXY(x, y);
    }

    /// convert int coordinate to img::ShortXY
    inline img::ShortXY GetShortXY(INT x, INT y)
    {
        return img::ShortXY(short(x), short(y));
    }

    /// convert windows point to img::ShortXY
    inline img::ShortXY GetShortXY(const POINT& pt)
    {
        return img::ShortXY(short(pt.x), short(pt.y));
    }

    /// convert Point to img::ShortXY
    inline img::ShortXY GetShortXY(const Gdiplus::Point& pt)
    {
        return img::ShortXY(short(pt.X), short(pt.Y));
    }

    /// convert PointF to img::ShortXY
    inline img::ShortXY GetShortXY(const Gdiplus::PointF& pt)
    {
        return img::ShortXY(xtl::RealCast<short>(pt.X), xtl::RealCast<short>(pt.Y));
    }

#pragma endregion

    /// cast point types between ShortXY, Point, and PointF
    template<class DST, class SRC>
    DST PointCast(const SRC& src)
    {
        COMPILE_TIME_ASSERT(IS_SAME_TYPE(SRC, Gdiplus::Point) || 
                            IS_SAME_TYPE(SRC, Gdiplus::PointF) ||
                            IS_SAME_TYPE(SRC, ShortXY));

        COMPILE_TIME_ASSERT(IS_SAME_TYPE(DST, Gdiplus::Point) || 
                            IS_SAME_TYPE(DST, Gdiplus::PointF) ||
                            IS_SAME_TYPE(DST, ShortXY));

        typedef xtl::Select<IS_SAME_TYPE(DST, Gdiplus::PointF), float, int>::Result Type;

        return DST(xtl::RealCast<Type>(src.X), xtl::RealCast<Type>(src.Y));
    }

    template<class RC>
    struct MakeRect
    {
    private:
        template<class RectType> struct ElementType;

        template<> struct ElementType<Gdiplus::Rect>
        {
            typedef INT Result;
        };
        template<> struct ElementType<Gdiplus::RectF>
        {
            typedef float Result;
        };

    public:
        template<class PT>
        static const RC FromPointPair(
            const PT& ptTopLeft, const PT& ptBottomRight)
        {
            typedef typename ElementType<RC>::Result TargetType;
            return RC(
                xtl::RealCast<TargetType>(ptTopLeft.X),
                xtl::RealCast<TargetType>(ptTopLeft.Y),
                xtl::RealCast<TargetType>(ptBottomRight.X - ptTopLeft.X),
                xtl::RealCast<TargetType>(ptBottomRight.Y - ptTopLeft.Y));
        }
    };

}/// namespace img
