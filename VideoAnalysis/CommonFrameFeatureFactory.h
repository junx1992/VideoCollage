/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for common frame feature extract and it is only 
  hold one frame.


Notes:
  The common feature includes:black frame, brightness, RGB4096Histogram

History:
  Created on 06/22/2007 by v-huami@microsoft.com
\******************************************************************************/
#pragma once
#include <utility>
#include "RgbImage.h"
#include "GrayImage.h"
#include "VxFeature.h"
#include "VideoSegments.h"

namespace VideoAnalysis
{
    ///using derective
    using ImageAnalysis::RgbTriple;
    using ImageAnalysis::CRgbImage;
    using ImageAnalysis::CGrayImage;
    using VxCore::CIntFeature;
    
    ///the typedef for rgb and gray, histogram
    typedef RgbTriple RGB;
    typedef unsigned char GRAY;
    typedef CIntFeature CRGB4096Histo; 

    ///the frame feature factory, it only cache one frame
    class DLL_IN_EXPORT CommonFrameFeatureFactory
    {
    public:
       ///it's a singleton
       static CommonFrameFeatureFactory * GetInstance();

       bool IsBlackScreen(CFrame & frame);                          ///whether it is a black frame
       unsigned char ExtractBrightness(CFrame & frame);             ///get the birghtness of a frame
       CRGB4096Histo & ExtractRGB4096Histo(CFrame & frame);         ///get the rgb4096histo of a frame
  
    private:
       CommonFrameFeatureFactory();
       ///!!!it is not implemented, which means don't allow any kind of copy
       CommonFrameFeatureFactory(const CommonFrameFeatureFactory &);
       CommonFrameFeatureFactory & operator = (const CommonFrameFeatureFactory &);

       void InvalidateFeature();                                    ///invalidate all the features

    private:

       int m_FrameId;                                               ///the frame id
       int m_ImageWidth;                                            ///the width of frame
       int m_ImageHeight;                                           ///the height of frame
       bool m_IsBlackScreenValidate;                                ///whether the black screen feature validate
       bool m_BlackScreen;                                          ///the black screen feature
       bool m_IsBrightnessValidate;                                 ///whether the black screen feature validate
       unsigned char  m_Brightness;                                 ///the brightness feature
       bool m_IsRGB4096HistoValidate;                               ///whether the RGB4096Histogram validate
       CRGB4096Histo  m_RGB4096Histo;                               ///the rgb4096 historgram feature
       bool m_IsGrayImageValidate;                                  ///whether the gray image validate
       CGrayImage m_GrayImage;                                      ///the gray image
    };
}