#include "stdafx.h"
#include "FrameFeatureExtractor.h"
#include "IImage.h"
using namespace VxCore;
using namespace ImageAnalysis;

namespace VideoAnalysis
{
	CFrameFeatureExtractor::CFrameFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE)
	{
		SetImageFeatureExtractor(pImgFE);
	}

	void CFrameFeatureExtractor::SetImageFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE)
	{
		m_pImgFE = pImgFE;
	}

	HRESULT CFrameFeatureExtractor::OnNewSegment(IVideoSegment& segment)
	{
		if(m_pImgFE != NULL)
		{
			for( int i = 0; i < segment.GetKeyframeNum(); ++i )
			{
				CDoubleFeature f = m_pImgFE->Extract(*segment.GetKeyframe(i)->GetImage(), segment.GetKeyframe(i)->Id());
				m_Data.AddSample(f);
			}
		}
		return S_OK;
	}

	HRESULT CFrameFeatureExtractor::EndOfStream()
	{
		return S_OK;
	}

	const VxCore::IDataSet& CFrameFeatureExtractor::GetData() const
	{
		return m_Data;
	}
	void CFrameFeatureExtractor::Clear()
	{
		m_Data.Clear();
	}
}