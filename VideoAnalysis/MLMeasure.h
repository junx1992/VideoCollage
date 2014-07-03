/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  MachineLearning Sdk

Abstract:
  This header file provide the classes for measure, including AP. 

Notes:
  

History:
  Created on 07/30/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VxFeature.h"

namespace MachineLearning
{
	using VxCore::CDoubleVector;
	using VxCore::CIntVector;
	///Interface for Measure
	class DLL_IN_EXPORT IMeasure
	{
	public:
		///compute the measure
		virtual double Compute(const CDoubleVector& prediction, const CIntVector& groundTruth) const = 0;
		virtual ~IMeasure(void) = 0 {}
	};

	class DLL_IN_EXPORT CAPMeasure: public IMeasure
	{
	public:
		///apNumber is to indicate the maximum returned number of samples when computing AP. -1 indicates 
		///all the returned samples are considered, i.e. APAll.
		CAPMeasure(int apNumber=-1);
		///prediction is the score of samples, while groundTruth is the corresponding label (positive value means postive sample, while 
		///non-positive value means negtive sample).
		///return: the computed ap. However; if NO positice samples occur in the groundTruth, return -1;
		virtual double Compute(const CDoubleVector& prediction, const CIntVector& groundTruth) const;
	private:
		int m_apNum;
	};
}