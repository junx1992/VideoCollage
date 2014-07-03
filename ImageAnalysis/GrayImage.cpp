#include "StdAfx.h"
#include "GrayImage.h"
#include "img\img_image.h"
#include "img\img_gdiplus.h"
#include "RgbImage.h"
#include "ImageAnalysisException.h"
#include "ImageProcess.h"
#include "gdiplus.h"
#include "xtl\Xtl_Pointer.hpp"

#pragma region CGrayImageImp
//since forward declaration can't declare template class and typedef, we define a container class to 
//contain it
class CGrayImageImp
{
public:
    CGrayImageImp() {}
    CGrayImageImp(img::CImageByte* const pImg)
        :m_pImage(pImg) 
    {}
    CGrayImageImp(const CGrayImageImp& other)
        :m_pImage(other.m_pImage)
    {}
    img::CImageByte* operator->()
    {
        return GetPtr();
    }
    img::CImageByte* GetPtr()
    {
        if(!m_pImage)
            m_pImage = new img::CImageByte;
        return m_pImage;
    }
    CGrayImageImp& operator=(const CGrayImageImp& other)
    {
        if(this != &other)
            this->m_pImage = other.m_pImage;
        return *this;
    }
private:
    xtl::SharedPtr<img::CImageByte> m_pImage;
};
#pragma endregion

namespace ImageAnalysis
{
	CGrayImage::CGrayImage(void)
	{
		m_pImageImp = new CGrayImageImp();
	}

	CGrayImage::~CGrayImage(void)
	{
		delete m_pImageImp;
	}

    CGrayImage::CGrayImage(const CGrayImage& r)
    {
        m_pImageImp = new CGrayImageImp();
        *m_pImageImp = *r.m_pImageImp;
    }

    CGrayImage& CGrayImage::operator=(const CGrayImage& r)
    {
        if(this != &r)
            *m_pImageImp = *r.m_pImageImp;
        return *this;
    }

	const int CGrayImage::Width() const
	{
		return (*m_pImageImp)->Width();
	}
	const int CGrayImage::Height() const
	{
		return (*m_pImageImp)->Height();
	}
	const int CGrayImage::Stride() const
	{
		return (*m_pImageImp)->Stride();
	}
	const BYTE* CGrayImage::ImageOrigin() const
	{
		if((*m_pImageImp)->Stride() < 0)
			return (const BYTE*)(*m_pImageImp)->RowPtr((*m_pImageImp)->Height()-1);
		else
			return (const BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	const BYTE* CGrayImage::MemoryOrigin() const
	{
		return (const BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	const BYTE* CGrayImage::RowPtr(const int x) const
	{
		return (const BYTE*)(*m_pImageImp)->RowPtr(x);
	}
	const BYTE* CGrayImage::PixelPtr(const int x, const int y) const
	{
		return (const BYTE*)(*m_pImageImp)->PixelPtr(x, y);
	}
	BYTE* CGrayImage::ImageOrigin()
	{
		if((*m_pImageImp)->Stride() < 0)
			return (BYTE*)(*m_pImageImp)->RowPtr((*m_pImageImp)->Height()-1);
		else
			return (BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	BYTE* CGrayImage::MemoryOrigin()
	{	
		return (BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	BYTE* CGrayImage::RowPtr(const int x)
	{
		return (BYTE*)(*m_pImageImp)->RowPtr(x);
	}
	BYTE* CGrayImage::PixelPtr(const int x, const int y)
	{
		return (BYTE*)(*m_pImageImp)->PixelPtr(x, y);
	}
	const CGrayImage::ImagePixelFormat CGrayImage::PixelFormat() const
	{
		return Format8bppGray;
	}

	CGrayImage* CGrayImage::Clone() const
	{
		CGrayImage* pNew = new CGrayImage();
        (*pNew->m_pImageImp) = *m_pImageImp;
		return pNew;
	}
	HRESULT CGrayImage::Allocate(const int width, const int height)
	{
		return (*m_pImageImp)->Allocate(width, height);
	}
	HRESULT CGrayImage::Allocate(const int width, const int height, const int stride)
	{
		return E_NOTIMPL;
	}
	HRESULT CGrayImage::Allocate(const int width, const int height, const int stride, const BYTE* imgPtr)
	{
		return (*m_pImageImp)->Allocate(width, height, stride, (void*) imgPtr);
	}

	void CGrayImage::Release()
	{
		delete this;
	}

	/*HRESULT CGrayImage::Attach(const IImage* image) const
	{
		return (*m_pImageImp)->Allocate(image->Width(), image->Height(), image->Stride(), (void*)image->MemoryOrigin() );
	}

    HRESULT CGrayImage::Attach(IImage* image)
	{
		return (*m_pImageImp)->Allocate(image->Width(), image->Height(), image->Stride(), (void*)image->MemoryOrigin() );
	}*/

	CGrayImage* CGrayImage::SubImage(const RECTANGLE roi) const
	{
		Gdiplus::Rect _roi;
		_roi.X = roi.left;
		_roi.Y = roi.top;
		_roi.Width = roi.right - roi.left;
		_roi.Height = roi.bottom - roi.top;
		CGrayImage* pNew = new CGrayImage();
		if(FAILED((*pNew->m_pImageImp)->Copy(*m_pImageImp->GetPtr(), _roi)) )
			return NULL;
		return pNew;
	}

	HRESULT CGrayImage::Copy(const IImage& image, const RECTANGLE roi)
	{
		Gdiplus::Rect _roi;
		_roi.X = roi.left;
		_roi.Y = roi.top;
		_roi.Width = roi.right - roi.left;
		_roi.Height = roi.bottom - roi.top;
		CGrayImage src;
        src.Allocate(image.Width(), image.Height(), image.Stride(), image.MemoryOrigin());
		return (*m_pImageImp)->Copy(*src.m_pImageImp->GetPtr(), _roi);
	}

	HRESULT CGrayImage::Copy(const IImage& image)
	{
		CGrayImage src;
		src.Allocate(image.Width(), image.Height(), image.Stride(), image.MemoryOrigin());
		return (*m_pImageImp)->Copy(*src.m_pImageImp->GetPtr());
	}

	CGrayImage* CGrayImage::Resize(const int destWidth, const int destHeight) const
	{
		/*CGrayImage* pNew = new CGrayImage();
		if ( FAILED((*pNew->m_pImageImp)->Allocate(destWidth, destHeight)) ) 
		{
			return NULL;
		}
		
		Gdiplus::Rect recSrc = (*m_pImageImp)->GetRect();
		Gdiplus::Rect recDst = (*pNew->m_pImageImp)->GetRect();
		Gdiplus::Graphics *g = img::CreateGraphics(&pNew->*m_pImageImp->GetPtr());
		if(FAILED(img::DrawImage(g, &*m_pImageImp->GetPtr(), recDst, &recSrc) ))
		{
            delete pNew;
            pNew = NULL;
		}
		delete g;
		return pNew;*/
        return NULL;
	}

	HRESULT CGrayImage::Load(const wchar_t* fileName)
	{
		CRgbImage rgbImg;
		RETURN_IF_FAIL(rgbImg.Load(fileName));
		RETURN_IF_FAIL(Allocate(rgbImg.Width(), rgbImg.Height()));
		RETURN_IF_FAIL(Rgb2Gray(this, &rgbImg));
		return S_OK;
	}

	// used only for CGrayImage::Save(const wchar_t* fileName, ImageFormat format)
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
    {
        using Gdiplus::ImageCodecInfo;
        using Gdiplus::GetImageEncodersSize;
        using Gdiplus::GetImageEncoders;

        UINT  num = 0;          /// number of image encoders
        UINT  size = 0;         /// size of the image encoder array in bytes

        /// get the number of codecs in list
        GetImageEncodersSize(&num, &size);
        if (size == 0) return -1;  /// Failure
        if (size <= num * sizeof(ImageCodecInfo)) return -1;

        /// Allocate the memory, which maybe larger than ImageCodeInfo array
        xtl::ScopedArray<BYTE> pBuffer(new BYTE[size]);
        if (pBuffer == NULL) return -1;  /// Failure

        /// Bind the head of buffer to arrray of codec infos
        ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*) (void*) pBuffer;

        /// read the codec info
        GetImageEncoders(num, size, pImageCodecInfo);

        /// enumerate and check the name
        for (UINT j = 0; j < num; ++j)
        {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                return j;  /// Success
            }    
        }

        return -1;  /// Failure
    }

	HRESULT CGrayImage::Save(const wchar_t* fileName, ImageFormat format)
	{
		Gdiplus::Bitmap* pBmp = new Gdiplus::Bitmap(Width(), Height(), Stride(), 
										PixelFormat8bppIndexed, RowPtr(0));
		HRESULT hr = S_OK;
		//set palette
		UINT size = pBmp->GetPaletteSize();
		Gdiplus::ColorPalette* palette = NULL;
		do
		{
			palette = (Gdiplus::ColorPalette*)malloc(size);
			if(NULL == palette)
				BREAK_IF_FAIL(E_FAIL);
			Gdiplus::Status status = pBmp->GetPalette(palette, size);
			if(status != Gdiplus::Ok)
				BREAK_IF_FAIL(E_FAIL);
			for(UINT j = 0; j < palette->Count; ++j)
				palette->Entries[j] = Gdiplus::Color::MakeARGB(255, j, j, j);
			status = pBmp->SetPalette(palette);
			if(status != Gdiplus::Ok)
				BREAK_IF_FAIL(E_FAIL);
			//save
			const wchar_t* gdiplusImageFormat = ConvertImageFormatToGdiplusImageFormatString(format);
			if(NULL == gdiplusImageFormat)
				BREAK_IF_FAIL(E_INVALIDARG);
			CLSID  encoderClsid;
			if(GetEncoderClsid(gdiplusImageFormat, &encoderClsid) < 0) 
				BREAK_IF_FAIL(E_INVALIDARG);;
			status = pBmp->Save(fileName, &encoderClsid);
			if(status != Gdiplus::Ok)
				BREAK_IF_FAIL(E_FAIL);
		} while(0);
		delete pBmp;
		if(NULL != palette)
			free(palette);
        return hr;
	}

	const BYTE CGrayImage::operator()(const int x, const int y)
	{
		return *((BYTE*)PixelPtr(x, y));
	}
}
