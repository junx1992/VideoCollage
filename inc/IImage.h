/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the interface of image 

Notes:
  

History:
  Created on 04/12/2007 by linjuny@microsoft.com
\******************************************************************************/

#pragma once
#include "VxComm.h"
namespace ImageAnalysis
{
	struct RECTANGLE
	{
		long    left;
		long    top;
		long    right;
		long    bottom;
		RECTANGLE(long _left, long _top, long _right, long _bottom):
			left(_left), top(_top), right(_right), bottom(_bottom)
		{ }
	};

    ///the struct for representing a rgb triple
	struct RgbTriple
	{
		BYTE    b;
        BYTE    g;
        BYTE    r;
		RgbTriple()
			:b(0), g(0), r(0)
		{}
#ifdef _WINDOWS_
		RgbTriple(RGBTRIPLE c)
			:b(c.rgbtBlue), g(c.rgbtGreen), r(c.rgbtRed)
		{ }
		operator RGBTRIPLE()
		{
			RGBTRIPLE c;
			c.rgbtBlue = b; c.rgbtGreen = g; c.rgbtRed = r;
		}
#endif
	};

	struct HsvTriple
	{
		float h;
		float s;
		float v;
	};
	///image interface
	///mainly focus on image's basic properties, image memory access
	class DLL_IN_EXPORT IImage
	{
	public:
        ///pixel format
		enum ImagePixelFormat
		{
			FormatUndefined = 0,
			Format24bppRgb,
			Format8bppGray
		};
	public:
        ///image width
		virtual const int Width() const = 0;
        ///image height
		virtual const int Height() const = 0;
        ///image stride. i.e. the number of bytes per row
		virtual const int Stride() const = 0;
        ///the memory pointer to the image origin (the most left top)
		virtual const BYTE* ImageOrigin() const = 0;
        ///the memory pointer to the first byte of the image memory buffer
		virtual const BYTE* MemoryOrigin() const = 0;
        ///the memory pointer to the first byte of the specified row
		virtual const BYTE* RowPtr(const int x) const = 0;
        ///the memory pointer to the specified pixel by the coordinate x and y (relative to the most left top point)
		virtual const BYTE* PixelPtr(const int x, const int y) const = 0;
        ///the memory pointer to the image origin (the most left top)
		virtual BYTE* ImageOrigin() = 0;
        ///the memory pointer to the first byte of the image memory buffer
		virtual BYTE* MemoryOrigin() = 0;
        ///the memory pointer to the first byte of the specified row
		virtual BYTE* RowPtr(const int x) = 0;
        ///the memory pointer to the specified pixel by the coordinate x and y (relative to the most left top point)
		virtual BYTE* PixelPtr(const int x, const int y) = 0;
        ///the pixel format for this image
		virtual const ImagePixelFormat PixelFormat() const  = 0;
        ///clone doesn't copy the image data
		virtual IImage* Clone() const = 0;
        ///\brief Release the image. Normally this function is called to release the image returned by this sdk.
		///if the IImage object is not created by yourself, i.e., created by Copy, Resize, SubImage and etc., <br/>
		///please call Release to release it.<br/>
        ///!!!DO NOT call Release function on the image object create in the stack. <br/>
		///!!!DO NOT delete the object aftering call Release.
		virtual void Release() = 0;

	public:
		virtual ~IImage() = 0
		{

		}
	};

	///extented image interface
	class DLL_IN_EXPORT IImageEx
	{
	public:
		///allocate the image, the memory of the image content is allocated by the sdk
		virtual HRESULT Allocate(const int width, const int height) = 0;
        ///allocate the image, the memory of the image content is allocated by the sdk
		virtual HRESULT Allocate(const int width, const int height, const int stride) = 0;
        ///allocate the image, the memory of the image content is specified by the parameter imgPtr
		virtual HRESULT Allocate(const int width, const int height, const int stride, const BYTE* imgPtr) = 0;
		/*virtual HRESULT Attach(const IImage* image) const = 0;
        virtual HRESULT Attach(IImage* image)= 0;*/
		///ROI support
		virtual IImage* SubImage(const RECTANGLE roi) const = 0;
        ///copy this image to another, with ROI support
		virtual HRESULT Copy(const IImage& image, const RECTANGLE roi) = 0;
        ///copy this image to another
		virtual HRESULT Copy(const IImage& image) = 0;
        ///resize this image to form another new image. the returned image should be freed by calling Release()
		virtual IImage* Resize(const int destWidth, const int destHeight) const = 0;
	public:
		virtual ~IImageEx() = 0
		{

		}
	};
	
	///rgb and gray image in our image processing
	///specialize them so that the type is safer
	///for other image, such as HSV, you can differentiate
	///them by ImagePixelFormat
	class DLL_IN_EXPORT IRgbImage: public IImage
	{
    public:
        const ImagePixelFormat PixelFormat() const
	    {
		    return Format24bppRgb;
	    }
	};

	///rgb and gray image in our image processing
	///specialize them so that the type is safer
	class DLL_IN_EXPORT IGrayImage: public IImage
	{
    public:
        const ImagePixelFormat PixelFormat() const
	    {
		    return Format8bppGray;
	    }
	};

    ///interface for image load and save
	class DLL_IN_EXPORT IImagePersistence
	{
	public:
        ///image formant to be saved
		enum ImageFormat
		{
            ///jpeg image
			ImageFormat_JPEG,
            ///bmp image
			ImageFormat_BMP,
            ///gif image
			ImageFormat_GIF,
            ///png image
			ImageFormat_PNG,
            ///tiff image
			ImageFormat_TIFF
		};
	public:
        ///load image from a file
		virtual HRESULT Load(const wchar_t* fileName) = 0;
        ///save image to a file
		virtual HRESULT Save(const wchar_t* fileName, ImageFormat format) = 0;
	public:
		virtual ~IImagePersistence() = 0
		{

		}
	protected:
		static const wchar_t* ConvertImageFormatToGdiplusImageFormatString(const ImageFormat format)
		{
			switch(format)
			{
			case ImageFormat_JPEG:
				return L"image/jpeg";
			case ImageFormat_BMP:
				return L"image/bmp";
			case ImageFormat_GIF:
				return L"image/gif";
			case ImageFormat_PNG:
				return L"image/png";
			case ImageFormat_TIFF:
				return L"image/tiff";
			default:
				return NULL;
			}
		}
	};
}