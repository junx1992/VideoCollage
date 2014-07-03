/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2004 Microsoft Corporation

Module Name:
    Vis Lib: Boundary model, for running ants algorithms
  
Abstract:

History:
    Created  on 2005 Sep 12 by oliver_liyin

\*************************************************************************/

#pragma once

#include "img/Img_Image.h"
#include <algorithm>

namespace img
{
    /// Distance from point r, to line segment [p, q]
    template<class PT>
    typename float PointDist2Line(const PT& r, const PT& p, const PT& q);

    /// Compute the length of the given polygon
    /// PT can be PointF, Point, or ShortXY
    template<class PT>
    typename float PolygonLength(
        const PT* polygon, const size_t count, bool fClosed = false);

    /// Compute the length of the polygon, from point iFirst to iLast
    /// If iFirst == iLast, compute length of the closed polygon
    /// PT is ShortXY, Point, or PointF
    template<class PT>
    float PolygonLength(const PT* polygon, size_t count, size_t iFirst, size_t iLast);

    /// Simplify an open polyline segments
    /// PointList constains PT, and support: size(), resize(), and operator[]
    template<class PT, class PointList>
    void SimplifyPolyline(PointList& dst, const PT* src, size_t count, float max_error);

    /// Simplify an closed polygon
    /// PointList constains PT, and support: size(), resize(), and operator[]
    template<class PT, class PointList>
    void SimplifyPolygon(PointList& dst, const PT* src, size_t count, float max_error);

    /// Find the closest line in the given polygon, to the given Point
    /// and return the closest distance, and node i, in which pt to line [i]-[i+1];
    /// If there is one inside threshold, return S_OK.  Otherwise, return E_FAIL;
    /// typical parameters:
    ///      PT   = PointF;
    ///      IDX  = int;
    ///      DST  = float;
    template<class PT, class IDX, class DIST>
    HRESULT FindClosestLine(
        const PT& pt, const PT* polygon, size_t count,
        IDX* piNode, DIST* pflDist, DIST flThreshold);

    /// Resample the polygon In to Out with interval of step pixel(s)
    /// If step <= 0, automatically fit step according to the size of ptsOut
    ///     and if opened, then both end points are same with input polygon
    /// NOTE: the polyline is opened
    template<class PT, class PointList>
    HRESULT ResamplePolygon(
        PointList& ptsOut, const PT* ptsIn, size_t count, bool bClosed, float step = 0);

    /// Use Gdiplus::GraphicsPath::Flatten to resample a cardinal curve into polygons.
    /// PointListis vector of points, it's resized to output list size
    /// NOTE: PointListmust contains PointF
    /// Input ptList is a cardinal curve, output is a polygon
    /// Input bClosed chooses opened or closed cardinal curve
    /// PointList constains PT, and support: size(), resize(), and operator[]
    template<class PointList>
    HRESULT ResampleCurveToPolygon(
        PointList& ptList, bool bClosed, float flFlatness = Gdiplus::FlatnessDefault);

    /// Compute the bounding box of given polygon
    template<class RC, class PT>
    void PolygonBoundingBox(RC& rect, const PT* polygon, size_t count);

#pragma region Implementation

    template<class PT>
    typename float PointDist2Line(const PT& r, const PT& p, const PT& q)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, img::ShortXY) ||
            IS_SAME_TYPE(PT, Gdiplus::Point) ||
            IS_SAME_TYPE(PT, Gdiplus::PointF) );

        typedef xtl::Select<IS_SAME_TYPE(PT, Gdiplus::PointF), float, int>::Result DotType;

        const PT n = q - p;
        const PT rp = r - p;

        using img::PointDot;
        using img::PointLength;

        const DotType ndotn = PointDot(n, n);
        if (ndotn <= xtl::Limits<float>::denorm_min())
        {
            return PointLength(rp);
        }
        else
        {
            const DotType rpdotn = PointDot(rp, n);
            if (rpdotn > ndotn)
            {
                return PointLength(r - q);
            }
            else if (rpdotn < 0)
            {
                return PointLength(rp);
            }
            else
            {
                /// We have to compute alpha in floating point here
                /// otherwise the precision will be inaccurate
                const float alpha = (float) rpdotn / (float) ndotn;
                return PointLength(img::GetPointF(rp) - img::GetPointF(n) * alpha);
            }
        }
    }

    template<class PT>
    typename float PolygonLength(const PT* polygon, size_t count, bool fClosed)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, Gdiplus::Point) || 
            IS_SAME_TYPE(PT, Gdiplus::PointF) ||
            IS_SAME_TYPE(PT, img::ShortXY));

        if (count < 2) return 0;

        float sum = 0;
        for (size_t k = 1; k < count; k ++)
        {
            sum += PointLength(polygon[k-1] - polygon[k]);
        }
        if (fClosed)
        {
            sum += PointLength(polygon[count-1] - polygon[0]);
        }
        return sum;
    }

    template<class PT>
    float PolygonLength(const PT* polygon, size_t count, size_t iFirst, size_t iLast)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, Gdiplus::Point) || 
            IS_SAME_TYPE(PT, Gdiplus::PointF) ||
            IS_SAME_TYPE(PT, img::ShortXY));

        if (polygon == NULL || count == 0) return 0;
        if (iFirst >= count || iLast >= count) return 0;

        float sumLength = 0;
        size_t number = iLast + count - iFirst;
        if (number > count) number -= count;

        size_t i = iFirst;
        for (size_t k = 0; k < number; k ++)
        {
            const size_t j = (i + 1) % count;
            sumLength += img::PointLength(polygon[i] - polygon[j]);
            i = j;  /// move to next point
        }
        return sumLength;
    }

    /// Find the maximum error point to the given line segment
    /// The line segment is defined from index idp to idq.
    /// Typically idp < idq.
    /// If idp > idq, consider input polygon looped as closed polygon
    /// Return size_t(-1) if failed
    template<class PT>
    size_t FindMaxErrorPoint(
        const PT* polygon, size_t count, size_t idp, size_t idq, float max_error)
    {
        const PT p = polygon[idp];
        const PT q = polygon[idq];
        float max_dist = -1;
        size_t max_k = idp;

        const size_t N = count;
        if (N == 0) return size_t(-1);

        for (size_t k = (idp+1)%N; k != idq; k = (k+1)%N)
        {
            const PT r = polygon[k];
            const float dist = img::PointDist2Line(r, p, q);
            if (dist > max_dist)
            {
                max_dist = dist;
                max_k = k;
            }
        }
        return(max_dist > max_error) ? max_k : size_t(-1);
    }

    template<class PT, class PointList>
    void SimplifyPolygon(PointList& dst, const PT* src, size_t count, float max_error)
    {
        const size_t M = count;

        /// If the polygon has two or less points
        /// Simply copy points, no need to simplify
        if (M <= 2)
        {
            dst.resize(count);
            for (size_t k = 0; k < count; k ++)
            {
                dst[k] = src[k];
            }
            return;
        }

        using img::PointDot;
        using img::PointLength;

        /// find the max curverture point in src
        float max_cur = 0;
        size_t max_k = 0;
        for (size_t k = 0; k < M; k ++)
        {
            const PT r = src[k];
            const PT p = src[(k+3)%M];
            const PT q = src[(k-3+M)%M];

            const PT rp = r - p;
            const PT rq = r - q;

            const float dot = PointDot(rp, rq);
            const float rpdot = PointDot(rp, rp);
            const float rqdot = PointDot(rq, rq);

            const float curvature = 1 - fabsf(dot * dot / rpdot / rqdot);

            if (max_cur < curvature)
            {
                max_cur = curvature;
                max_k = k;
            }
        }

        /// use id to represent dst, id of src list
        std::vector<size_t> id_dst;
        id_dst.push_back(max_k);
        PT p0(src[max_k]);

        /// find the max dist point from the first point
        float max_dist = 0;
        size_t max_i;
        for (size_t i = 0; i < M; i ++)
        {
            const float dist = PointLength(p0 - src[i]);
            if (max_dist < dist)
            {
                max_dist = dist;
                max_i = i;
            }
        }
        id_dst.push_back(max_i);

        for (;;)
        {
            bool bChanged = false;
            for (size_t k = 0; k < id_dst.size(); k ++)
            {
                const size_t N = id_dst.size();

                const size_t id = 
                    FindMaxErrorPoint(&src[0], count, id_dst[k], id_dst[(k+1)%N], max_error);

                /// add the max error point into curve
                if (id != size_t(-1))
                {
                    bChanged = true;

                    /// insert after the k-th point
                    id_dst.insert(id_dst.begin() + k + 1, id);

                    /// advance k to skip new point
                    k ++;
                }                
            }

            if (!bChanged) break;
        }

        if (id_dst.size() > 2)
        {
            // convert the id into pointF2 list
            dst.resize(id_dst.size());
            for (size_t k = 0; k < dst.size(); k ++)
            {
                dst[k] = src[id_dst[k]];
            }
        }
        else
        {
            dst.clear();
        }
    }

    /// simplify the segment for an open line segments
    template<class PT, class PointList>
    void SimplifyPolyline(PointList& dst, const PT* src, size_t count, float max_error)
    {
        if (count <= 2) return;

        std::vector<size_t> ids;
        ids.push_back(0);
        ids.push_back(count - 1);

        for (;;)
        {
            bool bChanged = false;
            for (size_t k = 1; k < ids.size(); k ++)
            {
                assert(ids[k-1] < ids[k]);  /// must be opened curve
                const size_t id = FindMaxErrorPoint(src, count, ids[k-1], ids[k], max_error);

                /// add the max error point into curve
                if (id != size_t(-1))
                {
                    bChanged = true;

                    /// insert after the (k-1)th point
                    ids.insert(ids.begin() + k, id);

                    /// advance k to skip new point
                    k ++;
                }                
            }

            if (!bChanged) break;
        }

        dst.resize(ids.size());
        for (size_t k = 0; k < ids.size(); k ++)
        {
            dst[k] = src[ids[k]];
        }
    }

    template<class PT, class IDX, class DIST>
    HRESULT FindClosestLine(
        const PT& pt, const PT* polygon, size_t count,
        IDX* piNode, DIST* pflDist, DIST flThreshold)
    {
        assert(piNode != NULL && pflDist != NULL);
        if (piNode == NULL || pflDist == NULL) return E_INVALIDARG;
        if (flThreshold < 0) return E_INVALIDARG;
        if (count < 2) return E_FAIL;

        (*pflDist) = xtl::Limits<DIST>::max_value();
        for (IDX k = 1; k < (IDX) count; k ++)
        {
            const PT& p = polygon[k-1];
            const PT& q = polygon[k];

            const DIST dist = (DIST) PointDist2Line(pt, p, q);
            if (dist < (*pflDist))
            {
                (*pflDist) = dist;
                (*piNode) = k - 1;
            }
        }

        return ((*pflDist) < flThreshold) ? S_OK : E_FAIL;
    }

    /// Resample the polyline In to Out with interval of step pixel(s)
    template<class PT, class PointList>
    HRESULT ResamplePolygon(
        PointList& ptsOut, const PT* ptsIn, size_t count, bool bClosed, float step)
    {
        /// Cannot simplify not-a-line input polygon
        if (count < 2) return E_INVALIDARG;

        /// compute the length of input polygon
        const float lengthIn = PolygonLength(ptsIn, count, bClosed);

        assert(lengthIn > 0);
        if (lengthIn == 0) return E_INVALIDARG;

        if (step > 0)
        {
            ptsOut.clear();

            /// refine the step, distribute round off error to all
            step = lengthIn / xtl::RealCast<int>(lengthIn / step);

            float length = 0;  /// the length that is already covered in segment
            for(size_t i = 0; i < count - (bClosed ? 0 : 1); i++)
            {
                const PT& pt0 = ptsIn[i % count];
                const PT& pt1 = ptsIn[(i + 1) % count];

                /// length of current segment
                const float L = PointLength(pt0 - pt1);

                /// sample current segment
                while(length < L)
                {
                    ptsOut.push_back(vtl::Lerp(pt0, pt1, length / L));
                    length += step;
                }

                /// reset the length to and add remaining to next segment
                length -= L;
                assert(length >= 0 && length < step);
            }
        }
        else
        {
            assert(!ptsOut.empty());
            if (ptsOut.size() < 2) return E_INVALIDARG;

            /// compute the step of each node in ptsOut
            step = lengthIn / (ptsOut.size() - (bClosed ? 0 : 1));

            /// refine the step, distribute round off error to all
            step = lengthIn / xtl::RealCast<int>(lengthIn / step);

            size_t next = 0;   /// the position to be added in ptsOut
            float length = 0;  /// the length that is already covered in segment
            for(size_t i = 0; i < count - (bClosed ? 0 : 1); i++)
            {
                /// current segment
                const PT& pt0 = ptsIn[i % count];
                const PT& pt1 = ptsIn[(i + 1) % count];

                /// length of current segment
                const float L = PointLength(pt0 - pt1);

                /// sample current segment
                while(length < L)
                {
                    /// Due to round off error, there might be one more points
                    /// we can ignore it, since the error is very small
                    if (next >= ptsOut.size())
                    {
                        assert(next == ptsOut.size());          /// must be last point in ptsOut
                        goto premature_exit;
                    }

                    ptsOut[next] = vtl::Lerp(pt0, pt1, length / L);
                    length += step;
                    ++ next;
                }

                /// reset the length to and add remaining to next segment
                length -= L;
                assert(length >= 0 && length < step);
            }

            /// Due to round off error, there might be less points
            if(next != ptsOut.size())
            {
                assert(next == ptsOut.size()-1);
                ptsOut[next] = ptsIn[bClosed ? 0 : count - 1];
            }

premature_exit:
            __noop;
        }

        return S_OK;
    }

    template<class PointList>
    HRESULT ResampleCurveToPolygon(PointList& ptList, bool bClosed, float flFlatness)
    {
        /// if two or less points, no need to resample
        if (ptList.size() <= 2) return S_OK;

        Gdiplus::GraphicsPath path;

        if (bClosed)
        {
            path.AddClosedCurve(&ptList[0], (INT)ptList.size());
        }
        else
        {
            path.AddCurve(&ptList[0], (INT)ptList.size());
        }

        /// flat curve using Gdiplus
        path.Flatten(0, flFlatness);

        /// Read the results out
        ptList.resize(path.GetPointCount());
        path.GetPathPoints(&ptList[0], (INT) ptList.size());
        return S_OK;
    }

    /// Compute the bounding box of given polygon
    template<class RC, class PT>
    void PolygonBoundingBox(RC& rect, const PT* polygon, size_t count)
    {
        if (count == 0)
        {
            rect.X = rect.Y = rect.Width = rect.Height = 0;
        }
        else
        {
            PT tl(polygon[0]), br(polygon[0]);

            for (size_t k = 1; k < count; k ++)
            {
                const PT& pt = polygon[k];
                if(tl.X > pt.X) tl.X = pt.X;
                if(tl.Y > pt.Y) tl.Y = pt.Y;
                if(br.X < pt.X) br.X = pt.X;
                if(br.Y < pt.Y) br.Y = pt.Y;
            }

            rect = MakeRect<RC>::FromPointPair(tl, br);
        }
    }

#pragma endregion

} /// namespace img