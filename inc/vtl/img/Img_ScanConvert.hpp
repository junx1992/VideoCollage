/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib: Scan convert a concaved non-simple polygon
  
Abstract:
    
Notes:
    Code is adabed from Acrylic, June 9, 2005

History:
    Created  on 2005 June, 9 by t-yinli@microsoft.com
          
\*************************************************************************/

#pragma once

#include "xtl/Xtl_Pointer.hpp"
#include <algorithm>
#include <vector>

namespace img
{

    /// function prototype for scan converter to process inside scanlines
    /// Usage example: Callback(int yScanline, int xBegin, int xEnd, INT_PTR hint);
    /// Here, always xBegin < xEnd.
    typedef bool (*ScanConvertCallback)(int, int, int, INT_PTR);

    /// Scan convert a concave non-simple polygon
    /// Polygon can be clockwise or counterclockwise
    /// Algorithm does uniform point sampling at pixel centers.
    /// callback is invoked for each scan line segment inside polygon
    /// Inside test done by Jordan's rule: a point is inside if an
    ///     emanating ray intersects the polygon an odd number of times.
    /// NOTE: P must be able to be accessed by .X and .Y.
    ///       It can be Gdiplus::Point, Gdiplus::PointF, img::ShortXY
    template<class P> HRESULT ScanConvertPolygon(
        const P* pts,
        size_t count,
        const Gdiplus::Rect& clip,
        ScanConvertCallback callback,
        INT_PTR hint);


#pragma region -- Scan conver implementation
        
    namespace impl
    {
        template<class P>
        class ScanConvertHelper
        {
        private:
            INT n;          /// number of points in polygon
            const P* pt;    /// vertices of polygon

            struct Edge
            {
                double x;	/// x coordinate of edge's intersection with current scanline
                double dx;	/// change in x with respect to y
                INT i;	    /// edge index: edge i goes from pt[i] to pt[i+1]
            };
            std::vector<Edge> active;
            INT nact;    /// number of active edges

            struct CompareIndex
            {
                CompareIndex(const P* pt) : m_polygon(pt) {}

                bool operator() (INT a, INT b)
                {
                    return m_polygon[a].Y < m_polygon[b].Y;
                }

            private:
                const P* m_polygon;
            };

            struct CompareEdge
            {
                bool operator() (const Edge& a, const Edge& b)
                {
                    return a.x < b.x;
                }
            };

            void InsertActiveEdge(INT i, INT y) /// append edge i to end of active list
            {
                const P *p, *q;

                INT j = (i < n-1) ? i+1 : 0;
                if (pt[i].Y < pt[j].Y)
	            {
	                p = &pt[i];
                    q = &pt[j];
	            }
                else
	            {
	                p = &pt[j];
                    q = &pt[i];
	            }

                /// initialize x position at intersection of edge with scanline y
                const double dy = ((double)q->Y - (double)p->Y);
                const double dx = ((double)q->X - (double)p->X) / dy;
                active[nact].dx = dx;
                active[nact].x = dx * ((double) y - (double) p->Y) + (double) p->X;
                active[nact].i = i;
                ++ nact;
            }

            void DeleteActiveEdge (INT i) /// remove edge i from active list
            {
                INT j;
                for (j=0; (j < nact) && (active[j].i != i); j++) {}

                if (j >= nact) return;  /// edge not in active list; happens at clip.GetTop()

                -- nact;
                memmove(&active[j], &active[j+1], (nact-j) * sizeof(active[0]));
            }

        public:
            HRESULT Action(
                const P* pts,
                size_t count,
                const Gdiplus::Rect& clip,
                ScanConvertCallback callback,
                INT_PTR hint)
            {
                if(count <= 0) return E_INVALIDARG;
                if(pts == NULL) return E_INVALIDARG;
                if(callback == NULL) return E_INVALIDARG;

                n = (INT) count;
                pt = pts;

                /// initialize index array as natrual order
                std::vector<INT> idx(n);
                active.resize(n);
                for(INT i = 0; i < n; i ++)
                {
                    idx[i] =  i;
                }

                /// Sort idx by the order of pt[idx[k]].Y
                std::sort(&idx[0], &idx[0] + n, CompareIndex(pt));

                nact = 0;       /// number of active edges, init as empty
                INT k = 0;      /// idx[k] is the next vertex to precess
                INT y0 = max(clip.GetTop(), xtl::FloorCast<INT>(pt[idx[0]].Y));     /// the topmost scanline
                INT y1 = min(clip.GetBottom(), xtl::CeilCast<INT>(pt[idx[n-1]].Y)); /// the bottom  scanline

                /// step through scanlines
                /// scanline y is at y+.5 in continuous coordinates
                for(INT y = y0; y < y1; y ++)
                {
		            /// check vertices between previous scanline and current one, if any
                    for (; (k < n) && (pt[idx[k]].Y <= y); k++)
                    {
			            /// to simplify, if pt.y = y+.5, pretend it's above
                        /// invariant: y-.5 < pt[i].y <= y+.5
			            INT i = idx[k];

			            ///  insert or delete edges before and after vertex i 
                        ///  (i-1 to i, and i to i+1) from active list if they cross scanline y
                        INT j = i > 0 ? i-1 : n-1;	/// vertex previous to i
			            if (pt[j].Y <= y)	        /// old edge, remove from active list
                        {
                            DeleteActiveEdge(j);
                        }
                        else if (pt[j].Y > y)       ///  new edge, add to active list
                        {
                            InsertActiveEdge(j, y);
                        }

			            j = i < (n - 1) ? i+1 : 0;	/// vertex next after i
			            if (pt[j].Y <= y)	        /// old edge, remove from active list
				        {
				            DeleteActiveEdge(i);
				        }
			            else if (pt[j].Y > y)	    /// new edge, add to active list
				        {
				            InsertActiveEdge(i, y);
				        }
                    }

		            /// sort active edge list by active[j].x
                    std::sort(&active[0], &active[0] + nact, CompareEdge());

		            /// draw horizontal segments for scanline y
                    for (INT j=0; j < nact; j += 2)	/// draw horizontal segments
                    {
                        /// span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside
                        INT xl = xtl::CeilCast<INT>(active[j].x);		/// left end of span
                        if (xl < clip.GetLeft()) xl = clip.GetLeft();
                        if (xl > clip.GetRight()) xl = clip.GetRight();

                        INT xr = xtl::CeilCast<INT>(active[j+1].x);	 /// right end of span
                        if (xr < clip.GetLeft()) xr = clip.GetLeft();
                        if (xr > clip.GetRight()) xr = clip.GetRight();

                        if (xl < xr)
                        {
                            /// Invoke call back in span
                            if(!callback(y, xl, xr, hint))
                            {
                                /// if call back returns false
                                /// return prematurely
                                return S_OK;
                            }
                        }

                        active[j].x += active[j].dx;	    /// increment edge coords
                        active[j+1].x += active[j+1].dx;
                    }
                }

                return S_OK;
            }
        };
    } // namespace impl

    template<class P>
    HRESULT ScanConvertPolygon(
        const P* pts,
        size_t count,
        const Gdiplus::Rect& clip,
        ScanConvertCallback callback,
        INT_PTR hint)
    {
        impl::ScanConvertHelper<P> helper;
        return helper.Action(pts, count, clip, callback, hint);
    }

#pragma endregion

}   // namespace img
