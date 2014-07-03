/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  VxCore

Abstract:
  This header file provide the interface of DataSet 

Notes:
  

History:
  Created on 06/28/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VxFeature.h"

namespace VxCoreHelper
{
	class CDataSetImp;
}

namespace VxCore
{
	///interface for DataSet
	class DLL_IN_EXPORT IDataSet
	{
	public:
		///get the sample.
		virtual CDoubleFeature& GetSample(unsigned int i) = 0;
		///get the sample.
		virtual const CDoubleFeature& GetSample(unsigned int i) const = 0;
		///get the corresponding target for the sample i.
		virtual CDoubleVector&  GetTarget(unsigned int i)  = 0;
		///get the corresponding target for the sample i.
		virtual const CDoubleVector&  GetTarget(unsigned int i) const = 0;
		///get the sample by ID. Return NULL if the sample cannot be found.
		virtual CDoubleFeature* GetSampleById(unsigned int id) = 0;
		///get the sample by ID. Return NULL if the sample cannot be found.
		virtual const CDoubleFeature* GetSampleById(unsigned int id) const = 0;
		///get the corresponding target for the sample by ID. Return NULL if the sample cannot be found.
		virtual CDoubleVector*  GetTargetById(unsigned int id)  = 0;
		///get the corresponding target for the sample by ID. Return NULL if the sample cannot be found.
		virtual const CDoubleVector*  GetTargetById(unsigned int id) const = 0;
		///add a sample without target
		virtual void AddSample(const CDoubleFeature& sample) = 0;
		///add a sample with target
		virtual void AddSample(const CDoubleFeature& sample, const CDoubleVector& target) = 0;
		///set targets. must set the target for all the samples in the dataset simultaneously.
		void SetTargets(const CDoubleVector* pTargets, unsigned int num);
		///set targets which is just a signle value.
		void SetTargets(const double* pTargets, unsigned int num);
		///clear all the samples
		virtual void Clear() = 0;
		///get the total number of samples in this dataset.
		virtual unsigned int GetSampleNum() const = 0;
		///determine whether the samples in the dataset has labels.
		/**Only two situations can happend: (1) all the samples in the dataset have been labeled. (2) None are labeled.
		*/
		virtual bool IsLabeled() const = 0;
		///load the dataset from the file.
		virtual HRESULT Load(const wchar_t* fileName, bool bTextMode=true) = 0;
		///save the dataset into the file.
		virtual HRESULT Save(const wchar_t* fileName, bool bTextMode=true) const = 0;
		///destructor
		virtual ~IDataSet() = 0 {}
	};

	///DataSet with all the data in memory.
	class DLL_IN_EXPORT CDataSet: public IDataSet
	{
	public:
		CDataSet();
		~CDataSet();
		CDataSet(const CDataSet&);
		CDataSet& operator=(const CDataSet&);
		
		//implementation of IDataSet interface
		virtual CDoubleFeature& GetSample(unsigned int i);
		virtual const CDoubleFeature& GetSample(unsigned int i) const;
		virtual CDoubleVector&  GetTarget(unsigned int i);
		virtual const CDoubleVector&  GetTarget(unsigned int i) const;
		virtual CDoubleFeature* GetSampleById(unsigned int id);
		virtual const CDoubleFeature* GetSampleById(unsigned int id) const;
		virtual CDoubleVector*  GetTargetById(unsigned int id);
		virtual const CDoubleVector*  GetTargetById(unsigned int id) const;
		virtual void AddSample(const CDoubleFeature& sample);
		virtual void AddSample(const CDoubleFeature& sample, const CDoubleVector& target);
		void SetTargets(const CDoubleVector* pTargets, unsigned int num);
		void SetTargets(const double* pTargets, unsigned int num);
		virtual void Clear();
		virtual unsigned int GetSampleNum() const;
		virtual bool IsLabeled() const ;
		virtual HRESULT Load(const wchar_t* fileName, bool bTextMode=true);
		virtual HRESULT Save(const wchar_t* fileName, bool bTextMode=true) const;
	private:
		VxCoreHelper::CDataSetImp* m_pImp;
	};

	template<class Func>
	void ForEach(IDataSet& dataset, Func& f)
	{
		for(unsigned int i=0;i<dataset.GetSampleNum(); ++i)
		{
			f(dataset.GetSample(i));
		}
	}
}