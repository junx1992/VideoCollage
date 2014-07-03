// CubicBezier.h: interface for the CubicBezier class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "img/Img_Utility.h"
#include <vector>

namespace img
{

    ///
    ///                       [-1  3 -3 1][P0] 
    ///                       [ 3 -6  3 0][P1]
    ///   P(u) = [u^3 u^2 u 1][-3  3  0 0][P2]
    ///                       [ 1  0  0 0][P3]
    ///
    ///                       [-1  3 -3 1][P0] 
    ///                       [ 3 -6  3 0][P1]
    ///   D(u) = [3u^2 2u 1 0][-3  3  0 0][P2]
    ///                       [ 1  0  0 0][P3]
    ///
    ///                       [-1  3 -3 1][P0] 
    ///                       [ 3 -6  3 0][P1]
    ///   Q(u) = [6u  2  0  0][-3  3  0 0][P2]
    ///                       [ 1  0  0 0][P3]
    ///                       ________________
    ///                        pre-calculate points

    class CubicBezier
    {
    public:
        /// use PointF to be compatible with Gdiplus algorithms
        typedef Gdiplus::PointF PointType;
        typedef float ValueType;

        CubicBezier() : m_fDirty(true) {}

        PointType P   (ValueType u) const;           /// the point at
        PointType D   (ValueType u) const;           /// the first derivative
        PointType Q   (ValueType u) const;           /// the second derivative
        PointType N   (ValueType u) const;           /// the normal at
        ValueType R   (ValueType u) const;           /// radius of curvature

        /// Length of the curve
        ValueType Length() const;

        /// distance from point to curve
        ValueType DistantToPoint(const PointType& point);
        
        /// set four control points
        void SetControlPoints(const PointType* points);

        /// set one control point at
        void SetControlPoint(size_t n, const PointType& point);

        /// get one control point at
        const PointType& ControlPoint(size_t i) const { return m_ptControl[i];}

    private:
        /// pre calculate for better performance
        void Precalculate();

    private:
        PointType m_ptControl[4];	
        PointType m_ptPreCalc[4];
        bool m_fDirty;
    };
} // namespace img