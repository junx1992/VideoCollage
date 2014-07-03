/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the implementation of gray image 

Notes:
  

History:
  Created on 04/23/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "iimage.h"
//declaration of vtl image implementation
class CGrayImageImp;

namespace ImageAnalysis
{
    ///image with the pixel format is byte
	class DLL_IN_EXPORT CGrayImage :
		public IGrayImage, public IImageEx, public IImagePersistence
	{
	public:
		CGrayImage(void);
		~CGrayImage(void);
        CGrayImage(const CGrayImage& r);
        CGrayImage& operator=(const CGrayImage& r);
	public:
		virtual const int Width() const;
		virtual const int Height() const;
		virtual const int Stride() const;
		virtual const BYTE* ImageOrigin() const;
		virtual const BYTE* MemoryOrigin() const;
		virtual const BYTE* RowPtr(const int x) const;
		virtual const BYTE* PixelPtr(const int x, const int y) const;
		virtual BYTE* ImageOrigin();
		virtual BYTE* MemoryOrigin();
		virtual BYTE* RowPtr(const int x);
		virtual BYTE* PixelPtr(const int x, const int y);
		virtual const ImagePixelFormat PixelFormat() const;
		virtual CGrayImage* Clone() const;
		virtual HRESULT Allocate(const int width, const int height);
		virtual HRESULT Allocate(const int width, const int height, const int stride);
		virtual HRESULT Allocate(const int width, const int height, const int stride, const BYTE* imgPtr);
		virtual void Release();
		/*virtual HRESULT Attach(const IImage* image) const;
        virtual HRESULT Attach(IImage* image);*/
		//ROI support
		virtual CGrayImage* SubImage(const RECTANGLE roi) const;
		virtual HRESULT Copy(const IImage& image, const RECTANGLE roi);
		virtual HRESULT Copy(const IImage& image);
		virtual CGrayImage* Resize(const int destWidth, const int destHeight) const;
	//implementation of interface IImagePersistence
	public:
		///load the image (which can be  a rgb image) and then convert it to gray
		HRESULT Load(const wchar_t* fileName);
		HRESULT Save(const wchar_t* fileName, ImageFormat format = ImageFormat_JPEG);

	//implementation of some other methods to ease the use of this class
	public:
		const BYTE operator()(const int x, const int y);

	private:
		CGrayImageImp* m_pImageImp;
	};
}