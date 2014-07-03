#include "Stdafx.h"
#include "IImage.h"
#include "ImageProcess.h"
#include "CommonFrameFeatureFactory.h"
#include "ColorHistogramFeatureExtractor.h"


namespace VideoAnalysis
{
    using namespace ImageAnalysis;
    //decide whether it is a black frame
    bool IsBlackScreen(CGrayImage &gray,CRgbImage &image)
    {
        int _width			= image.Width();
	    int _height		    = image.Height();
	    int _stride_rgb	    = image.Stride();
	    int _stride_gray	= gray.Stride();
	    int _size			= _width * _height;
	    int _black			= static_cast<unsigned int>(_size*0.95) + 1;
	    int _white			= _size - _black +1;

	    unsigned char *p = (unsigned char *)image.RowPtr(0);
	    unsigned char *q = (unsigned char *)gray.RowPtr(0);
        for ( int y = 0; y < _height; ++y, p += _stride_rgb, q += _stride_gray )
        {
		       RGB *rgb = (RGB*)p;
		       GRAY *g  = (GRAY*)q;
		       for( int x = 0; x < _width; ++x, ++rgb, ++g )
		       {
				      if( *g < 75 && rgb->b < 100 && rgb->g < 100 && rgb->r < 100 )
				      {
					       if( 0 == --_black ) 
						       return true;
				      }	else {
					       if( 0 == --_white ) 
						       return false;
				      }
	           }
      }
      return false;	
    } 

     //the frame feature cache, right now it only cache one frame
     //it's a singleton
     CommonFrameFeatureFactory * CommonFrameFeatureFactory::GetInstance()
     {
        //return the single object of feature factory
        static CommonFrameFeatureFactory featureFactory;
        return &featureFactory;
     }

     //whether it is a black frame
     bool CommonFrameFeatureFactory::IsBlackScreen(CFrame & frame)
     {
        if( m_FrameId != frame.Id() )  //it has been the new frame, we first invalidate the features of the pre-frame
        {
            InvalidateFeature();
            m_FrameId = frame.Id();
        }
            
        //get the width , height
	    int width = frame.GetImage()->Width();
	    int height = frame.GetImage()->Height();
            
        //get the gray image, if it is invalidate
        if( !m_IsGrayImageValidate )
        {       
            //just allocate when the width and height of the frame have been changed
            if( m_ImageWidth != width || m_ImageHeight != height )
            {
                m_GrayImage.Allocate(width, height);
                m_ImageWidth = width;
                m_ImageHeight = height;
             }
             Rgb2Gray(&m_GrayImage,  frame.GetImage());
	         m_IsGrayImageValidate = true;
         }

         //Test whether it is a black screen
         if( !m_IsBlackScreenValidate )
         {
             m_BlackScreen = VideoAnalysis::IsBlackScreen(m_GrayImage, *frame.GetImage());
             m_IsBlackScreenValidate = true;
         }
    
         return m_BlackScreen;
     }

       
    //get the birghtness of a frame
    unsigned char CommonFrameFeatureFactory::ExtractBrightness(CFrame & frame)
    {
         if( m_FrameId != frame.Id() )  //it has been the new frame, we first invalidate the features of the pre-frame
         {
             InvalidateFeature();
             m_FrameId = frame.Id();
         }

         //get the width , height
	     int width = frame.GetImage()->Width();
	     int height = frame.GetImage()->Height();

         //get the gray image, if it is invalidate
         if( !m_IsGrayImageValidate )
         {
             //just allocate when the width and height of the frame have been changed
	         if( m_ImageWidth != width || m_ImageHeight != height )
             {
                 m_GrayImage.Allocate(width, height);
                 m_ImageWidth = width;
                 m_ImageHeight = height;
              }
              Rgb2Gray(&m_GrayImage, frame.GetImage());
	          m_IsGrayImageValidate = true;
         }

         //get brightness of current frame, if it is invalidate
         if( !m_IsBrightnessValidate )
         {
	          m_Brightness = Brightness(&m_GrayImage);
              m_IsBrightnessValidate = true;
         }
           
         return m_Brightness;
    }
    
    //get the rgb4096histo of a frame
    CRGB4096Histo & CommonFrameFeatureFactory::ExtractRGB4096Histo(CFrame & frame)
    {
         //it has been the new frame, we first invalidate the features of the pre-frame
         if( m_FrameId != frame.Id() )  
         {
             InvalidateFeature();
             m_FrameId = frame.Id();
          }
            
          //get the width , height
	      int width = frame.GetImage()->Width();
	      int height = frame.GetImage()->Height();
            
          //get rgb4096Histogram
          if( !m_IsRGB4096HistoValidate )
          {
               m_RGB4096Histo = CRGB4096HistogramFeatureExtractor::ExtractNonNormalizeHisto(*frame.GetImage());
    	       m_IsRGB4096HistoValidate = true;
          }
          return m_RGB4096Histo;
    }

    void CommonFrameFeatureFactory::InvalidateFeature()
    {
          m_IsBlackScreenValidate = false;
          m_IsBrightnessValidate = false;
          m_IsRGB4096HistoValidate = false;
          m_IsGrayImageValidate = false;
    }

    CommonFrameFeatureFactory::CommonFrameFeatureFactory()
    {
          m_FrameId = -1;
          m_ImageWidth = -1;
          m_ImageHeight = -1;
          InvalidateFeature();
    }
}