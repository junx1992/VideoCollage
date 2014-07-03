#include "StdAfx.h"
#include "RgbImage.h"
#include "img\img_image.h"
#include "img\img_gdiplus.h"
#include "xtl\Xtl_Pointer.hpp"

#pragma region CRgbImageImp
//since forward declaration can't declare template class and typedef, we define a container class to 
//contain it
class CRgbImageImp
{
public:
    CRgbImageImp() {}
    CRgbImageImp(img::CImageRgb* const pImg)
        :m_pImage(pImg) 
    {}
    CRgbImageImp(const CRgbImageImp& other)
        :m_pImage(other.m_pImage)
    {}
    img::CImageRgb* operator->()
    {
        return GetPtr();
    }
    img::CImageRgb* GetPtr()
    {
        if(!m_pImage)
            m_pImage = new img::CImageRgb;
        return m_pImage;
    }
    CRgbImageImp& operator=(const CRgbImageImp& other)
    {
        if(this != &other)
            this->m_pImage = other.m_pImage;
        return *this;
    }
private:
    xtl::SharedPtr<img::CImageRgb> m_pImage;
};
#pragma endregion

namespace ImageAnalysis
{
	CRgbImage::CRgbImage(void)
	{
		m_pImageImp = new CRgbImageImp();
	}

	CRgbImage::~CRgbImage(void)
	{
		delete m_pImageImp;
	}

    CRgbImage::CRgbImage(const CRgbImage& r)
    {
        m_pImageImp = new CRgbImageImp();
        *m_pImageImp = *r.m_pImageImp;
    }

    CRgbImage& CRgbImage::operator=(const CRgbImage& r)
    {
        if(this != &r)
            *m_pImageImp = *r.m_pImageImp;
        return *this;
    }

	const int CRgbImage::Width() const
	{
        return (*m_pImageImp)->Width();
	}
	const int CRgbImage::Height() const
	{
		return (*m_pImageImp)->Height();
	}
	const int CRgbImage::Stride() const
	{
		return (*m_pImageImp)->Stride();
	}
	const BYTE* CRgbImage::ImageOrigin() const
	{
		if((*m_pImageImp)->Stride() < 0)
			return (const BYTE*)(*m_pImageImp)->RowPtr((*m_pImageImp)->Height()-1);
		else
			return (const BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	const BYTE* CRgbImage::MemoryOrigin() const
	{
		return (const BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	const BYTE* CRgbImage::RowPtr(const int x) const
	{
		return (const BYTE*)(*m_pImageImp)->RowPtr(x);
	}
	const BYTE* CRgbImage::PixelPtr(const int x, const int y) const
	{
		return (const BYTE*)(*m_pImageImp)->PixelPtr(x, y);
	}
	BYTE* CRgbImage::ImageOrigin()
	{
		if((*m_pImageImp)->Stride() < 0)
			return (BYTE*)(*m_pImageImp)->RowPtr((*m_pImageImp)->Height()-1);
		else
			return (BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	BYTE* CRgbImage::MemoryOrigin()
	{	
		return (BYTE*)(*m_pImageImp)->RowPtr(0);
	}
	BYTE* CRgbImage::RowPtr(const int x)
	{
		return (BYTE*)(*m_pImageImp)->RowPtr(x);
	}
	BYTE* CRgbImage::PixelPtr(const int x, const int y)
	{
		return (BYTE*)(*m_pImageImp)->PixelPtr(x, y);
	}
	const CRgbImage::ImagePixelFormat CRgbImage::PixelFormat() const
	{
		return Format24bppRgb;
	}

	CRgbImage* CRgbImage::Clone() const
	{
		CRgbImage* pNew = new CRgbImage();
        (*pNew->m_pImageImp) = *m_pImageImp;
		return pNew;
	}
	HRESULT CRgbImage::Allocate(const int width, const int height)
	{
		return (*m_pImageImp)->Allocate(width, height);
	}
	HRESULT CRgbImage::Allocate(const int width, const int height, const int stride)
	{
		return E_NOTIMPL;
	}
	HRESULT CRgbImage::Allocate(const int width, const int height, const int stride, const BYTE* imgPtr)
	{
		return (*m_pImageImp)->Allocate(width, height, stride, (void*) imgPtr);
	}

	void CRgbImage::Release()
	{
		delete this;
	}

	/*HRESULT CRgbImage::Attach(const IImage* image) const
	{
		return (*m_pImageImp)->Allocate(image->Width(), image->Height(), image->Stride(), (void*)image->MemoryOrigin() );
	}

    HRESULT CRgbImage::Attach(IImage* image)
	{
		return (*m_pImageImp)->Allocate(image->Width(), image->Height(), image->Stride(), (void*)image->MemoryOrigin() );
	}*/

	CRgbImage* CRgbImage::SubImage(const RECTANGLE roi) const
	{
		Gdiplus::Rect _roi;
		_roi.X = roi.left;
		_roi.Y = roi.top;
		_roi.Width = roi.right - roi.left;
		_roi.Height = roi.bottom - roi.top;
		CRgbImage* pNew = new CRgbImage();
        if(FAILED((*pNew->m_pImageImp)->Copy(*m_pImageImp->GetPtr(), _roi)) )
			return NULL;
		return pNew;
	}

	HRESULT CRgbImage::Copy(const IImage& image, const RECTANGLE roi)
	{
        if(dynamic_cast<const IRgbImage*>(&image) ==NULL)
            return E_FAIL;
		Gdiplus::Rect _roi;
		_roi.X = roi.left;
		_roi.Y = roi.top;
		_roi.Width = roi.right - roi.left;
		_roi.Height = roi.bottom - roi.top;
		CRgbImage src;
		src.Allocate(image.Width(), image.Height(), image.Stride(), image.MemoryOrigin());
		return (*m_pImageImp)->Copy(*src.m_pImageImp->GetPtr(), _roi);
	}

	HRESULT CRgbImage::Copy(const IImage& image)
	{
        if(dynamic_cast<const IRgbImage*>(&image) ==NULL)
            return E_FAIL;
		CRgbImage src;
		src.Allocate(image.Width(), image.Height(), image.Stride(), image.MemoryOrigin());
		return (*m_pImageImp)->Copy(*src.m_pImageImp->GetPtr());
	}

	CRgbImage* CRgbImage::Resize(const int destWidth, const int destHeight) const
	{
		CRgbImage* pNew = new CRgbImage();
		if ( FAILED((*pNew->m_pImageImp)->Allocate(destWidth, destHeight)) ) 
		{
			return NULL;
		}
		
		Gdiplus::Rect recSrc = (*m_pImageImp)->GetRect();
		Gdiplus::Rect recDst = (*pNew->m_pImageImp)->GetRect();
		Gdiplus::Graphics *g = img::CreateGraphics(pNew->m_pImageImp->GetPtr());
		if(FAILED(img::DrawImage(g, m_pImageImp->GetPtr(), recDst, &recSrc) ))
		{
			delete g;
			return NULL;
		}
		delete g;
		return pNew;
	}

	HRESULT CRgbImage::Load(const wchar_t* fileName)
	{
		return img::LoadImageFile(m_pImageImp->GetPtr(), fileName);
	}

	HRESULT CRgbImage::Save(const wchar_t* fileName, ImageFormat format)
	{
		const wchar_t* gdiplusImageFormat = ConvertImageFormatToGdiplusImageFormatString(format);
		if(NULL == gdiplusImageFormat)
			return E_INVALIDARG;
		return img::SaveImageFile(m_pImageImp->GetPtr(), fileName, gdiplusImageFormat); 
	}

	const RgbTriple CRgbImage::operator()(const int x, const int y)
	{
		RgbTriple r;
		img::PixelRgb* pixelPtr = (img::PixelRgb*)PixelPtr(x, y);
		r.b = pixelPtr->B();
		r.g = pixelPtr->G();
		r.r = pixelPtr->R();
		return r;
	}

}