/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  MachineLearning Sdk

Abstract:
  This header file provide the class for SVM 

Notes:
  

History:
  Created on 07/05/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "Machine.h"

namespace MachineLearningImp
{
	class CSVMImp;
}

namespace MachineLearning
{
	using VxCore::CIntVector;
	using VxCore::CDoubleVector;
	///class for SVM parameters
	class DLL_IN_EXPORT CSVMParam: IMachineParam
	{
	public:
		enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
		enum { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED };//kernel type;

		int svm_type;
		int kernel_type;
		int degree;	/* for poly */
		double gamma;	/* for poly/rbf/sigmoid */
		double coef0;	/* for poly/sigmoid */

		/* these are for training only */
		double cache_size; /* in MB */
		double eps;	/* stopping criteria */
		double C;	/* for C_SVC, EPSILON_SVR and NU_SVR */
		CIntVector weight_label;	/* for C_SVC */
		CDoubleVector weight;		/* for C_SVC */
		double nu;	/* for NU_SVC, ONE_CLASS, and NU_SVR */
		double p;	/* for EPSILON_SVR */
		int shrinking;	/* use the shrinking heuristics */
		int probability; /* do probability estimates */

		CSVMParam();
	};

	///class for SVM
	class DLL_IN_EXPORT CSVM: public IMachine
	{
	public:
		CSVM();
		~CSVM();
		///train the model.
		virtual void Train(CDataSet* pTrainingSet, CSVMParam* pParams=NULL);
		//implementation of IMachine
		/**
		*\return 2-dim vector. the first element is the predicted label. and the second element is the score 
		*					   (only effective for two-class SVC).
		*/
		virtual CDoubleVector Predict(const CDoubleFeature& sample);
		virtual HRESULT LoadModel(const wchar_t* fileName);
		virtual HRESULT SaveModel(const wchar_t* fileName);

	private:
		CSVM(const CSVM&);
		CSVM& operator=(const CSVM&);

	private:
		//implementation of IMachine
		virtual void Train(CDataSet* pTrainingSet, IMachineParam* pParams=NULL, CDataSet* pTestingSet=NULL, CDataSet* pAuxiliaryData=NULL);

	private:
		MachineLearningImp::CSVMImp* m_pImp;
	};
}