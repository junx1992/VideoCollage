/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib Image Processing, Convolution HV, seperate kernel for x and y.
  
Abstract:

Notes:
    1.  The convolution kernel is designed as policy class,
        since virtual functions cannot be template.
     
Usage:
    ConvolveImageAlgorithm<PixelByte, PixelByte> convolver;
    convolver.ConvolveHV(imDst, imSrc);
    convolver.ConvolveHV(imSelf, imSelf);
        
History:
    Created  on 2004 Dec. 18 by t-yinli
          
\*************************************************************************/

#pragma once

#include "img/Img_Image.h"
#include "xtl/Xtl_Progress.hpp"
#include "vtl/Vtl_Vector.hpp"

namespace img
{
    enum PaddingMethod
    {
        PaddingClamp    = 0,    /// if index is outside, it's clamped to nearest inside index
        PaddingRepeat   = 1,    /// if index is outside, wrap to the other side of image
        PaddingReflex   = 2,    /// if index is outside, it's mirrored into inside
        PaddingZero     = 3,    /// if index is outside, color / gray is fixed zero
    };

    template<BOOL includeAlpha, PaddingMethod padding>
    struct ConvolutionTraits
    {
        const static BOOL IncludeAlpha = includeAlpha;
        const static PaddingMethod Padding = padding;
    };

    typedef ConvolutionTraits<FALSE, PaddingClamp> ConvolutionDefault;

    /// General convolution kernel
    /// Including a float kernel and an integer kernel as well
    template<class FLT, class Traits> class ConvolutionKernel;

    /// simple and fast kernel for Gaussian kernel sigma = 0.7
    template<class Traits> class FixedKernel_121;

    /// simple and fast kernel for Gaussian kernel sigma = 1
    template<class Traits> class FixedKernel_14641;

    /// morphological filter.  if fDilation == false, it's erosion
    enum MorphologicalOperation { Dilation, Erosion };
    template<MorphologicalOperation operation, class Traits> class MorphologicalKernel;
    template<MorphologicalOperation operation, class Traits> class BinaryMorphologicalKernel;
    
    /// Convolve the image horizontally, using given kernel
    template<class IMG_DST, class IMG_SRC, class KERNEL>
    HRESULT ConvolveImageH(IMG_DST& imDst, const IMG_SRC& imSrc,
                           KERNEL& kernel, xtl::IProgress* progress = NULL)
    {
        if (imSrc.IsEmpty()) return E_INVALIDARG;
        if (imDst != imSrc) imDst.Allocate(imSrc);
        impl::ConvolveImageAlgorithm<IMG_DST, IMG_SRC> convolver;

        if(progress) progress->SetMessage(_T("Convolving image horizontally ..."));
        return convolver.ConvolveH(imDst, imSrc, kernel, progress);
    }

    /// Convolve the image vertically, using given kernel
    template<class IMG_DST, class IMG_SRC, class KERNEL>
    HRESULT ConvolveImageV(IMG_DST& imDst, const IMG_SRC& imSrc,
                           KERNEL& kernel, xtl::IProgress* progress = NULL)
    {
        if (imSrc.IsEmpty()) return E_INVALIDARG;
        if (imDst != imSrc) imDst.Allocate(imSrc);
        impl::ConvolveImageAlgorithm<IMG_DST, IMG_SRC> convolver;

        if(progress) progress->SetMessage(_T("Convolving image virtically ..."));
        return convolver.ConvolveV(imDst, imSrc, kernel, progress);
    }

    /// Convolve the image using separate 2D kernel
    template<class IMG_DST, class IMG_SRC, class KERNELH, class KERNELV>
    HRESULT ConvolveImageHV(IMG_DST& imDst, const IMG_SRC& imSrc,
                            KERNELH& kernelH, KERNELV& kernelV, xtl::IProgress* progress = NULL)
    {
        HRESULT hr = S_OK;
        if(progress) progress->PushRangeMapping(0, 100, 0, 200);
        hr = ConvolveImageH(imDst, imSrc, kernelH, progress);
        if(FAILED(hr)) return hr;
        if(progress) progress->PopRangeMapping();

        if(progress) progress->PushRangeMapping(100, 200, 0, 200);
        hr = ConvolveImageV(imDst, imDst, kernelV, progress);
        if(FAILED(hr)) return hr;
        if(progress) progress->PopRangeMapping();
        return S_OK;
    }

    namespace impl
    {
        /// Backward == TRUE,  when move back toward negative infinity
        /// Backward == FALSE, when move forward toward positive infinity
        template<PaddingMethod, BOOL Backward>
        struct IndexWrapper;

        template<BOOL Backward>
        struct IndexWrapper<PaddingReflex, Backward>
        {
            /// Compute the index by reflecting toward head, i.e. to smaller index
            static int Wrap(int x, int width)
            {
                for(;;)
                {
                    bool ready = false;

                    if (x < 0) x = -x;
                    else ready = true;

                    if (x >= width) x = (2 * width - 1) - x;
                    else if (ready) break;
                }
                return x;
            }
        };

        template<BOOL Backward>
        struct IndexWrapper<PaddingRepeat, Backward>
        {
            /// repeat the index outside the image region
            static int Wrap(int x, int width)
            {
                return (x % width);
            }
        };

        template<>
        struct IndexWrapper<PaddingClamp, TRUE>
        {
            /// if x is left of image region, set x to leftmost
            static int Wrap(int x, int /*width*/)
            {
                return (x < 0) ? 0 : x;
            }
        };

        template<>
        struct IndexWrapper<PaddingClamp, FALSE>
        {
            /// if x is right of image region, set x to rightmost
            static int Wrap(int x, int width)
            {
                return (x >= width) ? width - 1 : x;
            }
        };

        /// Access a scanline, which can be continouse or have interval of stride
        /// Can continousely iterate the scanline using ++
        /// Or randomly access by operator[]
        template<class PIXEL>
        class ScanlinePtr
        {
        public:
            typedef PIXEL PixelType;

        public:
            ScanlinePtr() : m_ptr(NULL), m_offsetBytes(0)
            {
            }
            
            /// Set the iterator to given position and offset
            ScanlinePtr(PixelType* ptr, size_t pixelOffset)
                : m_ptr((BYTE*) ptr)
                , m_offsetBytes(pixelOffset)
            {
            }

            /// access the item using pointer interface
            PixelType* operator-> () const { return  (PixelType*) m_ptr;}
            PixelType& operator*  () const { return  *(PixelType*)m_ptr;}
            
            /// advance the current position of the iterator
            void operator ++ ()
            { 
                m_ptr += m_offsetBytes;
            }

            /// access the item using array indexing operator
            PixelType& operator[] (int pos) const
            {
                return *(PixelType*)(m_ptr + pos * m_offsetBytes);
            }

        private:
            BYTE* m_ptr;            /// the pointer to current pixel of iteration
            size_t m_offsetBytes;   /// the offset between pixels, in byte
        };

        /// Each convolution kernel must have a interface
        /// template<class PixelDst, class PixelSrc>
        ///      void Convolve(ScanlinePtr<PixelDst>& it, const PixelSrc* pSrc, size_t size)
        template<class IMG_DST, class IMG_SRC>
        class ConvolveImageAlgorithm
        {
        public:
            typedef IMG_DST ImageDst;
            typedef IMG_SRC ImageSrc;
            typedef typename IMG_DST::PixelType PixelDst;
            typedef typename IMG_SRC::PixelType PixelSrc;

            template<class KERNEL>
            HRESULT ConvolveH(ImageDst& imDst, const ImageSrc& imSrc,
                KERNEL& kernel, xtl::IProgress* progress = NULL);

            template<class KERNEL>
            HRESULT ConvolveV(ImageDst& imDst, const ImageSrc& imSrc,
                KERNEL& kernel, xtl::IProgress* progress = NULL);

        private:
            /// copy the scanline of continuous memory block of given size
            void CopyLine(PixelSrc* pDst, const PixelSrc* pSrc, size_t size)
            {
                memcpy(pDst, pSrc, size * sizeof(PixelSrc));
            }

            /// copy the scanline of not continous src to continous dst
            /// pixels in src have interval of stride
            void CopyLine(PixelSrc* pDst, const PixelSrc* pSrc, size_t stride, size_t size)
            {
                for (size_t y = 0; y < size; y ++)
                {
                    *pDst = *pSrc;
                    pSrc = (PixelSrc*)((BYTE*)(pSrc) + stride);
                    pDst ++;
                }
            }
        };

        /// Convolve the given image horizontally
        template<class IMG_DST, class IMG_SRC> template<class KERNEL>
        HRESULT ConvolveImageAlgorithm<IMG_DST, IMG_SRC>::ConvolveH(
            ImageDst& imDst, const ImageSrc& imSrc,
            KERNEL& kernel, xtl::IProgress* progress = NULL)
        {
            const int cx = imSrc.Width();
            const int cy = imSrc.Height();
            assert(imDst.Width() == cx && imDst.Height() == cy);

            xtl::ScopedArray<PixelSrc> copyline(new PixelSrc[cx]);
            if (copyline == NULL) return E_OUTOFMEMORY;

            /// Do convolution row by row
            for (int y = 0; y < cy; y ++)
            {
                if (progress && (y % (cy/20+1) == 0))
                {
                    progress->SetValue(y, 0, cy);
                }

                ScanlinePtr<PixelDst> ptrDst(imDst.RowPtr(y), sizeof(PixelDst));
                CopyLine(copyline, imSrc.RowPtr(y), cx);
                kernel.Convolve(ptrDst, &copyline[0], cx);
            }

            return S_OK;
        }

        /// Convolve the given image vertically
        template<class IMG_DST, class IMG_SRC> template<class KERNEL>
        HRESULT ConvolveImageAlgorithm<IMG_DST, IMG_SRC>::ConvolveV(
            ImageDst& imDst, const ImageSrc& imSrc,
            KERNEL& kernel, xtl::IProgress* progress = NULL)
        {
            const int cx = imSrc.Width();
            const int cy = imSrc.Height();
            assert(imDst.Width() == cx && imDst.Height() == cy);

            xtl::ScopedArray<PixelSrc> copyline(new PixelSrc[cy]);
            if (copyline == NULL) return E_OUTOFMEMORY;

            /// Do convolution column by column
            for (int x = 0; x < cx; x ++)
            {
                if(progress && (x % (cx/20+1) == 0))
                {
                    progress->SetValue(x, 0, cx);
                }

                ScanlinePtr<PixelDst> ptrDst(imDst.RowPtr(0) + x, imDst.Stride());
                CopyLine(copyline, imSrc.RowPtr(0) + x, imSrc.Stride(), cy);
                kernel.Convolve(ptrDst, &copyline[0], cy);
            }

            return S_OK;
        }

    } /// namespace impl

    using impl::ScanlinePtr;

    template<class FLT, class Traits>
    class ConvolutionKernel
    {
    public:
        ConvolutionKernel(int shift = 8) : m_anchor(0), m_shift(shift)
        {
        }

        void Reset()
        {
            m_anchor = 0;
            m_fltKernel.clear();
            m_intKernel.clear();
        }

        int KernelLength() const { return (int) m_fltKernel.size();}
        int KernelAnchor() const { return m_anchor;}

        template<class PixelDst, class PixelSrc>
        void Convolve(ScanlinePtr<PixelDst>& it, const PixelSrc* pSrc, size_t size)
        {
            const PaddingMethod padding = Traits::Padding;
            Helper<PixelDst, PixelSrc> helper(*this);

            /// Kernel [0 ... Anchor ... Length)
            /// Image  [0 ... head ... tail ... size)
            int head = min((int) size, KernelAnchor());
            int tail = max(0, (int) size - KernelLength() + KernelAnchor() + 1);

            /// if tail is ahead of head, move both to center
            if (tail < head)
            {
                tail = head = (tail + head) / 2;
            }

            /// the first segment in scanline
            for(int x = 0; x < head; x ++)
            {
                helper.ZeroSum();
                helper.Collect<padding, TRUE>(pSrc, x, (int) size);
                helper.Assign(*it);
                ++it;
            }

            /// the middle segment of scanline
            for (int x = head; x < tail; x ++)
            {
                helper.ZeroSum();
                helper.CollectFast(pSrc, x);
                helper.Assign(*it);
                ++it;
            }

            /// the last segment in scanline
            for(int x = tail; x < (int) size; x ++)
            {
                helper.ZeroSum();
                helper.Collect<padding, FALSE>(pSrc, x, (int) size);
                helper.Assign(*it);
                ++it;
            }
        }

        /// normalize the kernel and generate integer version
        /// i is the normalize order; 0, 1, or 2. If negative, ignore normalize
        HRESULT SetKernel(FLT* kernel, int length, int anchor, int normalizeOrder)
        {
            m_anchor = anchor;

            /// Allocate floating point kernel
            m_fltKernel.resize(length);
            if ((int) m_fltKernel.size() != length)
            {
                Reset();
                return E_OUTOFMEMORY;
            }

            /// Allocating int kernel
            m_intKernel.resize(length);
            if ((int) m_intKernel.size() != length)
            {
                Reset();
                return E_OUTOFMEMORY;
            }

            /// Copy the floating point kernel
            for (int k = 0; k < length; k ++)
            {
                m_fltKernel[k] = kernel[k];
            }

            /// Normalize the floating point kernel, if necessary
            if (normalizeOrder >= 0 && normalizeOrder <= 2)
            {
                m_fltKernel.Normalize(normalizeOrder);
            }

            /// Copy it to integer kernel, according to shift
            for(int k = 0; k < length; k ++)
            {
                m_intKernel[k] = xtl::RealCast<INT>(ldexp(m_fltKernel[k], m_shift));
            }

            return S_OK;
        }

    private:
        int m_shift;    /// can only be set in ctor
        int m_anchor;
        typedef vtl::VectorDynamic<FLT> FltKernelType;
        typedef vtl::VectorDynamic<INT> IntKernelType;
        FltKernelType m_fltKernel;
        IntKernelType m_intKernel;

    private:

#pragma region ---- Class Helper algorithms

        /// Convolve one pixel using given three pixels
        /// Dst is located at position of p1, p0 is previous pixel, p2 is next pixel
        template<class PixelDst, class PixelSrc>
        struct Helper
        {
            typedef PixelTraits<PixelDst> TraitsDst;
            typedef PixelTraits<PixelSrc> TraitsSrc;
            typedef typename TraitsSrc::ChannelType ChannelSrc;
            typedef typename TraitsDst::ChannelType ChannelDst;
            typedef typename xtl::TypeTraits<ChannelSrc>::AccumType AccumType;

            const static bool IntegerKernel = xtl::TypeTraits<ChannelSrc>::IsInteger;

            COMPILE_TIME_ASSERT(TraitsDst::ColorChannelNum == TraitsSrc::ColorChannelNum);
            const static bool IncludeAlpha =
                Traits::IncludeAlpha && TraitsDst::HasAlpha && TraitsSrc::HasAlpha;
            const static size_t ChannelNum =
                IncludeAlpha ? TraitsSrc::ChannelNum : TraitsSrc::ColorChannelNum;

            Helper(const ConvolutionKernel& kernel)
                : m_collector(kernel)
                , m_length(kernel.KernelLength())
                , m_anchor(kernel.KernelAnchor())                
            {
                COMPILE_TIME_ASSERT(TraitsDst::ColorChannelNum == TraitsSrc::ColorChannelNum);
            }

            template<PaddingMethod padding, BOOL Backward>
            void Collect(const PixelSrc* pSrc, int pos, int width)
            {
                typedef impl::IndexWrapper<padding, Backward> Wrapper;
                for (int i = 0; i < m_length; i ++)
                {
                    const int x = Wrapper::Wrap(i + pos - m_anchor, width);
                    m_collector.CollectOnePixel(m_sum, i, (const ChannelSrc*) &pSrc[x]);
                }
            }

            template<>
            void Collect<PaddingZero, TRUE>(const PixelSrc* pSrc, int pos, int width)
            {
                CollectZeroPadding(pSrc, pos, width);
            }

            template<>
            void Collect<PaddingZero, FALSE>(const PixelSrc* pSrc, int pos, int width)
            {
                CollectZeroPadding(pSrc, pos, width);
            }

            void CollectZeroPadding(const PixelSrc* pSrc, int pos, int width)
            {
                const int begin = max(0, pos - m_anchor);
                const int end = min(width, m_length + pos - m_anchor);
                for (int x = begin; x < end; x ++)
                {
                    const int i = x - pos + m_anchor;
                    m_collector.CollectOnePixel(m_sum, i, (const ChannelSrc*) &pSrc[x]);
                }
            }

            /// fast algorithm ensuring x is always inside scanline [0, width)
            void CollectFast(const PixelSrc* pSrc, int pos)
            {
                for (int i = 0; i < m_length; i ++)
                {
                    int x = i + pos - m_anchor;
                    m_collector.CollectOnePixel(m_sum, i, (const ChannelSrc*) &pSrc[x]);
                }
            }

            /// make the sum pixel zero
            void ZeroSum()
            {
                for(int k = 0; k < ChannelNum; k ++)
                {
                    m_sum[k] = 0;
                }
            }

            /// assign the sum to dst pixel
            void Assign(PixelDst& dst)
            {
                m_collector.AssignOnePixel((ChannelDst*) &dst, m_sum);
            }

        private:

#pragma region ---- Class Collector<bool IntegerKernel>
            /// collect source pixel and assign dst pixel using proper kernel
            template<bool IntegerKernel> class Collector;

            /// using integer kernal
            template<>
            class Collector<true>
            {
            public:
                Collector(const ConvolutionKernel& kernel)
                    : m_intKernel(kernel.m_intKernel), m_shift(kernel.m_shift)
                {
                }

                void CollectOnePixel(AccumType* sum, int kernelIndex, const ChannelSrc* pcSrc)
                {
                    for(int c = 0; c < ChannelNum; c++)
                    {
                        sum[c] += xtl::RealCast<AccumType>(m_intKernel[kernelIndex] * pcSrc[c]);
                    }
                }

                void AssignOnePixel(ChannelDst* pcDst, const AccumType* sum)
                {
                    for(int c = 0; c < ChannelNum; c++)
                    {
                        pcDst[c] = xtl::RealCast<ChannelDst>(sum[c] >> m_shift);
                    }
                }

            private:
                IntKernelType m_intKernel;
                int m_shift;
            };

            /// using floating point kernal
            template<>
            class Collector<false>
            {
            public:
                Collector(const ConvolutionKernel& kernel)
                    : m_fltKernel(kernel.m_fltKernel)
                {
                }

                void CollectOnePixel(AccumType* sum, int kernelIndex, const ChannelSrc* pcSrc)
                {
                    for(int c = 0; c < ChannelNum; c++)
                    {
                        sum[c] += xtl::RealCast<AccumType>(m_fltKernel[kernelIndex] * pcSrc[c]);
                    }
                }

                void AssignOnePixel(ChannelDst* pcDst, const AccumType* sum)
                {
                    for(int c = 0; c < ChannelNum; c++)
                    {
                        pcDst[c] = xtl::RealCast<ChannelDst>(sum[c]);
                    }
                }

            private:
                const FltKernelType& m_fltKernel;
            private:
                Collector(const Collector&);
                Collector& operator = (const Collector&);
            };
#pragma endregion

        private:
            Collector<IntegerKernel> m_collector;
            const int m_length, m_anchor;
            AccumType m_sum[ChannelNum];

        private:
            Helper(const Helper&);    
            Helper& operator = (const Helper&);    
        };
#pragma endregion

    };

    template<class FLT, class Traits>
    HRESULT BuildGaussianKernel(ConvolutionKernel<FLT, Traits>& kernel, FLT sigma)
    {
        COMPILE_TIME_ASSERT(xtl::TypeTraits<FLT>::IsReal);

        const static FLT pi = (FLT) 3.14159265358979323846;
        const int half = xtl::CeilCast<int>(sigma * 3);
        if (half <= 0) return E_INVALIDARG;

        const size_t size = 2 * half + 1;
        
        std::vector<FLT> data;
        data.resize(size);
        if (data.size() != size)
        {
            kernel.Reset();
            return E_OUTOFMEMORY;
        }

        const FLT s2 = sigma * sigma;
        const FLT denorm = 1 / (2 * s2);

        for (int k = 0; k < (int)size; k ++)
        {
            const FLT x = FLT(k) - FLT(half);
            data[k] = exp(- x * x * denorm);
        }

        return kernel.SetKernel(&data[0], (int) size, half, 1);
    }

    template<class Traits>
    class FixedKernel_121
    {
    public:
        /// PixelDst ScanlinePtr, which stores the output data
        /// PixelSrc is always fed as array of given size
        /// size is the width of the scanline
        template<class PixelDst, class PixelSrc>
        void Convolve(ScanlinePtr<PixelDst>& it, const PixelSrc* pSrc, size_t size)
        {
            const PaddingMethod padding = Traits::Padding;
            typedef Helper<PixelDst, PixelSrc> Helper;

            /// the first pixel in scanline
            Helper::Convolve<padding, TRUE>(*it, pSrc, 0, (int) size);
            ++it;

            /// the middle segment of scanline
            for (size_t x = 1; x < size - 1; x ++)
            {
                Helper::ConvolveFast(*it, pSrc + x);
                ++it;
            }

            /// the last pixel in scanline
            Helper::Convolve<padding, FALSE>(*it, pSrc, (int) size - 1, (int) size);
        };

    private:

#pragma region ---- Class Helper

        /// Convolve one pixel using given three pixels
        /// Dst is located at position of c, p is previous pixel, q is next pixel
        /// Typical pixel order is p, c, q
        template<class PixelDst, class PixelSrc>
        struct Helper
        {
            typedef PixelTraits<PixelDst> TraitsDst;
            typedef PixelTraits<PixelSrc> TraitsSrc;
            typedef typename TraitsSrc::ChannelType ChannelSrc;
            typedef typename TraitsDst::ChannelType ChannelDst;
            typedef typename xtl::TypeTraits<ChannelSrc>::AccumType AccumType;

            COMPILE_TIME_ASSERT(TraitsDst::ColorChannelNum == TraitsSrc::ColorChannelNum);
            const static bool IncludeAlpha =
                Traits::IncludeAlpha && TraitsDst::HasAlpha && TraitsSrc::HasAlpha;
            const static size_t ChannelNum =
                IncludeAlpha ? TraitsSrc::ChannelNum : TraitsSrc::ColorChannelNum;

            /// No pixel of, p, c, q are outside if image range.
            static void ConvolveFast(PixelDst& dst, const PixelSrc* pSrc)
            {
                ChannelDst* pd = (ChannelDst*) &dst;
                ChannelSrc* pp = (ChannelSrc*) &pSrc[-1];
                ChannelSrc* pc = (ChannelSrc*) &pSrc[0];
                ChannelSrc* pq = (ChannelSrc*) &pSrc[1];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pp[i] + pc[i] * 2 + pq[i];
                    pd[i] = xtl::RealCast<ChannelDst>(sum / 4);
                }
            }

            template<PaddingMethod padding, BOOL Backward>
            static void Convolve(PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                typedef impl::IndexWrapper<padding, Backward> Wrapper;
                ChannelDst* pd = (ChannelDst*) &dst;
                ChannelSrc* pp = (ChannelSrc*) &pSrc[Wrapper::Wrap(x-1, width)];
                ChannelSrc* pc = (ChannelSrc*) &pSrc[x];
                ChannelSrc* pq = (ChannelSrc*) &pSrc[Wrapper::Wrap(x+1, width)];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pp[i] + pc[i] * 2 + pq[i];
                    pd[i] = xtl::RealCast<ChannelDst>(sum / 4);
                }
            }

            template<>
            static void Convolve<PaddingZero, TRUE>(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ConvolveZeroPadding(dst, pSrc, x, width);
            }

            template<>
            static void Convolve<PaddingZero, FALSE>(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ConvolveZeroPadding(dst, pSrc, x, width);
            }

            static void ConvolveZeroPadding(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ChannelDst* pd = (ChannelDst*) &dst;
                ChannelSrc* pp = (ChannelSrc*) &pSrc[x-1];
                ChannelSrc* pc = (ChannelSrc*) &pSrc[x];
                ChannelSrc* pq = (ChannelSrc*) &pSrc[x+1];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pc[i] * 2;
                    if (x-1 >= 0) sum += pp[i];
                    if (x+1 < width) sum += pq[i];
                    pd[i] = xtl::RealCast<ChannelDst>(sum / 4);
                }
            }
        };

#pragma endregion

    };

    template<class Traits>
    class FixedKernel_14641
    {
    public:
        /// PixelDst ScanlinePtr, which stores the output data
        /// PixelSrc is always fed as array of given size
        /// size is the width of the scanline
        template<class PixelDst, class PixelSrc>
        void Convolve(ScanlinePtr<PixelDst>& it, const PixelSrc* pSrc, size_t size)
        {
            const PaddingMethod padding = Traits::Padding;
            typedef Helper<PixelDst, PixelSrc> Helper;

            /// the first pixel in scanline
            Helper::Convolve<padding, TRUE>(*it, pSrc, 0, (int) size);
            ++it;

            /// the second pixel in scanline
            Helper::Convolve<padding, TRUE>(*it, pSrc, 1, (int) size);
            ++it;

            /// the middle segment of scanline
            for (size_t x = 2; x < size - 2; x ++)
            {
                Helper::ConvolveFast(*it, pSrc + x);
                ++it;
            }

            /// the second last pixel in scanline
            Helper::Convolve<padding, FALSE>(*it, pSrc, (int) size - 2, (int) size);
            ++it;

            /// the last pixel in scanline
            Helper::Convolve<padding, FALSE>(*it, pSrc, (int) size - 1, (int) size);
        };

    private:

#pragma region ---- Class Helper

        /// Convolve one pixel using given three pixels
        /// Dst is located at position of cc, p0, p1 is on left side, q0, q1 is on right side
        /// the pixel order is typically, p1, p0, cc, q0, q1
        template<class PixelDst, class PixelSrc>
        struct Helper
        {
            typedef PixelTraits<PixelDst> TraitsDst;
            typedef PixelTraits<PixelSrc> TraitsSrc;
            typedef typename TraitsSrc::ChannelType ChannelSrc;
            typedef typename TraitsDst::ChannelType ChannelDst;
            typedef typename xtl::TypeTraits<ChannelSrc>::AccumType AccumType;

            COMPILE_TIME_ASSERT(TraitsDst::ColorChannelNum == TraitsSrc::ColorChannelNum);
            const static bool IncludeAlpha =
                Traits::IncludeAlpha && TraitsDst::HasAlpha && TraitsSrc::HasAlpha;
            const static size_t ChannelNum =
                IncludeAlpha ? TraitsSrc::ChannelNum : TraitsSrc::ColorChannelNum;

            static void ConvolveFast(PixelDst& dst, const PixelSrc* pSrc)
            {
                ChannelDst* pdd = (ChannelDst*) &dst;
                ChannelSrc* pp1 = (ChannelSrc*) &pSrc[-2];
                ChannelSrc* pp0 = (ChannelSrc*) &pSrc[-1];
                ChannelSrc* pcc = (ChannelDst*) &pSrc[0];
                ChannelSrc* pq0 = (ChannelSrc*) &pSrc[+1];
                ChannelSrc* pq1 = (ChannelSrc*) &pSrc[+2];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pp1[i] + pp0[i] * 4 + pcc[i] * 6 + pq0[i] * 4 + pq1[i];
                    pdd[i] = xtl::RealCast<ChannelDst>(sum / 16);
                }
            }

            template<PaddingMethod padding, BOOL Backward>
            static void Convolve(PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                typedef impl::IndexWrapper<padding, Backward> Wrapper;
                ChannelDst* pd = (ChannelDst*) &dst;
                ChannelSrc* pp1 = (ChannelSrc*) &pSrc[Wrapper::Wrap(x-2, width)];
                ChannelSrc* pp0 = (ChannelSrc*) &pSrc[Wrapper::Wrap(x-1, width)];
                ChannelSrc* pcc = (ChannelDst*) &pSrc[x];
                ChannelSrc* pq0 = (ChannelSrc*) &pSrc[Wrapper::Wrap(x+1, width)];
                ChannelSrc* pq1 = (ChannelSrc*) &pSrc[Wrapper::Wrap(x+2, width)];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pp1[i] + pp0[i] * 4 + pcc[i] * 6 + pq0[i] * 4 + pq1[i];
                    pd[i] = xtl::RealCast<ChannelDst>(sum / 16);
                }
            }

            template<>
            static void Convolve<PaddingZero, TRUE>(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ConvolveZeroPadding(dst, pSrc, x, width);
            }

            template<>
            static void Convolve<PaddingZero, FALSE>(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ConvolveZeroPadding(dst, pSrc, x, width);
            }

            static void ConvolveZeroPadding(
                PixelDst& dst, const PixelSrc* pSrc, int x, int width)
            {
                ChannelDst* pd = (ChannelDst*) &dst;
                ChannelSrc* pp1 = (ChannelSrc*) &pSrc[x-2];
                ChannelSrc* pp0 = (ChannelSrc*) &pSrc[x-1];
                ChannelSrc* pcc = (ChannelDst*) &pSrc[x];
                ChannelSrc* pq0 = (ChannelSrc*) &pSrc[x+1];
                ChannelSrc* pq1 = (ChannelSrc*) &pSrc[x+2];

                for (size_t i = 0; i < ChannelNum; i ++)
                {
                    AccumType sum = pcc[i] * 6;
                    if (x-2 >= 0) sum += pp1[i];
                    if (x-1 >= 0) sum += pp0[i] * 4;
                    if (x+1 < width) sum += pq0[i] * 4;
                    if (x+2 < width) sum += pq1[i];
                    pd[i] = xtl::RealCast<ChannelDst>(sum / 16);
                }
            }
        };

#pragma endregion

    };

    template<MorphologicalOperation operation, class Traits>
    class MorphologicalKernel
    {
    public:
        MorphologicalKernel(size_t scale) : m_iScale (scale)
        {
            COMPILE_TIME_ASSERT(operation == Dilation || operation == Erosion);
        }

        /// PixelDst ScanlinePtr, which stores the output data
        /// PixelSrc is always fed as array of given size
        /// size is the width of the scanline
        template<class PixelType>
        void Convolve(ScanlinePtr<PixelType>& it, const PixelType* pSrc, size_t size)
        {
            typedef PixelTraits<PixelType> PixelTraits;
            typedef typename PixelTraits::ChannelType ChannelType;
            const static size_t ChannelNum =
                Traits::IncludeAlpha ? PixelTraits::ChannelNum : PixelTraits::ColorChannelNum;

            /// operator related to current operation: Erode or Dilate
            typedef GrayLevelMorphologicalOperator<operation> OP;

            const INT xhead = (INT)m_iScale / 2;
            const INT xtail = (INT)m_iScale - xhead;  /// inclusive

            for (INT x = 0; x < (INT) size; x ++)
            {
                INT xfirst = __max(x - xhead, 0);
                INT xlast  = __min(x + xtail, (INT) size - 1);

                /// copy the last pixel for initialization
                *it = pSrc[xlast];

                ChannelType* pcDst = (ChannelType*) &(*it); /// channel pointer dst

                /// for each pixel inside range
                for (INT x = xfirst; x < xlast; x ++)
                {
                    const ChannelType* pcSrc = (const ChannelType*)(&pSrc[x]);

                    /// for each channel, loop will be expanded in compile time
                    for (size_t i = 0; i < ChannelNum; i ++)
                    {
                        OP::ComputeOneResult(pcDst[i], pcSrc[i]);
                    }
                }

                ++it;
            }
        };

    private:
        size_t m_iScale;

    private:
        template<MorphologicalOperation operation>
        struct GrayLevelMorphologicalOperator
        {
            template<class ChannelType>
            static void ComputeOneResult(ChannelType& dst, const ChannelType& src)
            {
                COMPILE_TIME_ASSERT(operation == Dilation);
                if(dst < src) dst = src;    /// dilation, find the max in window
            }
        };

        template<>
        struct GrayLevelMorphologicalOperator<Erosion>
        {
            template<class ChannelType>
            static void ComputeOneResult (ChannelType& dst, const ChannelType& src)
            {
                COMPILE_TIME_ASSERT(operation == Erosion);
                if(dst > src) dst = src;    /// erosion, find the min in window
            }
        };
    };

    /// Binary morphological kernel handles only one channel without alpha channel
    /// This kernel only works when dst are copied from src
    /// If convolution is done separately H and V, copy once is enough
    /// If convolution is done on image its self, ignore copying.
    template<MorphologicalOperation operation, class Traits>
    class BinaryMorphologicalKernel
    {
    public:
        BinaryMorphologicalKernel(size_t scale) : m_iScale (scale)
        {
            COMPILE_TIME_ASSERT(operation == Dilation || operation == Erosion);
        }

        /// PixelDst ScanlinePtr, which stores the output data
        /// PixelSrc is always fed as array of given size
        /// size is the width of the scanline
        void Convolve(ScanlinePtr<BYTE>& it, const BYTE* pSrc, size_t size)
        {
            COMPILE_TIME_ASSERT(!Traits::IncludeAlpha);

            /// the window is [x - xhead, x + xtail), size = 2 * scale + 1
            /// .....[ head ] x [ tail ), xtail is exclusive
            const int xhead = (int) m_iScale;
            const int xtail = (int) m_iScale + 1;

            /// operator related to current operation: Erode or Dilate
            typedef BinaryMorphologicalOperator<operation> OP;

            /// One is the 1 in binary image, opposite to zero
            const BYTE One = OP::One;

            /// The value out side of image,
            /// If dilating, assume all outside pixels are zero
            /// while eroding, assume all outside pixels are ones
            const BYTE Outside = OP::Outside;

            /// All is the value when the window are all Ones
            const int All = One * (xhead + xtail);

            /// Initialize window sum. in window [-xhead, xtail)
            int wsum = xhead * Outside;
            for (int x = 0; x < xtail; x ++)
            {
                wsum += pSrc[x];
            }

            /// the pointer always pointing to the next pixel to be added to window
            const BYTE* pTail = pSrc + xtail;

            /// when prediction is same as tail, which should be added
            /// we don't need to add it since it will not change the sum
			int prediction = ((wsum==0) ? 0 : (wsum == All ? One : -1));

            /// when window overlap (x < 0) part.
            for (int x = 0; x < xhead; x ++)
            {
				if( prediction != (*pTail) )
                {
					OP::ComputeOneResult(*it, wsum, All);
					MoveWindow(wsum, Outside, (*pTail));
                    prediction = Predict(wsum, All, One);
                }
                
                ++ pTail;
                ++ it;
            }

            /// the pointer always pointing to the next pixel to be removed from window
            const BYTE* pHead = pSrc;

            /// in this range, window will never touch outside
            for (int x = xhead; x < (int) size - xtail; x ++)
            {
                if( prediction != (*pTail) )
                {
                    OP::ComputeOneResult(*it, wsum, All);
					MoveWindow(wsum, (*pHead), (*pTail));
					prediction = Predict(wsum, All, One);
                }
                
                ++ pHead;
                ++ pTail;
                ++ it;
            }

            /// when window overlap (x >= size) part.
            for (int x = (int) size - xtail; x < (int) size; x ++)
            {
				if( prediction != Outside )
                {
                    OP::ComputeOneResult(*it, wsum, All);
					MoveWindow(wsum, (*pHead), Outside);
                    prediction = Predict(wsum, All, One);
                }

                ++ pHead;
                ++ it;
            }
        };

    private:
        size_t m_iScale;

    private:
        int Predict(int sum, int all, int one)
        {
            return ((sum == 0) ? 0 : (sum == all ? one : -1));
        }

        template<MorphologicalOperation operation>
        struct BinaryMorphologicalOperator
        {
            const static BYTE One = 255;
            const static BYTE Outside = 0;

            static void ComputeOneResult(BYTE& dst, int wsum, int /*all*/)
            {
                COMPILE_TIME_ASSERT(operation == Dilation);
                dst = (wsum == 0) ? 0 : One;
            }

        };

        template<>
        struct BinaryMorphologicalOperator<Erosion>
        {
            const static BYTE One = 255;
            const static BYTE Outside = One;

            static void ComputeOneResult(BYTE& dst, int wsum, int all)
            {
                COMPILE_TIME_ASSERT(operation == Erosion);
                dst = (wsum == all) ? One : 0;
            }
        };

        /// every time we move the window, 
        /// remove one pixel at head, and add new pixel at tail
        void MoveWindow(int& wsum, BYTE head, BYTE tail) const
        {
            wsum -= head;
            wsum += tail;
        }
    };


} /// namespace img