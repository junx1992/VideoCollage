#include "Stdafx.h"
#include "IImage.h"
#include "RgbImage.h"
#include "VideoSegments.h"
#include "KeyframeSaver.h"
#include <cstdio>

namespace VideoAnalysis
{
    using namespace std;
    using namespace ImageAnalysis;    
	
    CKeyframeSaver::CKeyframeSaver(const wchar_t * const saveDir, int width, int height)
	{
		//save the key frame saving dir
		swprintf_s(m_wszSaveDir, MAX_PATH, L"%s", saveDir);
        m_Width = width;
        m_Height= height;
	}

    HRESULT CKeyframeSaver::OnNewSegment(IVideoSegment& segment)
    {
        int size = segment.GetKeyframeNum();
        for( int i = 0; i < size; ++i )
        {
             //get the i-th keyframe
             CFrame * frame = segment.GetKeyframe(i);
             //make the key frame's name
			 wchar_t wszKeyFrameFileName[MAX_PATH];
		     swprintf_s(wszKeyFrameFileName, MAX_PATH, L"%s\\%d.jpg", m_wszSaveDir, frame->Id());
			           
             //save the key frame
             CRgbImage * newImage = frame->GetImage();
             if( m_Width > 0 && m_Height > 0 )
             {
                 newImage = frame->GetImage()->Resize(m_Width, m_Height);
                 if( newImage == NULL )
                     newImage = frame->GetImage();
             }

             if( FAILED(newImage->Save(wszKeyFrameFileName)) )
             {
                 if( newImage != frame->GetImage() )
                     newImage->Release();
                 return S_FALSE;
             }

             if( newImage != frame->GetImage() )
                 newImage->Release();
       }

	   return S_OK;         
    }



    HRESULT CKeyframeSaver::EndOfStream(){  return S_OK; }
}