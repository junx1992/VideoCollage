/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib Image Processing
  
Abstract:
    A group of image processing functions.

Notes:

    TODO:

    /// Apply the callback function on each pixel
    /// typedef void (*Function) (T&);
    template<class T, class Function>
    HRESULT ForEachPixel(img::CImage<T>& imDst, Function function);

    /// Apply the callback function to transform from one to the other image
    /// typedef void (*Function) (T&, const S&);
    template<class T, class S, class Function>
    HRESULT ForEachPixel(img::CImage<T>& imDst, const img::CImage<S>& imSrc, Function function);

     
Usage:
        
History:
    Created  on 2003 Aug. 16 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "Img_Image.h"
#include "Img_ConvHV.hpp"

namespace img
{

    /// blend colors as: r = alpha * s + beta * t
    template<class R, class S, class T>
    void BlendPixel(R& r, const S& s, const T& t, float alpha, float beta);

    /// [Compute the guassian blur image].
    template<class IMAGE>
    HRESULT ImageBlurGaussian(IMAGE& imDst,
        const IMAGE& imSrc, float sigma, xtl::IProgress* progress = NULL);

    /// [Compute the guassian blur image, sigma = 0.717f].
    template<class IMAGE>
    HRESULT ImageBlurFixed121(IMAGE& imDst,
        const IMAGE& imSrc, xtl::IProgress* progress = NULL)
    {
        img::FixedKernel_121<ConvolutionDefault> K;
        return img::ConvolveImageHV(imDst, imSrc, K, K, progress);
    }

    /// [Compute the guassian blur image, sigma = 1f].
    template<class IMAGE>
    HRESULT ImageBlurFixed14641(IMAGE& imDst,
        const IMAGE& imSrc, xtl::IProgress* progress = NULL)
    {
        img::FixedKernel_14641<ConvolutionDefault> K;
        return img::ConvolveImageHV(imDst, imSrc, K, K, progress);
    }

    /// Blur and down sample image by integer facter
    template<class IMAGE>
    HRESULT ImageDecimate(IMAGE& imDst, const IMAGE& imSrc, INT xScale, INT yScale);

    /// [To compute the gradient component on X, and Y].
    template<class ImageGradientType>
    HRESULT ComputeGradientXY(
        ImageGradientType& imGx, ImageGradientType& imGy, const CImageArgb& imSrc);

    /// [composite gradient magnitude by x, y components.
    template<class G, class T>
    HRESULT CompositeGradient(CImage<G>& imG, const CImage<T>& imX, const CImage<T>& imY);

    /// Compute the inverse of given image
    template<class IMAGE>
    HRESULT InvertImageColor(IMAGE& imDst, const IMAGE& imSrc);

    /// Normalize the image to given min max value range
    /// If no min max is given, use default Black and White color of given pixel type
    template <class T> 
    void NormalizeMinMax(CImage<T>& image,
        T min_val = PixelTraits<T>::Black,
        T max_val = PixelTraits<T>::White);

    /// convert label image to Psuedo
    HRESULT ImageLabelToPsuedo(
        CImageArgb& imPsuedo, const CImageInt& imLabel, UINT number_of_colors = 1024);

    /// Morphologic filter
    template<class DilateKernel, class ErosionKernel>
    class MorphologicalFilter
    {
    public:
        MorphologicalFilter(size_t scale) : m_Dilation(scale), m_Erosion(scale)
        {
        }

        template<class ImageType>
        HRESULT Dilate(ImageType& imOut, const ImageType& imIn)
        {
            return ConvolveImageHV(imOut, imIn, m_Dilation, m_Dilation);
        }

        template<class ImageType>
        HRESULT Erode(ImageType& imOut, const ImageType& imIn)
        {
            return ConvolveImageHV(imOut, imIn, m_Erosion, m_Erosion);
        }

        template<class ImageType>
        HRESULT Open(ImageType& imOut, const ImageType& imIn)
        {
            HRESULT hr = Erode(imOut, imIn);
            if (FAILED(hr)) return hr;
            return Dilate(imOut, imOut);
        }

        template<class ImageType>
        HRESULT Close(ImageType& imOut, const ImageType& imIn)
        {
            HRESULT hr = Dilate(imOut, imIn);
            if (FAILED(hr)) return hr;
            return Erode(imOut, imOut);
        }

        template<class ImageType>
        HRESULT Smooth(ImageType& imOut, const ImageType& imIn)
        {
            HRESULT hr = Open(imOut, imIn);
            if (FAILED(hr)) return hr;
            return Close(imOut, imOut);
        }

    private:
        DilateKernel m_Dilation;
        ErosionKernel m_Erosion;

    private:
        MorphologicalFilter();
        MorphologicalFilter(MorphologicalFilter&);
        MorphologicalFilter& operator= (MorphologicalFilter&);
    };

    inline HRESULT BinaryImageErode(CImageByte& imSrcDst, size_t scale)
    {
        typedef BinaryMorphologicalKernel<Erosion, ConvolutionDefault> Kernel;
        Kernel kernel(scale);
        return ConvolveImageHV(imSrcDst, imSrcDst, kernel, kernel);
    }

    inline HRESULT BinaryImageDilate(CImageByte& imSrcDst, size_t scale)
    {
        typedef BinaryMorphologicalKernel<Dilation, ConvolutionDefault> Kernel;
        Kernel kernel(scale);
        return ConvolveImageHV(imSrcDst, imSrcDst, kernel, kernel);
    }

	inline HRESULT BinaryImageErode(
		CImageByte& imDst, const CImageByte& imSrc, size_t scale)
    {
		imDst.Copy(imSrc);
		return BinaryImageErode(imDst, scale);
    }

	inline HRESULT BinaryImageDilate(
		CImageByte& imDst, const CImageByte& imSrc, size_t scale)
    {
		imDst.Copy(imSrc);
		return BinaryImageDilate(imDst, scale);
    }

#pragma region template implementations

    template<class IMAGE>
    HRESULT ImageBlurGaussian(
        IMAGE& imDst,
        const IMAGE& imSrc,
        float sigma,
        xtl::IProgress* progress)
    {
        if (imSrc.IsEmpty()) return E_INVALIDARG;
        if (sigma < 0) return E_INVALIDARG;

        if (sigma == 0)
        {
            return imDst.Copy(imSrc);
        }
        else
        {
            img::ConvolutionKernel<float, ConvolutionDefault> K;
            HRESULT hr = BuildGaussianKernel(K, sigma);
            if (FAILED(hr)) return hr;
            return img::ConvolveImageHV(imDst, imSrc, K, K, progress);
        }
    }

    template<class IMAGE>
    HRESULT InvertImageColor(IMAGE& imDst, const IMAGE& imSrc)
    {
        if(imSrc.IsEmpty()) return E_INVALIDARG;
        if(imDst != imSrc) imDst.Allocate(imSrc);

        typedef typename IMAGE::PixelType PixelType;

        const int cx = imSrc.Width();
        const int cy = imSrc.Height();
        for(int y = 0; y < cy; y ++)
        {
            PixelType* pDst = imDst.RowPtr(y);
            const PixelType* pSrc = imSrc.RowPtr(y);
            for(int x = 0; x < cx; x ++)
            {
                PixelInvert(pDst[x], pSrc[x]);
            }
        }

        return S_OK;
    }

    template<class IMAGE>
    HRESULT ImageDecimate(IMAGE& imDst, const IMAGE& imSrc, INT xScale, INT yScale)
    {
        if (imDst == imSrc) return E_INVALIDARG;
        if (xScale <= 0 || yScale <= 0) return E_INVALIDARG;

        /// if decimate scale is 1, copy image
        if (xScale == 1 && yScale == 1)
        {
            imDst.Copy(imSrc);
            return S_OK;
        }

        const int cx = imSrc.Width() / xScale;
        const int cy = imSrc.Height() / yScale;
        imDst.Allocate(cx, cy);

        typedef typename IMAGE::PixelType PixelType;

        for (int y = 0; y < cy; y ++)
        {
            const PixelType* pSrc = imSrc.RowPtr(y * yScale);
            PixelType* pDst = imDst.RowPtr(y);
            for (int xDst = 0, xSrc = 0; xDst < cx; xDst ++, xSrc += xScale)
            {
                pDst[xDst] = pSrc[xSrc];
            }
        }
        return S_OK;
    }

    template <class T>
    void NormalizeMinMax(CImage<T>& image, T min_val, T max_val)
    {
        if (image.IsEmpty()) return;

        if (min_val > max_val) std::swap(min_val, max_val);

        /// first, estimate min max value
        T _maxv = image(0, 0);
        T _minv = image(0, 0);

        for (int  y = 0; y < image.Height(); y ++)
        {
            T* p = image.RowPtr(y);
            for (int  x = 0; x < image.Width(); x ++)
            {
                if (_minv >(*p)) _minv =(*p);
                if (_maxv <(*p)) _maxv =(*p);
                ++p;
            }
        }

        const T diff = _maxv - _minv;

        if (diff > xtl::Limits<T>::denorm_min())
        {
            const T denorminator =(max_val - min_val) / diff;
            for (int y = 0; y < image.Height(); y ++)
            {
                T* p = image.RowPtr(y);
                for (int x = 0; x < image.Width(); x ++)
                {
                    (*p) = ((*p) - _minv) * denorminator + min_val;
                    ++p;
                }
            }
        }
        else
        {
            /// uniform image
            image.FillPixels(0);
        }
    }

    /// morphological operations
    namespace impl
    {
        template<class S> struct ColorDifference;

        template<>
        struct ColorDifference<BYTE>
        {
            BYTE operator() (const PixelArgb& p, const PixelArgb& q) const
            {
                const int dr = int(p.r) - int(q.r);
                const int dg = int(p.g) - int(q.g);
                const int db = int(p.b) - int(q.b);
                //const float result = sqrtf(float(dr*dr + dg*dg + db*db) / 3.f);
                const int result = (abs(dr) + abs(dg) + abs(db)) / 3;
                assert(result >= 0 && result <= 255);
                return static_cast<BYTE>(result);
            };

            BYTE operator() (const BYTE p, const BYTE q) const
            {
                return static_cast<BYTE>(abs(int(p) - int(q)));
            };

            BYTE operator() (const float p, const float q) const
            {
                return static_cast<BYTE>(fabs(p - q) * 255);
            };
        };

        template<>
        struct ColorDifference<float>
        {
            float operator()(const PixelArgb& p, const PixelArgb& q) const
            {
                const int dr = int(p.r) - int(q.r);
                const int dg = int(p.g) - int(q.g);
                const int db = int(p.b) - int(q.b);
                //const float result = sqrtf(float(dr*dr + dg*dg + db*db) / 3.f);
                const int result = (abs(dr) + abs(dg) + abs(db)) / 3;
                assert(result >= 0 && result <= 255);
                return static_cast<float>(result / 255.f);
            };

            float operator() (const BYTE p, const BYTE q) const
            {
                return static_cast<float>(abs(int(p) - int(q)) / 255.f);
            };

            float operator() (const float p, const float q) const
            {
                return fabs(p - q);
            };
        };

        /// Compute gradient using average of foreward and backward difference
        template<class T>
        struct ComputeGradientXY_On_Scanline
        {
            ColorDifference<T> color_diff;

            void operator() (
                T* pDstX,
                T* pDstY,
                int cx,
                const PixelArgb* pSrcP,      /// upper scanline, y - 1
                const PixelArgb* pSrc0,      /// current scanline, y
                const PixelArgb* pSrcQ)      /// lower scanline, y + 1
            {
                for(int x = 1; x < cx-1; x ++)
                {
                    pDstX[x] = color_diff(pSrc0[x+1], pSrc0[x-1]);
                    pDstY[x] = color_diff(pSrcP[x], pSrcQ[x]);
                }

                /// reflect at left and right boundary
                pDstX[0] = color_diff(pSrc0[0], pSrc0[1]);
                pDstY[0] = color_diff(pSrcP[0], pSrcQ[0]);

                pDstX[cx-1] = color_diff(pSrc0[cx-1], pSrc0[cx-2]);
                pDstY[cx-1] = color_diff(pSrcP[cx-1], pSrcQ[cx-1]);
            }
        };

    } /// namespace impl

    template<class ImageGradientType>
    HRESULT ComputeGradientXY(
        ImageGradientType& imGx,
        ImageGradientType& imGy,
        const CImageArgb& imSrc)
    {
        typedef ImageGradientType::PixelType T;

        if (imSrc.IsEmpty()) return E_INVALIDARG;

        const int cx = imSrc.Width();
        const int cy = imSrc.Height();

        imGx.Allocate(cx, cy);
        imGy.Allocate(cx, cy);

        /// If image size is too small, gradient is zero
        if (cx == 1 || cy == 1)
        {
            imGx.ClearPixels();
            imGy.ClearPixels();
            return S_OK;
        }

        impl::ComputeGradientXY_On_Scanline<T> _InnerFunction;

        /// reflect at top and bottom
        _InnerFunction(imGx.RowPtr(0), imGy.RowPtr(0), cx,
            imSrc.RowPtr(0), imSrc.RowPtr(0), imSrc.RowPtr(1));

        for(int y = 1; y < cy-1; y ++)
        {
            _InnerFunction(imGx.RowPtr(y), imGy.RowPtr(y), cx,
                imSrc.RowPtr(y-1), imSrc.RowPtr(y), imSrc.RowPtr(y+1));
        }

        _InnerFunction(imGx.RowPtr(cy-1), imGy.RowPtr(cy-1), cx,
            imSrc.RowPtr(cy-2), imSrc.RowPtr(cy-1), imSrc.RowPtr(cy-1));

        return S_OK;
    }

    template<class G, class T>
    HRESULT CompositeGradient(CImage<G>& imG, const CImage<T>& imX, const CImage<T>& imY)
    {
        typedef CImage<G> Image_G;
        typedef CImage<T> Image_XY;

        assert(imX.Width() == imY.Width() && imX.Height() == imY.Height());
        if (imX.IsEmpty() || imY.IsEmpty()) return E_INVALIDARG;

        const int cx = imX.Width();
        const int cy = imX.Height();

        imG.Allocate(cx, cy);
        if (imG.IsEmpty()) return E_OUTOFMEMORY;

        for (int y = 0; y < cy; y ++)
        {
            const Image_XY::PixelType* px = imX.RowPtr(y);
            const Image_XY::PixelType* py = imY.RowPtr(y);
            Image_G::PixelType* pg = imG.RowPtr(y);
            for (int x = 0; x < cx; x ++)
            {
                typedef Image_XY::PixelType T;
                typedef xtl::TypeTraits<T>::AccumType AccumType;

                //*/// use L2 norm
                const AccumType xx = vtl::Square(px[x]);
                const AccumType yy = vtl::Square(py[x]);
                pg[x] = xtl::RealCast<Image_G::PixelType>(vtl::Sqrt((xx + yy) / 2.));
                /*/// use L1 norm
                pg[x] = xtl::RealCast<Image_G::PixelType>((abs(px[x]) + abs(py[x])) / 2);
                //*/
            }
        }
        return S_OK;
    }

    /// BlendPixel two pixels into one using given alpha, beta
    /// r = s * alpha + t * beta
    template<class R, class S, class T>
    void BlendPixel(R& r, const S& s, const T& t, float alpha, float beta)
    {
        typedef PixelTraits<R> TraitsR;
        typedef PixelTraits<S> TraitsS;
        typedef PixelTraits<T> TraitsT;
        COMPILE_TIME_ASSERT(TraitsR::ColorChannelNum == TraitsS::ColorChannelNum);
        COMPILE_TIME_ASSERT(TraitsR::ColorChannelNum == TraitsT::ColorChannelNum);
        TraitsR::ChannelType* pr = (TraitsR::ChannelType*) &r;
        const TraitsS::ChannelType* ps = (const TraitsS::ChannelType*) &s;
        const TraitsT::ChannelType* pt = (const TraitsT::ChannelType*) &t;
        for(size_t i = 0; i < TraitsR::ColorChannelNum; i ++)
        {
            pr[i] = xtl::RealCast<TraitsR::ChannelType>(alpha * ps[i] + beta * pt[i]); 
        }
    }

#pragma endregion

    /// The context to fill in Byte image with given value
    /// It's usually used as hint of FillImageMethod
    template<class T>
    struct FillImageContext
    {
        img::CImage<T>* pimTarget;
        T value;
    };

    /// A general call back function to fill a Byte image with value
    /// For example:
    ///     FillImageContext<BYTE> context = { &m_imMask, value};
    ///     ScanConvertPolygon(pts, count, rc, &img::FillImageMethod<BYTE>, (INT_PTR) &context);
    template<class T>
    bool FillImageMethod(int y, int x0, int x1, INT_PTR hint)
    {
        FillImageContext<T>* context = (FillImageContext<T>*) hint;

        assert(y >= 0 && y < context->pimTarget->Height());
        assert(x0 >=0 && x0 < context->pimTarget->Width());
        assert(x1 >=0 && x1 <= context->pimTarget->Width());
        assert(x0 < x1);

        T* pRow = context->pimTarget->RowPtr(y);
        for(int x = x0; x < x1; x ++)
        {
            pRow[x] = context->value;
        }

        return true;
    }

    /// Fill in the image with value inside polygon pts[0..count)
    template<class T, class S>
    HRESULT FillImage(CImage<T>& imTarget,
        const Gdiplus::PointF* pts, size_t count, const Gdiplus::Rect& rcClip,  const S& value)
    {
        FillImageContext<T> context = { &imTarget, (T) value};
        return img::ScanConvertPolygon(
            pts, count, rcClip, &FillImageMethod<T>, (INT_PTR) &context);
    }

    /// Fill in the given graphics path
    /// the path must contains only polygons
    /// Therefore, you should Flatten the path beforehand.
    template<class T, class S>
    HRESULT FillGraphicsPath(CImage<T>& imTarget,
        Gdiplus::GraphicsPath& path, const Gdiplus::Rect& rcClip, const S& value)
    {
        using namespace Gdiplus;

        FillImageContext<T> context = { &imTarget, (T) value};

        /// read the path data
        PathData data;
        path.GetPathData(&data);

        /// analysis path data, get each segment of polygon
        INT start = 0;
        for(INT k = 0; k < data.Count; k ++)
        {
            if(data.Types[k] == PathPointTypeStart)
            {
                start = k;
            }
            else if((data.Types[k] & PathPointTypeCloseSubpath) != 0)
            {
                /// If polygon is found, scan convert it
                img::ScanConvertPolygon(
                    data.Points + start, k - start + 1, rcClip,
                    &FillImageMethod<T>, (INT_PTR) &context);
            }
            else
            {
                /// Assume after flatten, all data are line data
                assert(data.Types[k] == PathPointTypeLine);
            }
        }

        return S_OK;
    }

        /// Copy channel iSrc of imSrc, to the channel iDst to imDst
    template<class T, class S>
    HRESULT CopyChannel(
        CImage<T>& imDst, const CImage<S>& imSrc, 
        ChannelIndex iDst, ChannelIndex iSrc)
    {
        typedef PixelTraits<T>::ChannelType DstChannelType;
        typedef PixelTraits<S>::ChannelType SrcChannelType;

        if (iDst < 0 || iSrc < 0) return E_INVALIDARG;
        if (iDst >= PixelTraits<T>::ChannelNum) return E_INVALIDARG;
        if (iSrc >= PixelTraits<S>::ChannelNum) return E_INVALIDARG;

        const int cx = imSrc.Width();
        const int cy = imSrc.Height();

        if(imDst.Width() != cx || imDst.Height() != cy) return E_INVALIDARG;

        for (int y = 0; y < cy; y ++)
        {
            T* pDst = imDst.RowPtr(y);
            const S* pSrc = imSrc.RowPtr(y);
            for (int x = 0; x < cx; x ++)
            {
                DstChannelType* cpDst = (DstChannelType*) &pDst[x];
                const SrcChannelType* cpSrc = (const SrcChannelType*) &pSrc[x];

                cpDst[iDst] = xtl::RealCast<DstChannelType>(cpSrc[iSrc]);
            }
        }

        imDst.ResetDisplayImage();
        return S_OK;
    }

    /// Copy image pixels, and clamp source pixel values in boundary
    /// imDst and imSrc must be allocated and same size before calling
    template<typename T, typename S>
    HRESULT CopyPixelsWithClamp(
        img::CImage<T>& imDst, const img::CImage<S>& imSrc,
        typename img::PixelTraits<S>::ChannelType minValue = img::PixelTraits<typename img::PixelTraits<S>::ChannelType>::Black, 
        typename img::PixelTraits<S>::ChannelType maxValue = img::PixelTraits<typename img::PixelTraits<S>::ChannelType>::White)
    {
        if (imDst.IsEmpty() || imSrc.IsEmpty()) return E_INVALIDARG;
        if (imDst.GetSize() != imSrc.GetSize()) return E_INVALIDARG;

        typedef img::PixelTraits<T> DstTraits;
        typedef img::PixelTraits<S> SrcTraits;

        if (DstTraits::ColorChannelNum != SrcTraits::ColorChannelNum) return E_INVALIDARG;
        const size_t ColorChannelNum = DstTraits::ColorChannelNum;

        typedef DstTraits::PixelType DstPixelType;
        typedef DstTraits::ChannelType DstChannelType;

        typedef SrcTraits::PixelType SrcPixelType;
        typedef SrcTraits::ChannelType SrcChannelType;

        const int width = imDst.Width();
        const int height = imDst.Height();

        for (int y = 0; y < height; y ++)
        {
            DstPixelType* ptrDst = imDst.RowPtr(y);
            const SrcPixelType* ptrSrc = imSrc.RowPtr(y);

            for (int x = 0; x < width; x ++)
            {
                DstChannelType* pixDst = (DstChannelType*)(&ptrDst[x]);
                const SrcChannelType* pixSrc = (const SrcChannelType*)(&ptrSrc[x]);

                for (int k = 0; k < ColorChannelNum; k ++)
                {
                    SrcChannelType src = pixSrc[k];
                    if (src < minValue) src = minValue;
                    if (src > maxValue) src = maxValue;
                    pixDst[k] = xtl::RealCast<DstChannelType>(src);
                }
            }
        }

        return S_OK;
    }

} /// namespace img