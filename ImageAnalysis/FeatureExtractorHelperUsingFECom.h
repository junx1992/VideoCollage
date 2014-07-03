/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the implementation of feature extractor class based on the FE Com.

Notes:

History:
  Created on 04/16/2007 by linjuny@microsoft.com
\******************************************************************************/

#pragma once
#include <atlbase.h>
#include "FeatureExtractorComInc\FeatureExtractor.h"
#include "ImageAnalysisException.h"

namespace ImageAnalysisHelper
{
	class CFeatureExtractorHelperUsingFECom
	{
	public:
		CFeatureExtractorHelperUsingFECom()
		{
			CoInitialize(NULL);
			HRESULT hr = m_FeatureExtractor.CoCreateInstance( CLSID_FeatureExtractor, NULL, CLSCTX_INPROC_SERVER );
			if (FAILED(hr)) 
			{
				throw ImageAnalysis::feature_extractor_exception("can't find the feature extractor dll, please contact linjuny@microsoft.com");
			}
		}
		~CFeatureExtractorHelperUsingFECom()
		{
			m_FeatureExtractor.Release();
			CoUninitialize();
		}
		CComPtr<IFeatureExtractor> operator ->()
		{
			return m_FeatureExtractor;
		}
	private:
		CComPtr<IFeatureExtractor> m_FeatureExtractor;
	};
}