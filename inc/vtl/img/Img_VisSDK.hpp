/*************************************************************************\
Microsoft Research Asia
Copyright(c) 2003 Microsoft Corporation

Module Name:
    Vis Lib Image: Image accessing from Vision SDK.
  
Abstract:
    
Notes:

Usage:
    CImageArgb m_imColor;
    img::LoadImageFile(m_imColor, "...");

    CVisRGBAByteImage imVis;
    imVis = img::GetVisImage(m_imColor);
    VisDisplayImage(imVis);

    CImageArgb imVtl;
    img::BindVisImage(imVtl, imVis);
    utl::DisplayImage(imVtl);

History:
    Created  on 2004 Nov 4 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "viscore.h"

#ifdef _DEBUG
#   pragma comment(lib, "viscoreDB.lib")
#else
#   pragma comment(lib, "viscore.lib")
#endif

namespace img
{
    template<class T> class CImage;

#pragma region -- Implementation of Vision SDK pixel format mapping

    namespace impl
    {
        template<class T>
        struct MapVisSDKPixelType
        {
            typedef T Result;   // default is the type itself
        };

#define MAPPING_VIS_PIXEL_TYPE(PIXEL_TYPE, VIS_PIXEL_TYPE)          \
        template<> struct MapVisSDKPixelType<PIXEL_TYPE>            \
            { typedef VIS_PIXEL_TYPE Result;}                       \

        MAPPING_VIS_PIXEL_TYPE(PixelArgb, CVisRGBABytePixel);
        MAPPING_VIS_PIXEL_TYPE(PixelByte, CVisBytePixel);
        MAPPING_VIS_PIXEL_TYPE(PixelFloat, CVisFloatPixel);
        MAPPING_VIS_PIXEL_TYPE(PixelInt, CVisLongPixel);
        MAPPING_VIS_PIXEL_TYPE(PixelDouble, CVisDoublePixel);

#undef MAPPING_VIS_PIXEL_TYPE

    } // namespace impl

#pragma endregion

    template<class T>
    CVisImage<typename impl::MapVisSDKPixelType<T>::Result> GetVisImage(const CImage<T>& image)
    {
        typedef CVisImage<typename impl::MapVisSDKPixelType<T>::Result> CVisImageType;
        CVisImageType result;
        result.SetMemBlock(
            CVisMemBlock((void*) image.RowPtr(0), image.Stride() * image.Height()),
            CVisShape(0, 0, image.Stride() / image.GetPixelSize(), image.Height()),
            CVisShape(0, 0, image.Width(), image.Height()));
        return result;
    }

    template<class T>
    void BindVisImage(CImage<T>& image, 
                      const CVisImage<typename impl::MapVisSDKPixelType<T>::Result>& visImage)
    {
        image.Allocate(visImage.Width(),
                       visImage.Height(),
                       visImage.CbRow(),
                       (void*) visImage.RowPointer(0));
    }

} /// namespace img
