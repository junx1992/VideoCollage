/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Vis Lib Image interface
  
Abstract:
    1. An class for image pixel access and memory management
    2. Access image by Pixel(int x, int y)(ShortXY)(PointF2)(POINT)
    4. Allocate as continuous row-major memory block, as in Gdiplus::Bitmap
    5. Support BYTE, RGBA, float, int, and many other pixel format
    6. IImage is for untyped pixel interface, CImage<T> is for pixel type related
    7. Use Pimpl pattern for image data and displayable image.
    
Notes:
    Pimpl pattern advantages.
    1. Separate interface with implementation. (Not used here).
    2. Use internal shared_ptr, so as to be used in STL container.
    3. With equal and less operator, it can be used in associated container.
    4. DisplayImagePolicy also uses Pimpl pattern for less memory usage.

Usage:
        
History:
    Created  on 2003 Aug. 16 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "xtl/Xtl_Pointer.hpp"
#include "xtl/Xtl_Memory.hpp"

#include "Img_Utility.h"
#include "Img_Pixel.h"

#include <vector>


/// Define the maximum image dimension to avoid memory overflow
/// The maximum image size cannot be larger than (MAX_IMAGE_WIDTH * MAX_IMAGE_WIDTH)
/// And any dimension of the image cannot be larger than MAX_IMAGE_WIDTH
/// The default is 0x7fff, which is max of short value (15bits), so that ShortXY is availible.
#ifndef MAX_IMAGE_WIDTH
#define MAX_IMAGE_WIDTH     (0x7fff)
#endif


namespace img
{
    template<class> class CImage;

    typedef CImage<PixelByte>      CImageByte;
    typedef CImage<PixelWord>      CImageWord;
    typedef CImage<PixelShort>     CImageShort;
    typedef CImage<PixelInt>       CImageInt;
    typedef CImage<PixelFloat>     CImageFloat;
    typedef CImage<PixelDouble>    CImageDouble;

    typedef CImage<PixelArgb>      CImageArgb;
    typedef CImage<PixelRgb>       CImageRgb;

    typedef CImage<PixelArgbByte>  CImageArgbByte;
    typedef CImage<PixelRgbByte>   CImageRgbByte;

    typedef CImage<PixelRgbWord>   CImageRgbWord;
    typedef CImage<PixelRgbFloat>  CImageRgbFloat;
    typedef CImage<PixelRgbDouble> CImageRgbDouble;

    typedef CImage<ShortXY>        CImageShortXY;
    typedef CImage<PixelHsv>       CImageHsv;
    typedef CImage<PixelLab>       CImageLab;

    /// Acyclic Visitor Pattern for image interface
    /// Usage:
    ///      SomeVisitor visitor;
    ///      std::vector<IImage*> rgImages;
    ///      for(int k = 0; k < rgImages.size(); k ++)
    ///          rgImage[k]->Accept(visitor);
    /// For a complete example,
    ///      please reference QueryPixelInfo in utl/Utl_Displayimage.cpp
    class IImage;
    class IImageVisitor
    {
    public:
        virtual HRESULT Visit(const IImage*) const = 0;
    };

#pragma region -- Implementation of Native image defination
    namespace impl
    {
        /// Image access policy, for aligned memory allocation and basic image operation
        /// It uses Pimple pattern, associate real implementation ImageData
        ///     through a SharedPtr, for shallow copy and assignment of image object
        class NativeImage
        {
            const static size_t uAlignment = 16;    /// aligned scan-lines for SSE2, 128bits
            typedef xtl::AlignedAllocator<uAlignment> MemoryBlockAllocator;

        public:
            /// access the image dimension, width and height in pixels
            /// stride in bytes for each scanline.
            /// each scanline is aligned according to uAlignment
            INT Width()  const { return m_iWidth;}
            INT Height() const { return m_iHeight;}
            INT Stride() const { return m_iStride;}     /// Stride() >= sizeofPixel * Width()

            /// if the image has data or not
            bool IsEmpty() const { return m_pData == NULL;}

            /// access the pointer of the y-th scanline
            void* RowPtr(INT y) { return m_pData + m_iStride * y;}
            const void* RowPtr(INT y) const { return m_pData + m_iStride * y;}

            /// allocate the image according to given size
            /// if it has same size memory, do not allocate twice
            /// also allocate a small block for scanline pointers
            /// assume continuous mem-block, scanline-based storage
            /// each scanline/row is uAlignment byte aligned
            HRESULT AllocateImage(INT cx, INT cy, INT sizeofPixel);

            /// Bind the given memory, will never delete given pBuffer
            HRESULT AllocateImage(INT cx, INT cy, INT sizeofPixel, INT stride, void* pBuffer);

            /// de-allocate memory, and reset states, never fail
            void DeallocateImage();

        public:
            /// protected ctor, only available for CImage
            NativeImage() :
                m_pData(NULL), m_fOwnMemory(false),
                m_iWidth(0), m_iHeight(0), m_iStride(0) {}

            /// clear memory in dtor
            virtual ~NativeImage() { DeallocateImage();}

        private:    /// should not be used even for child class
            INT m_iWidth;           /// the image width in pixels
            INT m_iHeight;          /// the image height in pixels
            INT m_iStride;          /// the offset of pixels, &row[y+1] - &row[y]
            BYTE* m_pData;          /// the memory block pointer
            bool m_fOwnMemory;      /// if this object own the memory and should delete it
        };

    } // end namespace impl
#pragma endregion

    /// Abstract interface for image, for pixel type unrelated operations
    class IImage
    {
    public:
        /// virtual dtor, make sure the correct dtor is called from IImage*
        virtual ~IImage() {}

        /// clone a copy of the current image
        virtual IImage* Clone() const = 0;

        /// Visitor Pattern for image interface
        virtual HRESULT Accept(IImageVisitor& visitor) const = 0;

        /// ---------------------- Image information related -------------------------------

        /// get the image pixel format, in PixelFormat
        virtual Gdiplus::PixelFormat GetPixelFormat() const = 0;

        /// get the size of each pixel, in bytes
        virtual int GetPixelSize() const = 0;

        /// Get image data according to given rectangle
        /// if pRect == NULL, output whole image
        /// pRect will be rectified according to image size
        HRESULT GetBitmapData(
            Gdiplus::BitmapData* pData,
            const Gdiplus::Rect* pRect = NULL) const;

        /// Bind the image to given BitmapData
        HRESULT SetBitmapData(Gdiplus::BitmapData* data);

        /// Is this image sharing the same memory ptr as the given one?
        bool operator == (const IImage& src) const { return m_nativeBmp == src.m_nativeBmp;}

        /// Is this image not sharing the same memory ptr as the given one?
        bool operator != (const IImage& src) const { return m_nativeBmp != src.m_nativeBmp;}

        /// Less operator and equal operator enables IImage for associative container, such as set
        bool operator <  (const IImage& src) const { return m_nativeBmp <  src.m_nativeBmp;}

        /// Get a display image
        virtual const IImage* GetDisplayImage() const = 0;

        /// Reset the display image if image has been changed
        virtual void ResetDisplayImage() = 0;

        /// ---------------------- Image geometry related -------------------------------

        /// if the image contains no data, empty image
        bool IsEmpty() const { return m_nativeBmp == NULL || m_nativeBmp->IsEmpty();}
        bool IsValid() const { return !IsEmpty();}

        /// Number of pixels in a row / scanline
        int Width()  const { return (m_nativeBmp == NULL) ? 0 : m_nativeBmp->Width() ;}

        /// umber of pixels in a column, number of scanlines
        int Height() const { return (m_nativeBmp == NULL) ? 0 : m_nativeBmp->Height();}

        /// Number of bytes in a row, offset to access same pixel in next scanline
        int Stride() const { return (m_nativeBmp == NULL) ? 0 : m_nativeBmp->Stride();}

        /// access the size of image, in pixels
        const Gdiplus::Size GetSize() const
        {
            return (m_nativeBmp == NULL) ? Gdiplus::Size(0, 0) :
                Gdiplus::Size(m_nativeBmp->Width(), m_nativeBmp->Height());
        }

        /// access the visible rectangle of the image, in pixels
        const Gdiplus::Rect GetRect() const
        {
            return (m_nativeBmp == NULL) ? Gdiplus::Rect(0, 0, 0, 0) :
                Gdiplus::Rect(0, 0, m_nativeBmp->Width(), m_nativeBmp->Height());
        }

        /// ContainsPoint() check if given coordinate is inside image
        /// TX, TY can be int, float or double.
        /// if TX TY is int, they are considered as unsigned int
        /// TX and TY can be different.
        /// If any of them is real, corresponding range -= 1
        /// because we want both x and x+1 in range, for linear interpolation
        template<class TX, class TY>
        bool ContainsPoint(TX x, TY y) const
        {
            if(m_nativeBmp == NULL) return false;
            const bool xIsInt = xtl::TypeTraits<TX>::IsInteger;
            const bool yIsInt = xtl::TypeTraits<TY>::IsInteger;
            return InRange<TX, xIsInt>::Result(x, (UINT)m_nativeBmp->Width()) &&
                   InRange<TY, yIsInt>::Result(y, (UINT)m_nativeBmp->Height());
        }

        #define CONTAINSPOINT(P, PX, PY)                    \
            bool ContainsPoint(const P& pt) const           \
            {                                               \
                return ContainsPoint(pt.##PX, pt.##PY);     \
            }                                               \

        CONTAINSPOINT(POINT,    x, y);
        CONTAINSPOINT(ShortXY,  X, Y);
        CONTAINSPOINT(Gdiplus::PointF,   X, Y);
        CONTAINSPOINT(Gdiplus::Point,    X, Y);
        #undef CONTAINSPOINT

        /// ---------------------- Pixel access related -------------------------------

        /// Access the y-th scanline
        void* RowPtr(int y)
        {
            assert(m_nativeBmp != NULL && (UINT)y  < (UINT) Height());
            return m_nativeBmp->RowPtr(y);
        }

        /// Access the y-th scanline
        const void* RowPtr(int y) const
        {
            assert(m_nativeBmp != NULL && (UINT)y  < (UINT) Height());
            return m_nativeBmp->RowPtr(y);
        }

        /// Access the pointer to the pixel (x, y)
        void* PixelPtr(int x, int y)
        {
            return (void*) ((BYTE*) RowPtr(y) + GetPixelSize() * x);
        }

        /// Access the pointer to the pixel (x, y)
        const void* PixelPtr(int x, int y) const
        {
            return (const void*) ((const BYTE*) RowPtr(y) + GetPixelSize() * x);
        }

        /// ---------------------- Memory allocation related -------------------------------

        /// allocate the image copying the given image size
        HRESULT Allocate(const IImage& image)
        {
            const Gdiplus::Size size = image.GetSize();
            return Allocate(size.Width, size.Height);
        }

        /// Allocate the memory for proper pixel format and stride
        HRESULT Allocate(INT iWidth, INT iHeight)
        {
            if(iWidth == Width() && iHeight == Height()) return S_OK;
            GuaranteeImagePtrIsValid();
            ResetDisplayImage();
            return m_nativeBmp->AllocateImage(iWidth, iHeight, GetPixelSize());
        }

        /// Use the given memory buffer to construct the image
        HRESULT Allocate(INT iWidth, INT iHeight, INT iStride, VOID* pUserBuffer)
        {
            if (iWidth == Width() && iHeight == Height() && iStride == Stride() &&
                (!IsValid() || RowPtr(0) == pUserBuffer)) return S_OK;
            GuaranteeImagePtrIsValid();
            ResetDisplayImage();
            return m_nativeBmp->AllocateImage(
                iWidth, iHeight, GetPixelSize(), iStride, pUserBuffer);
        }

        /// free the memory of image data
        /// But still attaching to shared_ptr
        void Deallocate()
        {
            ResetDisplayImage();
            if (m_nativeBmp != NULL) m_nativeBmp->DeallocateImage();
        }

        void Swap(IImage& img)
        {
            assert(GetPixelSize() == img.GetPixelSize());
            SwapPtr(m_nativeBmp, img.m_nativeBmp);
        }

    protected:
        /// protected ctor, only derived class can use it
        explicit IImage() {}

        /// Copy ctor, copy shared_ptr and bind together.
        /// But if input image is NULL ptr, no binding
        IImage(const IImage& image)
        {
            m_nativeBmp = image.m_nativeBmp;
        }

        /// Copy operator, copy shared_ptr and bind together.
        /// But if input image is NULL ptr, no binding
        IImage& operator= (const IImage& image)
        {
            m_nativeBmp = image.m_nativeBmp;
            return *this;
        }

    private:
        template<class X, bool fIsInteger>
        struct InRange
        {
            static bool Result(X x, UINT cx)
            {
                COMPILE_TIME_ASSERT(xtl::TypeTraits<X>::IsInteger);
                return (UINT) x < cx;
            }
        };

        template<class X>
        struct InRange<X, false>
        {
            static bool Result(X x, UINT cx)
            {
                COMPILE_TIME_ASSERT(xtl::TypeTraits<X>::IsReal);
                return x >= 0 && x <= X(cx-1);
            }
        };

        void GuaranteeImagePtrIsValid() const
        {
            if (m_nativeBmp == NULL)
            {
                const_cast<IImage*>(this)->m_nativeBmp = new impl::NativeImage;
            }
        }

    private:
        /// Pimple pattern, to separate instance with interfaces
        xtl::SharedPtr<impl::NativeImage> m_nativeBmp;
    };

#pragma region Implementation of pixel Format mapping
    namespace impl
    {
        template<class ImageType> class ImageDisplayPolicy;

        template<class T> struct MappingPixelFormat
        { 
            const static Gdiplus::PixelFormat Result = PixelFormatUndefined;
        };

        #define MAPPING_PIXEL_FORMAT(TYPE, FORMAT)                              \
            template<> struct MappingPixelFormat<TYPE>                          \
            {                                                                   \
                const static Gdiplus::PixelFormat Result = FORMAT;              \
            }                                                                   \

        MAPPING_PIXEL_FORMAT(img::PixelByte,       PixelFormat8bppIndexed);
        MAPPING_PIXEL_FORMAT(img::PixelRgb,        PixelFormat24bppRGB);
        MAPPING_PIXEL_FORMAT(img::PixelArgb,       PixelFormat32bppARGB);
        MAPPING_PIXEL_FORMAT(img::PixelFloat,      PixelFormat32bppARGB);
        MAPPING_PIXEL_FORMAT(img::ShortXY,         PixelFormat32bppARGB);
        MAPPING_PIXEL_FORMAT(img::PixelInt,        PixelFormat32bppARGB);
        MAPPING_PIXEL_FORMAT(img::PixelArgbWord,   PixelFormat64bppARGB);
        MAPPING_PIXEL_FORMAT(img::PixelDouble,     PixelFormat64bppARGB);
        MAPPING_PIXEL_FORMAT(img::PixelLab,        PixelFormat48bppRGB);
        MAPPING_PIXEL_FORMAT(img::PixelHsv,        PixelFormat24bppRGB);

        #undef  MAPPING_PIXEL_FORMAT

    } /// namespace impl
#pragma endregion

#pragma region Implementation of ImageDisplayPolicy

    namespace impl
    {
        /// Display policy classs, Prepare display image according to pixel format
        /// Default policy does nothing, just return host image
        template<class ImageType>
        class ImageDisplayPolicy
        {
        public:
            static void ResetDisplayImage() {}
            static const IImage* GetDisplayImage(const ImageType& imHost) { return &imHost;}
        };

    }   /// namespace impl
#pragma endregion

    /// Typed image with given pixel type T
    template<class T>
    class CImage : public IImage
    {
    public:
        typedef typename PixelTraits<T>::PixelType      PixelType;
        typedef typename PixelTraits<T>::ChannelType    ChannelType;
        const static size_t ChannelNum  = PixelTraits<T>::ChannelNum;
        const static size_t PixelSize   = PixelTraits<T>::PixelSize;
        const static size_t ChannelSize = PixelTraits<T>::ChannelSize;

        class IVisitor
        {
        public:
            virtual HRESULT Visit(const CImage*) const = 0;
        };

    public:
        /// access the pointer of the y-th scanline
        /// This is not virtual function, intent to override
        T* RowPtr(INT y) { return (T*) IImage::RowPtr(y);}
        const T* RowPtr(INT y) const { return (const T*) IImage::RowPtr(y);}

        /// Visitor Pattern for image interface
        virtual HRESULT Accept(IImageVisitor& visitor) const
        {
            const IVisitor* thisVisitor = dynamic_cast<IVisitor*>(&visitor);
            return (thisVisitor == NULL) ? visitor.Visit(this) : thisVisitor->Visit(this);
        }

        virtual Gdiplus::PixelFormat GetPixelFormat() const
        {
            return impl::MappingPixelFormat<T>::Result;
        }

        virtual int GetPixelSize() const
        {
            return PixelSize;
        }

        virtual CImage* Clone() const
        {
            CImage * pNew = new CImage;
            if (pNew != NULL) pNew->Copy(*this);
            return pNew;
        }

        virtual const IImage* GetDisplayImage() const
        {
            return m_displayPolicy.GetDisplayImage(*this);
        }

        virtual void ResetDisplayImage()
        {
            m_displayPolicy.ResetDisplayImage();
        }

        /// Create a new image REFerencing to the given rectangle of this image
        /// The new image sharing the same memory block of current image
        /// If the given rectangle is out of image boundary, use intersecting rect
        const CImage SubImage(Gdiplus::Rect rc) const
        {
            CImage imResult;
            rc.Intersect(GetRect());
            if (!rc.IsEmptyArea())
            {
                imResult.Allocate(
                    rc.Width, rc.Height, Stride(), (void*) (RowPtr(rc.Y) + rc.X));
            }
            return imResult;
        }

        const CImage SubImage(INT X, INT Y, INT iWidth, INT iHeight) const
        {
            return SubImage(Rect(X, Y, iWidth, iHeight));
        }

        /// copy the content from source image, always override old size and data
        /// allows copying from different pixel types
        /// as long as you define according PixelAssignment function
        template<class S>
        HRESULT Copy(const CImage<S>& src, Gdiplus::Rect& rcSrc)
        {
            /// prevent copy from self
            if ((*this) == src) return S_OK;
            return Copy(src.SubImage(rcSrc));
        }

        template<class S>
        HRESULT Copy(const CImage<S>& src)
        {
            /// prevent copy from self
            if ((*this) == src) return S_OK;

            /// allocate new image
            HRESULT hr = Allocate(src.Width(), src.Height());
            if (FAILED(hr)) return hr;

            /// copy pixels
            return CopyPixels(src);
        }

        /// assume the memory has been allocated, only copy pixels
        template<class S>
        HRESULT CopyPixels(const CImage<S>& src)
        {
            if (IsEmpty()) return E_INVALIDARG;

            ResetDisplayImage();
            return impl::CopyImageData(
                Gdiplus::Point(0, 0), RowPtr(0), Stride(),
                src.GetRect(), src.RowPtr(0), src.Stride());
        }

        template<class S>
        HRESULT CopyPixels(const CImage<S>& src, Gdiplus::Point ptDst, Gdiplus::Rect rcSrc)
        {
            if (IsEmpty()) return E_INVALIDARG;

            Gdiplus::Rect rcCopy(0, 0, rcSrc.Width, rcSrc.Height);

            /// Get dst rectangle relative to rcCopy
            Gdiplus::Rect rcDstImage = GetRect();
            rcDstImage.Offset(-ptDst.X, -ptDst.Y);

            /// Get src rectangle relative to rcCopy
            Gdiplus::Rect rcSrcImage = src.GetRect();
            rcSrcImage.Offset(-rcSrc.X, -rcSrc.Y);

            /// Find the intersecting rectangle to copy
            rcCopy.Intersect(rcSrcImage);
            rcCopy.Intersect(rcDstImage);

            /// if it's empty, we are success
            if (rcCopy.IsEmptyArea()) return S_OK;

            /// move rectangle back to dst image coordinate
            rcDstImage = rcCopy;
            rcDstImage.Offset(ptDst.X, ptDst.Y);

            /// move rectangle back to src image coordinate
            rcSrcImage = rcCopy;
            rcSrcImage.Offset(rcSrc.X, rcSrc.Y);

            /// Update the dst point
            ptDst.X = rcDstImage.X;
            ptDst.Y = rcDstImage.Y;

            /// Finally, copy image data in the copy rectangle
            ResetDisplayImage();
            return impl::CopyImageData(
                ptDst, RowPtr(0), Stride(), rcSrcImage, src.RowPtr(0), src.Stride());
        }

        /// clear pixels to zero
        /// assumes zero memory is zero pixels
        void ClearPixels()
        {
            if (IsEmpty()) return;
            const INT cx = Width();
            const INT cy = Height();

            for (int y = 0; y < cy; y ++)
            {
                memset(RowPtr(y), 0, PixelSize * cx);
            }
            ResetDisplayImage();
        }

        /// Fill the image by give value
        void FillPixels(const PixelType value)
        {
            if (IsEmpty()) return;
            const INT cx = Width();
            const INT cy = Height();

            for (int y = 0; y < cy; y ++)
            {
                PixelType* p = RowPtr(y);
                for (int x = 0; x < cx; x ++)
                {
                    (*p++) = value;
                }
            }
            ResetDisplayImage();
        }

        /// fill the given channel with given value
        void FillChannel(ChannelIndex iChannel, const ChannelType value)
        {
            assert(iChannel < ChannelNum);

            if (IsEmpty()) return;
            const INT cx = Width();
            const INT cy = Height();

            for (int y = 0; y < cy; y ++)
            {
                ChannelType* p = ((ChannelType*) RowPtr(y)) + iChannel;
                for (int x = 0; x < cx; x ++)
                {
                    (*p) = value;
                    p += ChannelNum;
                }
            }
            ResetDisplayImage();
        }

        /// Access pixel using Pixel(x, y) function
        T& Pixel(int x, int y)
        {
            assert((UINT)x < (UINT)Width());
            return RowPtr(y)[x];
        }
        const T& Pixel(int x, int y) const
        {
            assert((UINT)x < (UINT)Width());
            return RowPtr(y)[x];
        }

        /// Access pixel using () operator
        T& operator()(int x, int y) { return Pixel(x, y); }
        const T& operator()(int x, int y) const { return Pixel(x, y);}

        /// Access pixel by various of point structures
        #define ACCESS_PIXEL_BY_XY(P, PX, PY)                                       \
            T& Pixel(const P& pt)                 { return Pixel(pt.##PX, pt.##PY);}\
            T& operator()(const P& pt)            { return Pixel(pt.##PX, pt.##PY);}\
            const T& Pixel(const P& pt) const     { return Pixel(pt.##PX, pt.##PY);}\
            const T& operator()(const P& pt)const { return Pixel(pt.##PX, pt.##PY);}\

        ACCESS_PIXEL_BY_XY(POINT,   x, y);
        ACCESS_PIXEL_BY_XY(ShortXY, X, Y);
        ACCESS_PIXEL_BY_XY(Gdiplus::Point,   X, Y);
        #undef ACCESS_PIXEL_BY_XY

        /// Arithmetic operator, pixel by pixel, to a scalar
        #define AOP(OP)                                 \
        void operator OP(T scalar)                      \
        {                                               \
            const int cx = Width();                     \
            const int cy = Height();                    \
            for (int y = 0; y < cy; y ++)               \
            {                                           \
                T* p = RowPtr(y);                       \
                for (int x = 0 ; x < cx; x ++)          \
                {                                       \
                    (*p++) OP scalar;                   \
                }                                       \
            }                                           \
        }                                               \

        /// Usage: Image *= scale;
        AOP(+=) AOP(-=) AOP(*=) AOP(/=)
        AOP(&=) AOP(|=) AOP(^=)
        AOP(>>=) AOP(<<=)
        #undef AOP

        #define IOP(OP)                                 \
        void operator OP(const CImage& src)             \
        {                                               \
            if (GetSize() != src.GetSize()) return;     \
            const int cx = Width();                     \
            const int cy = Height();                    \
            for (int y = 0; y < cy; y ++)               \
            {                                           \
                T* pdst = RowPtr(y);                    \
                const T* psrc = src.RowPtr(y);          \
                for (int x = 0 ; x <cx; x ++)           \
                {                                       \
                    (*pdst++) OP (*psrc++);             \
                }                                       \
            }                                           \
        }                                               \

        /// Usage: imgDst *= imgSrc;
        IOP(+=) IOP(-=) IOP(*=) IOP(/=)
        IOP(&=) IOP(|=) IOP(^=)
        IOP(>>=) IOP(<<=)
        #undef IOP

    private:
        impl::ImageDisplayPolicy<CImage> m_displayPolicy;
    };

#pragma region -- Implementation of Image Display Proxy

    namespace impl
    {

        /// Display image as a proxy image, typically display real image as byte image
        /// Derive your own ImageDisplayPolicy from this proxy class
        template<class ProxyImageType, class HostImageType>
        class ImageDisplayProxy
        {
        public:
            ImageDisplayProxy() : m_proxy(NULL) {}

            void ResetDisplayImage()
            {
                if(m_proxy != NULL) m_proxy->m_fDirty = true;
            }

            const IImage* GetDisplayImage(const HostImageType& imHost) const
            {
                return const_cast<ImageDisplayProxy*>(this)->GetDisplayImageImpl(imHost);
            }

        private:
            const IImage* GetDisplayImageImpl(const HostImageType& imHost)
            {
                if (imHost.IsEmpty()) return NULL;

				if (m_proxy == NULL || m_proxy->m_fDirty)
				{
                    m_proxy = new ProxyImpl;
                    if (m_proxy != NULL)
                    {
                        PrepareDisplayImage<ProxyImageType>(m_proxy->m_displayImage, imHost);
                    }
				}

                if (m_proxy != NULL)
                {
                    m_proxy->m_fDirty = m_proxy->m_displayImage.IsEmpty();
                    return &m_proxy->m_displayImage;
                }
                else
                {
                    return NULL;
                }
            }

        private:
            struct ProxyImpl
            {
                ProxyImpl() : m_fDirty(true) {}
                ProxyImageType m_displayImage;
                bool m_fDirty;
            };

            /// Pimpl pattern for display proxy
            xtl::SharedPtr<ProxyImpl> m_proxy;

        private:
            /// Prepare display image for display policy classes
            template<class ImageProxy>
            void PrepareDisplayImage(ImageProxy& imDst, const HostImageType& imSrc);

            template<> 
            void PrepareDisplayImage(CImageRgb& imDst, const HostImageType& imSrc)
            {
                imDst.Copy(imSrc);
            }

            template<> 
            void PrepareDisplayImage(CImageArgb& imDst, const HostImageType& imSrc)
            {
                imDst.Copy(imSrc);
            }

            /// Normalize a imSrc, typically Real image
            /// then, copy it to a ImageDst type, typically Byte image
            template<> 
            void PrepareDisplayImage(CImageByte& imDst, const HostImageType& imSrc)
            {
                typedef CImageByte ImageDst;
                typedef HostImageType ImageSrc;

                typedef ImageDst::PixelType DstType;
                typedef ImageSrc::PixelType SrcType;

                if(imDst != imSrc) imDst.Allocate(imSrc);
                const int cx = imSrc.Width();
                const int cy = imSrc.Height();

                SrcType minValue = xtl::Limits<SrcType>::max_value();
                SrcType maxValue = xtl::Limits<SrcType>::min_value();

                for (int y = 0; y < cy; y ++)
                {
                    const SrcType* pSrc = imSrc.RowPtr(y);
                    for (int x = 0; x < cx; x ++)
                    {
                        if (minValue > pSrc[x]) minValue = pSrc[x];
                        if (maxValue < pSrc[x]) maxValue = pSrc[x];
                    }
                }

                const SrcType diff = maxValue - minValue;
                const DstType black = img::PixelTraits<DstType>::Black;
                const DstType white = img::PixelTraits<DstType>::White;

                if (diff <= 0)
                {
                    imDst.FillPixels(0);
                }
                else
                {
                    if (black == 0)
                    {
                        for (int y = 0; y < cy; y ++)
                        {
                            DstType* pDst = imDst.RowPtr(y);
                            const SrcType* pSrc = imSrc.RowPtr(y);
                            for (int x = 0; x < cx; x ++)
                            {
                                pDst[x] = xtl::RealCast<DstType>(
                                    (pSrc[x] - minValue) * white / diff);
                            }
                        }
                    }
                    else
                    {
                        const DstType range = white - black;
                        for (int y = 0; y < cy; y ++)
                        {
                            DstType* pDst = imDst.RowPtr(y);
                            const SrcType* pSrc = imSrc.RowPtr(y);
                            for (int x = 0; x < cx; x ++)
                            {
                                pDst[x] = xtl::RealCast<DstType>(
                                    (pSrc[x] - minValue) * range / diff + black);
                            }
                        }
                    }
                }
            }
        };

        template<>
        class ImageDisplayPolicy<CImageFloat>
            : public impl::ImageDisplayProxy<CImageByte, CImageFloat>
        {
        };

        template<>
        class ImageDisplayPolicy<CImageDouble>
            : public impl::ImageDisplayProxy<CImageByte, CImageDouble>
        {
        };

        template<>
        class ImageDisplayPolicy<CImageRgbFloat>
            : public impl::ImageDisplayProxy<CImageRgb, CImageRgbFloat>
        {
        };

        template<>
        class ImageDisplayPolicy<CImageRgbDouble>
            : public impl::ImageDisplayProxy<CImageRgb, CImageRgbDouble>
        {
        };

        template<>
        class ImageDisplayPolicy<CImageRgbWord>
            : public impl::ImageDisplayProxy<CImageRgb, CImageRgbWord>
        {
        };
    }
#pragma endregion

#pragma region -- Helper functions to copy pixels

    namespace impl
    {

        ///  Copy scan line for different pixel format
        ///  for each pixel, call PixelAssignment to convert format
        template<class SRCPIXEL, class DSTPIXEL>
        void CopyScanLine(DSTPIXEL* pDst, const SRCPIXEL* pSrc, UINT nPixelCount)
        {
            if(nPixelCount > MAX_IMAGE_WIDTH) return;
            for (UINT x = 0; x < nPixelCount; x ++)
            {
                PixelAssignment<DSTPIXEL, SRCPIXEL>::Assign(*pDst++, *pSrc++);
            }
        }

        ///  Copy from same pixel format
        ///  only do memory copy, do not check pixel translation
        template<class PIXELTYPE>
        void CopyScanLine(PIXELTYPE* pDst, const PIXELTYPE* pSrc, UINT nPixelCount)
        {
            if(nPixelCount > MAX_IMAGE_WIDTH) return;
            memcpy(pDst, pSrc, nPixelCount * sizeof(PIXELTYPE));
        }

        /// Copy image data from srcImage, RectSrc to dstImage at ptDst
        /// All data must be exists, no checking inside the function
        template<class DSTPIXEL, class SRCPIXEL>
        HRESULT CopyImageData(
            const Gdiplus::Point& ptDst, DSTPIXEL* dstScan0, INT dstStride,
            const Gdiplus::Rect& rcSrc, const SRCPIXEL* srcScan0, INT srcStride)
        {
            assert(ptDst.X >= 0 && ptDst.Y >= 0);
            assert(rcSrc.X >= 0 && rcSrc.Y >= 0);
            assert(rcSrc.Width > 0 && rcSrc.Height > 0);

            DSTPIXEL* pDst = (DSTPIXEL*)
                ((BYTE*)(dstScan0) + dstStride * ptDst.Y) + ptDst.X;
            const SRCPIXEL* pSrc = (const SRCPIXEL*)
                ((const BYTE*)(srcScan0) + srcStride * rcSrc.Y) + rcSrc.X;

            for (int y = 0; y < rcSrc.Height; y ++)
            {
                CopyScanLine(pDst, pSrc, rcSrc.Width);
                pDst = (DSTPIXEL*) ((BYTE*)(pDst) + dstStride);
                pSrc = (const SRCPIXEL*) ((BYTE*)(pSrc) + srcStride);
            }

            return S_OK;
        }
    }
#pragma endregion

    /// class CImagePyramid, stores a pyramid of image
    /// Each level is a image
    /// Level 0 is same as base level image
    template<class ImageType>
    class CImagePyramid : private std::vector<ImageType>
    {
        typedef std::vector<ImageType> super;

    public:
        using super::resize;
        using super::size;
        using super::empty;
        using super::operator [];
        using super::clear;
    };

    /// Build the pyramid based on super::image
    /// if nLevel <= 0, build pyramid with max level, i.e. log2(min dimension)
    /// Note that level 0 is exactly original input image as a reference
    template<class ImageType>
    HRESULT BuildGaussianPyramid(
        CImagePyramid<ImageType>& pyramid, const ImageType& image, INT nLevel = 0)
    {
        if (image.IsEmpty()) return E_INVALIDARG;

        const int minDim = min(image.Width(), image.Height());
        const int maxLevel = xtl::FloorCast<int>(log((double)minDim) / log(2.0));

        if (nLevel <= 0 || nLevel > maxLevel)
        {
            /// maxLevel is possible zero when minDim == 1
            nLevel = max(1, maxLevel);
        }

        pyramid.resize(nLevel);
        pyramid[0] = image;      /// reference level 0 to src image

        ImageType imBlur;
        img::FixedKernel_121<ConvolutionDefault> kernel;

        for (INT k = 1; k < nLevel; k ++)
        {
            img::ConvolveImageHV(imBlur, pyramid[k - 1], kernel, kernel);
            img::ImageDecimate(pyramid[k], imBlur, 2, 2);
        }

        return S_OK;
    }

    /// Build the pyramid based on super::image
    /// Using decimate only, no blurring before downsample
    template<class ImageType>
    HRESULT BuildDecimatePyramid(
        CImagePyramid<ImageType>& pyramid, const ImageType& image, INT nLevel = 0)
    {
        if (image.IsEmpty()) return E_INVALIDARG;

        const int minDim = min(image.Width(), image.Height());
        const int maxLevel = xtl::FloorCast<int>(log((double)minDim) / log(2.0));

        if (nLevel <= 0 || nLevel > maxLevel)
        {
            /// maxLevel is possible zero when minDim == 1
            nLevel = max(1, maxLevel);
        }

        pyramid.resize(nLevel);
        pyramid[0] = image;      /// reference level 0 to src image

        for (INT k = 1; k < nLevel; k ++)
        {
            img::ImageDecimate(pyramid[k], pyramid[k - 1], 2, 2);
        }

        return S_OK;
    }


} /// namespace img

//#undef NativeImage
