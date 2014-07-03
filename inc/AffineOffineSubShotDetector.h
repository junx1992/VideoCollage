/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for offline sub shot detect

Notes:
  

History:
  Created on 06/28/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VideoParseInterface.h"


namespace VideoAnalysisHelper
{
	class CAffineOffineSubshotDetectorImp;
}

namespace VxCore
{
    class  IDataSet;
}
namespace VideoAnalysis
{
     ///the class for affine subshot detector
    class DLL_IN_EXPORT CAffineOffineSubshotDetector: public IOfflineSubshotDetector
	{
	public:
        ///the offline sub shot detector need information of shots and GME
        ///[in] shotList: the shot list 
        ///[in] gmeSeqSet: the GME information
        CAffineOffineSubshotDetector(const CVideoSegmentList& shotList, const VxCore::IDataSet & gmeSeqSet);
        ~CAffineOffineSubshotDetector();

        ///do the sub shot detect offline, it will return a sub shot list, 
        ///!!! the returned list may be empty, if there is any error 
		const CVideoSegmentList& DetectSegmentOffline();
        const CVideoSegmentList& GetSubshots() const;
    private:
        VideoAnalysisHelper::CAffineOffineSubshotDetectorImp *m_pImp;
    };
}