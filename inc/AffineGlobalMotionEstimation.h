/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the class for affine global motion Estimation 

Notes:
  

History:
  Created on 06/22/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include "VideoParseInterface.h"
#include "VxFeature.h"
#include "VxDataSet.h"


namespace VideoAnalysisHelper
{
    ///the implemention class of the AffineGlobalMotionEstimation
    class CAffineGlobalMotionEstimationImp;
}

namespace VideoAnalysis
{
    ///typedef for affine global motion
    typedef  VxCore::CDoubleFeature					CAffineGlobalMotion;
    typedef  VxCore::CDataSet                       CAffineGlobalMotionSequence;

    ///The class for affine global motion estimaion
    class DLL_IN_EXPORT CAffineGlobalMotionEstimation: public IVideoFeatureExtractor
    {
	public:
		CAffineGlobalMotionEstimation(int step = 1);
		~CAffineGlobalMotionEstimation(void);

    public:
        //implementation of IVideoParseReceiver
        virtual HRESULT OnNewSegment(IVideoSegment& segment);        
        virtual HRESULT EndOfStream();

  		///get the extracted feature sequence.
        ///every item in DataSet is 7-dimensional 
        ///
        ///in the Affine Model
        /// X' = P0 + P2 * X + P3 * Y
		/// Y' = P1 + P4 * X + P5 * Y
        /// so 
        /// [0] = P0;
		/// [1] = P[2]-1.0;
		/// [2] = P[3];
		/// [3] = P[1];
		/// [4] = P[4];
		/// [5] = P[5]-1.0;
        /// [6] = the diff between 2 adjcent frames after GME
        ///
        /// the reason why [1] and [5] minus 1 is that we store the position as velocity
        /// Vx = (X-X')/del(t)   Vy = (Y-Y')/del(t), 
        /// del(t) is consider as 1 frame
		virtual const VxCore::IDataSet& GetData() const;
		
    protected:
        HRESULT DoMotionEstimation(CFrame & frame);
	private:
        ///the real work is done by the CAffineGlobalMotionEstimationImp
		VideoAnalysisHelper::CAffineGlobalMotionEstimationImp* m_pImp;
    };
}