/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  MachineLearning Sdk

Abstract:
  This header file provide the interface and base classes for the machine learning algorithm 

Notes:
  

History:
  Created on 07/04/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VxDataSet.h"

namespace MachineLearning
{
	using VxCore::CDataSet;
	using VxCore::CDoubleFeature;
	using VxCore::CDoubleVector;

	///interface for the parameter of the machine learning algorithms
	class DLL_IN_EXPORT IMachineParam
	{
	public:
		virtual ~IMachineParam()=0 {}
	};
	///interface for Learning Machine.
	class DLL_IN_EXPORT IMachine
	{
	public:	
		///Train a model.
		/**Train a model.
		*\param CDataSet* pTrainingSet: the training set;
		*\param IMachineParams* pParams: the parameter to train the model;
		*\param CDataSet* pTestingSet: the test test, the parameter is for semi-supervised learning method;
		*\param CDataSet* pAuxiliaryData; the auxuluary data,the parameter is for transferring learning method that can utilize 
		*								  some auxiliary data. 
		*/
		virtual void Train(CDataSet* pTrainingSet, IMachineParam* pParam=NULL, CDataSet* pTestingSet=NULL, CDataSet* pAuxiliaryData=NULL) = 0;
		///Predict the target for a given sample.
		/**Predict the target for a given sample.
		*\param CCDoubleFeature& sample: the test sample;
		*\return the target of the input sample. Maybe some auxiliary information such as the confidene can be placed in the returned vector.
		*/
		virtual CDoubleVector Predict(const CDoubleFeature& sample) = 0;
		///load model from the file.
		virtual HRESULT LoadModel(const wchar_t* fileName) = 0;
		///Save model
		virtual HRESULT SaveModel(const wchar_t* fileName) = 0;
	public:
		virtual ~IMachine()=0 {}
	};
}