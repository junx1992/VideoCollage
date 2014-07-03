#include "StdAfx.h"
#include "SimpleKeyframeExtractor.h"
#include "RgbImage.h"
#include "GrayImage.h"
#include "VideoSegments.h"
#include "VxFeature.h"
#include "ColorHistogramFeatureExtractor.h"
#include "CommonFrameFeatureFactory.h"
#include "ImageProcess.h"
using namespace ImageAnalysis;


namespace VideoAnalysis
{
	//get the intersect sum of 2 features
	unsigned long Intersect(const CIntFeature &H1, const CIntFeature &H2)
	{
		unsigned long sum = 0;

		//get the size of each Featrue
		int size_h1 = H1.Size();
		int size_h2 = H2.Size();

		//check the H1's size == H2's size
		if( size_h1 != size_h2 )
			return sum;

		for ( int i = 0; i < size_h1; ++i)
		{
			sum += H1[i] < H2[i] ? H1[i] : H2[i];
		}

		return sum;
	}
	
	//some thresholds of key frame extractor
	struct KeyFrameExtractorThreshold
	{
		static const double dblKeyFrameThresh;            //0.22 Threshold 
		static const int  iSkippedFrames;                 //5  we should skip the first frames for denoising
	};

    const double KeyFrameExtractorThreshold::dblKeyFrameThresh = 0.22;
    const int KeyFrameExtractorThreshold::iSkippedFrames = 5;


    CSimpleKeyframeExtractor::CSimpleKeyframeExtractor(void):m_Shot(3),m_isBlackScreenPre(false)
    {
    }

    CFrameList & CSimpleKeyframeExtractor::OnNewSegment(IVideoSegment& segment)
    {
	        m_Shot = 3;
		    unsigned int shotBgnFid = segment.BeginFrameId();
		    unsigned int shotEndFid = segment.EndFrameId();
		    ChooseKeyframe(shotBgnFid, shotEndFid);

		    if ( m_Shot > 0 ) --m_Shot;

		    return m_KeyframeList;
    }

    HRESULT CSimpleKeyframeExtractor::OnNewFrame(CFrame& frame, bool isKeyframeCandidate)
    {
            //get the frame index
            m_FrameId = frame.Id();
     
		    HRESULT _ret = S_OK;
            if ( KeyFrameExtractorThreshold::iSkippedFrames > m_FrameId )
            {
                SaveFrameFeature(frame);			 
                return S_OK;
            } else if ( KeyFrameExtractorThreshold::iSkippedFrames == m_FrameId )
			     m_Shot = 3;
		    else if( isKeyframeCandidate ) {
			    double k = 1.0 - double(Intersect(m_HistRef, CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame))) / (frame.GetImage()->Width()*frame.GetImage()->Height());
			    if ( 
                    k > KeyFrameExtractorThreshold::dblKeyFrameThresh 
				    && !m_isBlackScreenPre ) 
			    {
				    AddKeyFrameCandidate(frame);
				    m_HistRef = CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
			    }
			    _ret = S_OK;
		    }

		    if ( m_Shot > 0 ) --m_Shot;
		    if ( m_Shot == 1 ) 
		    {
			    if( !CommonFrameFeatureFactory::GetInstance()->IsBlackScreen(frame))
			    {
				    AddKeyFrameCandidate(frame);
				    m_HistRef = CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
			    }
			    else
			    {
				    m_Shot++;
			    }
		    }

            //save the current frame's feature for future use
            SaveFrameFeature(frame);			 
		    return _ret;
    }

     //add a key frame candidate in the frame buffer
     void CSimpleKeyframeExtractor::AddKeyFrameCandidate(CFrame & condKey)
     {
	         CFrame newCandidate;
	         newCandidate.Copy(condKey);
             m_FrameArray.Add(newCandidate);
     }

     //save pre-frame's feature
     void CSimpleKeyframeExtractor::SaveFrameFeature(CFrame & frame)
     {
            m_isBlackScreenPre = CommonFrameFeatureFactory::GetInstance()->IsBlackScreen(frame);
     }

     //extract some key frames for the shot
     void CSimpleKeyframeExtractor::ChooseKeyframe(unsigned int frameBgnId, unsigned int frameEndId)
     {
            //clear the pre key frame list 
            m_KeyframeList.Clear();
		    //check keyframe candidate amount
		    int size = m_FrameArray.Size();
		    //if there is no key frame candidate in the frame buffer
		    if( size == 0 ) return;
    		
		    //we choose the middlest keyframe for output
		    unsigned int middle = (frameBgnId + frameEndId)>>1;
		    int num = 0;
		    for( num = 0; num < size; ++num )
		    {
			     if( unsigned long(m_FrameArray[num].Id()) >= middle )
			     {
				     unsigned long diff1 = (num > 0 ) ? middle - m_FrameArray[num-1].Id() : 0xFFFF;
				     unsigned long diff2 = m_FrameArray[num].Id() - middle;
				     if( diff1 < diff2 ) --num;
				     break;
			     }
		    }

		    //if this conditional happened, we use the last key frame
		    if( num == size ) --num;
		    //we only use key frame in the range [BgnNum, EndNum]
		    if( unsigned long(m_FrameArray[num].Id()) > frameEndId ) 
			    return;
    		
		    //save the key frame
		    m_KeyframeList.Add(m_FrameArray[num]);

		    //reset keyframes for next shot
		    m_FrameArray.Clear();
      }

}