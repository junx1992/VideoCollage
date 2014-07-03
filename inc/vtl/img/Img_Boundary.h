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

#undef ENABLE_XTL_STD_LINEL_MAP

#ifdef ENABLE_XTL_STD_LINEL_MAP
#include "xtl/Xtl_Std.hpp"
#define MULTIMAP xtl::std::multimap
#else
#include <map>
#define MULTIMAP ::std::multimap
#endif

#include "img/Img_Image.h"
#include <algorithm>

namespace img
{
    /// This condition is usually used to build running ants of a selection
    /// For example, To build running ants for gray image, threshold is usually 127
    ///     boundary.Build(imMask, &img::InsideGreaterThan, 127);
    inline BOOL InsideGreaterThan(INT value, INT_PTR hint) { return value > hint;}

    /// This condition is usually used to build boundary of region with given label
    /// For example, build boundary of uncertain region in matting trimap
    ///     boundary.Build(imMask, &img::InsideEqualsTo, 127);
    inline BOOL InsideEqualsTo(INT value, INT_PTR hint) { return value == hint;}

    /// A polygonal boundary model of a mask image
    /// Which is located in the seams between pixels
    class Boundary
    {
    public:
        typedef img::ShortXY PointType;
        typedef std::vector<PointType> OneListType;
        typedef std::vector<OneListType> AllListType;

    public:
        /// clear the boundary polygons
        void Clear();

        /// Build running ants from a image
        /// The Condition should take T as parameter, and return BOOL or BYTE
        /// The hint will be sent to condition function everytime it's called
        /// For example:
        ///     Build(imMask, &img::InsideGreaterThan, 127);
        template<class T, class Condition>
        HRESULT Build(const img::CImage<T>& imRegion, Condition condition, INT_PTR hint);

        /// Default inside condition is to build running ants in mask image
        template<class T>
        HRESULT Build(const img::CImage<T>& imRegion)
        {
            return Build(imRegion, &img::InsideGreaterThan, 127);
        }

        /// if the boundary mode is empty
        BOOL IsEmpty() const { return m_rgAll.empty();}

        /// Get the bounding box
        Gdiplus::Rect GetBox() const;

        /// Get the number of polygons
        size_t GetNumber() const { return m_rgAll.size();}

        /// Get the k-th polygon, add (-1/2, -1/2) to PointF points
        HRESULT GetPolygon(size_t k, std::vector<img::ShortXY>& pts) const;
        HRESULT GetPolygon(size_t k, std::vector<Gdiplus::Point>& pts) const;
        HRESULT GetPolygon(size_t k, std::vector<Gdiplus::PointF>& pts) const;

    private:

        /// a vector of bool values
        typedef std::vector<BYTE> BoolVector;

        /// Line elemental map, starting from ShortXY, and have INT pixels length
        typedef MULTIMAP<img::ShortXY, img::ShortXY> LinelMap;

        /// Linel coordinate is on topleft of pixel
        /// Linel coordinate is in range [0, cx] x [0, cy], inclusive
        LinelMap m_linelsH, m_linelsV;

        void ConnectLinels();

        /// Fill TRUE/FALSE indicating in/outside of polygon, for the given scanline
        /// Note: InsideCondition must take T as parameter, and return BYTE or BOOL
        /// The stride is image stride, in bytes, no matter what T is
        template<class T, class Condition>
        void FillScanline(BYTE* result,
            const T* src, int count, int stride, Condition condition, INT_PTR hint)
        {
            for (int k = 0; k < count; ++k, ++result)
            {
                *result = (BYTE) condition(*src, hint);
                src = (T*) (((BYTE*)(src)) + stride);
            }
        }

        /// add one linel from first to last, with currect direction
        template<BOOL horizontal>
        void FinishLinel(
            Boundary::LinelMap& linels, int first, int last, int level, BOOL upward);

        template<BOOL horizontal>
        void FindLinels(
            Boundary::LinelMap& linel,
            const Boundary::BoolVector& up,
            const Boundary::BoolVector& down,
            int count, int level);

        /// Find the horizontal linels, given the two scanlines of inside condition
        void FindLinelsH(
            Boundary::LinelMap& linel,
            const Boundary::BoolVector& up,
            const Boundary::BoolVector& down,
            int count, int level);

        /// Find the vertical linels, given the two scanlines of inside condition
        void FindLinelsV(
            Boundary::LinelMap& linel,
            const Boundary::BoolVector& left,
            const Boundary::BoolVector& right,
            int count, int level);

    private:
        AllListType m_rgAll;
    };

    template<class T, class Condition>
    HRESULT Boundary::Build(const img::CImage<T>& imRegion, Condition condition, INT_PTR hint)
    {
        if (condition == NULL) return E_FAIL;

        /// Clear the previous results
        Clear();

        /// if target image is empty, we are done.
        if (imRegion.IsEmpty()) return S_OK;

        /// get the size of the image
        int cx = imRegion.Width();
        int cy = imRegion.Height();

        /// false == background, true == foreground
        BoolVector scanline[2];

        /// ------- Try to find the horizontal linels ----------------------------
        scanline[0].resize(cx);
        scanline[1].resize(cx);

        /// fill in top most line as background
        std::fill(scanline[0].begin(), scanline[0].end(), FALSE);

        /// find lines in between
        for (int y = 0; y < cy; y ++)
        {
            /// update scanline[1] to next scanline
            FillScanline(&scanline[1][0],
                imRegion.RowPtr(y), cx, imRegion.PixelSize, condition, hint);

            /// [0] is always one scanline above [1]
            FindLinelsH(m_linelsH, scanline[0], scanline[1], cx, y);
            scanline[0].swap(scanline[1]);
        }

        /// fill in bottom most line as background
        std::fill(scanline[1].begin(), scanline[1].end(), FALSE);

        /// compute the bottom most scanline
        FindLinelsH(m_linelsH, scanline[0], scanline[1], cx, cy);

        /// ------- Try to find the vertical linels ----------------------------
        scanline[0].resize(cy);
        scanline[1].resize(cy);

        /// Fill in left most line as background
        std::fill(scanline[0].begin(), scanline[0].end(), FALSE);

        const int imRegionStride = imRegion.Stride();
        for (int x = 0; x < cx; x ++)
        {
            /// update scanline[1] to next scanline
            FillScanline(&scanline[1][0],
                imRegion.RowPtr(0) + x, cy, imRegionStride, condition, hint);

            /// [0] is always one scanline left of [1]
            FindLinelsV(m_linelsV, scanline[0], scanline[1], cy, x);
            scanline[0].swap(scanline[1]);
        }

        /// fill in right most line as background
        std::fill(scanline[1].begin(), scanline[1].end(), FALSE);

        /// compute the right most scanline
        FindLinelsV(m_linelsV, scanline[0], scanline[1], cy, cx);

        /// ---- Connect linels into running ants ----
        ConnectLinels();

        return S_OK;
    }

} /// namespace img