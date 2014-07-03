/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Sdk

Abstract:
  This header file provide the class for frame feature extractor.

Notes:

History:
  Created on 07/04/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoParseInterface.h"
#include "VxDataSet.h"
#include "IImageFeatureExtractor.h"

namespace VideoAnalysis
{
	///extract features for frame.
	class DLL_IN_EXPORT CFrameFeatureExtractor: public IVideoFeatureExtractor
	{
	public:
		///the pointer of IImageFeatureExtractor should be released by the caller.
		CFrameFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE = NULL);
		///Set the image feature extractor delegate to extract the features for the incoming frames.
		///the pointer of IImageFeatureExtractor should be released by the caller.
		void SetImageFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE);
		virtual HRESULT OnNewSegment(IVideoSegment& segment);
	    virtual HRESULT EndOfStream();
		virtual void Clear(); // the perpous of rewrite this class
		virtual const VxCore::IDataSet& GetData() const;
	private:
		ImageAnalysis::IImageFeatureExtractor* m_pImgFE;
		VxCore::CDataSet m_Data;
	};
}