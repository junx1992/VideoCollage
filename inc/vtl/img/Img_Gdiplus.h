/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib Gdiplus wrapper
  
Abstract:
    Functions in this header require Gdiplus.DLL

    Look at image_utility.h for those can be replaced without Gdiplus.DLL

Notes:

Usage:
        
History:
    Created  on 2003 Aug. 16 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include <tchar.h>
#include "Img_Utility.h"
#include "Img_Pixel.h"
#include "Img_Image.h"
#include "Img_ScanConvert.hpp"

namespace img
{
    /// Compute the scale factor of the given matrix
    float GetMatrixScale(const Gdiplus::Matrix* pMatrix);

    /// get the zoom scale of pGraphics using GetMatrixScale
    float GetGraphicsScale(const Gdiplus::Graphics* pGraphics);

    /// Copy the a matrix by elements
    void CopyMatrix(Gdiplus::Matrix& matDst, const Gdiplus::Matrix& matSrc);

    /// Transform a rectangle by given matrix
    template<class RC>
    void TransformRect(Gdiplus::Matrix& mat, IN OUT RC& rc)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(RC, Gdiplus::RectF) || 
            IS_SAME_TYPE(RC, Gdiplus::Rect));

        Gdiplus::PointF pt[2] = {
            Gdiplus::PointF((float) rc.X, (float) rc.Y),
            Gdiplus::PointF((float) rc.GetRight(), (float) rc.GetBottom())};
        mat.TransformPoints(&pt[0], 2);
        rc = img::MakeRect<RC>::FromPointPair(pt[0], pt[1]);
    }

    /// Get the bounding box of given bitmap
    inline Gdiplus::Rect GetBound(Gdiplus::Bitmap* bitmap)
    {
        if (bitmap == NULL) return Gdiplus::Rect(0, 0, 0, 0);
        else return Gdiplus::Rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    }

    /// Load image from file, return true if success
    HRESULT LoadImageFile(IImage* pImage, const TCHAR* szFilename);

    /// Save image to file, return true if success
    /// default format image/png can preserve from 1bit to 64bits image
    /// which is important for int, float, and double image
    /// Moreover, png is compressed losslessly by LZW algorithm
    /// Typical szFormat is "image/bmp", "image/jpeg", "image/gif", "image/tiff" and "image/png"
    HRESULT SaveImageFile(const IImage* pImage, const TCHAR* szFilename,
                          const WCHAR* szFormat = L"image/png",
                          const Gdiplus::EncoderParameters *encoderParams = NULL);

    /// Copy the specific rectangle from the bitmap
    /// Allocate pImage if necessary
    /// if pRect == NULL, copy whole image
    HRESULT CopyFromBitmap(IImage* pImage,
        const Gdiplus::Bitmap* pBitmap, const Gdiplus::Rect* pRect = NULL);

    /// Create a Bitmap for display or file I/O
    /// Use current bits buffer, hence it's as fast as AddRef()
    /// if pRect == NULL, use the whole image
    Gdiplus::Bitmap* CreateBitmap(
        const IImage* pImage, 
        const Gdiplus::Rect* pRect = NULL);

    /// Set the Bitmap color palette
    /// If true color image, do nothing
    /// If Byte image, set gray scale palette
    HRESULT SetBitmapPalette(Gdiplus::Bitmap* pBitmap);

    /// associate Graphics to draw on image
    /// Usage:
    ///      Graphics* pGraphics = image.CreateGraphics();
    ///      ...
    ///      delete pGraphics;
    /// or use xtl::ScopedPtr<Graphics> instead of the raw pointer.
    Gdiplus::Graphics* CreateGraphics(
        const IImage* pImage,
        const Gdiplus::Rect* pRect = NULL);

    /// BitBlt BitmapData to HDC
    /// Wrapper of GDI BitBlt for Gdiplus::BitmapData
    HRESULT BitBltBitmapData(
        HDC hdc,
        const Gdiplus::BitmapData& bmpData,
        int dstX, int dstY,
        const Gdiplus::Rect* prcSrc = NULL);

    /// BitBlt image on HDC or graphics
    /// Usage:
    ///      image.BitBlt(hDC);  /// draw whole image at (0, 0)
    ///      image.BitBlt(hDC, 60, -300);    /// set dst position.
    ///      image.BitBlt(hDC, 60, 128, &Rect(0, 0, 32, 32)); /// set src rect.
    HRESULT BitBltImage(
        HDC hdc,
        const IImage* pImage,
        int dstX = 0, int dstY = 0,
        const Gdiplus::Rect* prcSrc = NULL);

    /// Display image on Gdiplus::Graphics, Similar to BitBlt
    /// if rcSrc == NULL, it draws full image (0, 0, Width, Height)
    /// x, y is the offset of rcSrc->TopLeft() on pGraphics
    HRESULT DrawImage(
        Gdiplus::Graphics* pGraphics,
        const IImage* pImage,
        float dstX = 0, float dstY = 0,
        const Gdiplus::Rect* prcSrc = NULL);

    /// Similar to previous one, using integer destination point
    inline HRESULT DrawImage(
        Gdiplus::Graphics* pGraphics,
        const IImage* pImage,
        int dstX, int dstY,
        const Gdiplus::Rect* prcSrc = NULL)
    {
        return DrawImage(pGraphics, pImage, (float) dstX, (float) dstY, prcSrc);
    }

    /// Draw the image of given source rect(in image coordinate) into dst rect(in Graphics)
    HRESULT DrawImage(
        Gdiplus::Graphics* pGraphics,
        const IImage* pImage,
        const Gdiplus::RectF& rcDst,
        const Gdiplus::Rect* prcSrc = NULL);

    /// Draw the image of given source rect(in image coordinate) into dst rect(in Graphics)
    inline HRESULT DrawImage(
        Gdiplus::Graphics* pGraphics,
        const IImage* pImage,
        const Gdiplus::Rect& rcDst,
        const Gdiplus::Rect* prcSrc = NULL)
    {
        return DrawImage(pGraphics, pImage, GetRectF(rcDst), prcSrc);
    }

    /// Widen the given path to a set of polygons
    /// NOTE: a rounded cap pen is used for widen
    HRESULT WidenGraphicsPath(
        Gdiplus::GraphicsPath& path,
        float radius);

    /// Scanconver the given graphics path
    /// Assuming the path contains only polygons
    HRESULT ScanConvertGraphicsPath(
        Gdiplus::GraphicsPath& path,
        const Gdiplus::Rect& rcClip,
        img::ScanConvertCallback callback,
        INT_PTR hint);

    /// Convert a widen polygon to a set of polygons
    /// And insert them into given graphics path
    template<class PT>
    HRESULT WidenPolygonToGraphicsPath(
        Gdiplus::GraphicsPath& path,
        const PT* pts,
        size_t count,
        bool closed,
        float radius);

    /// Widen the given polygon with given radius
    /// And scanconvert it using given callback
    template<class PT>
    HRESULT ScanConvertWidenPolygon(
        const PT* pts,
        size_t count,
        bool closed,
        float radius,
        const Gdiplus::Rect& rcClip,
        img::ScanConvertCallback callback,
        INT_PTR hint);

    /// Display an open/save image dialog, and return the file name
    BOOL OpenImageFileDialog(TCHAR* szFilename, UINT nLen, HWND hWnd = NULL);
    BOOL SaveImageFileDialog(TCHAR* szFilename, UINT nLen, HWND hWnd = NULL);

    /// Set Graphics state for high speed rendering
    void MakeGraphicsHighSpeed(Gdiplus::Graphics* pGraphics);

    /// Set Graphics state for high quality rendering
    void MakeGraphicsHighQuality(Gdiplus::Graphics* pGraphics);


    /// ----------------- Implementation ------------------------------

    template<class PT>
    HRESULT WidenPolygonToGraphicsPath(
        Gdiplus::GraphicsPath& path,
        const PT* pts,
        size_t count,
        bool closed,
        float radius)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, Gdiplus::PointF) ||
            IS_SAME_TYPE(PT, Gdiplus::Point));

        if (pts == NULL) return E_INVALIDARG;

        if (count == 0) return S_OK;
        if (count == 1)
        {
            Gdiplus::RectF rc((float)pts[0].X, (float) pts[0].Y, 0, 0);
            rc.Inflate(radius, radius);
            path.AddEllipse(rc);
            path.Flatten();
        }
        else
        {
            for(size_t k = 1; k < count; k ++)
            {
                path.AddLine(pts[k-1], pts[k]);
                path.StartFigure();
            }

            if (closed)
            {
                path.AddLine(pts[count - 1], pts[0]);
            }

            WidenGraphicsPath(path, radius);
        }

        return S_OK;
    }

    template<class PT>
    HRESULT ScanConvertWidenPolygon(
        const PT* pts,
        size_t count,
        bool closed,
        float radius,
        const Gdiplus::Rect& rcClip,
        img::ScanConvertCallback callback,
        INT_PTR hint)
    {
        COMPILE_TIME_ASSERT(
            IS_SAME_TYPE(PT, Gdiplus::PointF) ||
            IS_SAME_TYPE(PT, Gdiplus::Point));

        HRESULT hr;
        Gdiplus::GraphicsPath path;
        hr = WidenPolygonToGraphicsPath(path, pts, count, closed, radius);
        if (FAILED(hr)) return hr;

        hr = ScanConvertGraphicsPath(path, rcClip, callback, hint);
        if (FAILED(hr)) return hr;

        return S_OK;
    }


} /// namespace img
