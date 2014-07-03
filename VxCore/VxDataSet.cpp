#include "stdafx.h"
#include "VxDataSet.h"
#include "VxFeatureHelper.h"
#include <vector>
#include <fstream>
using namespace std;

namespace VxCoreHelper
{
	using namespace VxCore;
	class CDataSetImp
	{
	public:
		virtual HRESULT Load(const wchar_t* fileName, bool bTextMode)
		{
			m_Samples.clear();
			m_Targets.clear();
			ifstream in;
			if(bTextMode)
				in.open(fileName);
			else
				in.open(fileName,ios_base::binary);
			try
			{
				LoadFeatureVector(m_Samples, in, bTextMode);
				LoadVectorVector(m_Targets, in, bTextMode);
			}
			catch(persistence_exception&)
			{
				return E_FAIL;
			}
			return S_OK;
		}

		virtual HRESULT Save(const wchar_t* fileName, bool bTextMode) const
		{
			ofstream out;
			if(bTextMode)
				out.open(fileName);
			else
				out.open(fileName,ios_base::binary);
			if(!out) return E_FAIL;
			try
			{
				SaveFeatureVector(m_Samples, out, bTextMode);
				SaveVectorVector(m_Targets, out, bTextMode);
			}
			catch(persistence_exception&)
			{
				return E_FAIL;
			}
			return S_OK;
		}

		virtual CDoubleFeature& GetSample(unsigned int i)
		{
			_ASSERT(i>=0 && i<m_Samples.size() );
			return m_Samples[i];
		}

		virtual const CDoubleFeature& GetSample(unsigned int i) const
		{
			_ASSERT(i>=0 && i<m_Samples.size() );
			return m_Samples[i];
		}

		virtual CDoubleVector&  GetTarget(unsigned int i)
		{
			_ASSERT(i>=0 && i<m_Targets.size() );
			return m_Targets[i];
		}

		virtual const CDoubleVector&  GetTarget(unsigned int i) const
		{
			_ASSERT(i>=0 && i<m_Targets.size() );
			return m_Targets[i];
		}

		virtual CDoubleFeature* GetSampleById(unsigned int id)
		{
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				if(m_Samples[i].Id() == id)
					return &m_Samples[i];
			}
			return NULL;
		}

		virtual const CDoubleFeature* GetSampleById(unsigned int id) const
		{
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				if(m_Samples[i].Id() == id)
					return &m_Samples[i];
			}
			return NULL;
		}

		virtual CDoubleVector* GetTargetById(unsigned int id)
		{
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				if(m_Samples[i].Id() == id)
					return &m_Targets[i];
			}
			return NULL;
		}

		virtual const CDoubleVector* GetTargetById(unsigned int id) const
		{
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				if(m_Samples[i].Id() == id)
					return &m_Targets[i];
			}
			return NULL;
		}

		virtual void AddSample(const CDoubleFeature& sample)
		{
			_ASSERT( m_Targets.empty() );
			m_Samples.push_back(sample);
		}
		virtual void AddSample(const CDoubleFeature& sample, const CDoubleVector& target)
		{
			_ASSERT( m_Samples.size() == m_Targets.size() );
			m_Samples.push_back(sample);
			m_Targets.push_back(target);
		}

		void SetTargets(const CDoubleVector* pTargets, unsigned int num)
		{
			_ASSERT(m_Samples.size() == num);
			m_Targets.clear();
			m_Targets.resize(m_Samples.size());
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				m_Targets[i] = pTargets[i];
			}
		}

		void SetTargets(const double* pTargets, unsigned int num)
		{
			_ASSERT(m_Samples.size() == num);
			m_Targets.clear();
			m_Targets.resize(m_Samples.size());
			for(unsigned int i=0; i<m_Samples.size(); ++i)
			{
				m_Targets[i].PushBack(pTargets[i]);
			}
		}

		virtual void Clear()
		{
			m_Samples.clear();
			m_Targets.clear();
		}

		virtual unsigned int GetSampleNum() const
		{
			_ASSERT( (m_Samples.size() == m_Targets.size()) || m_Targets.empty() );
			return (unsigned int)m_Samples.size();
		}

		virtual bool IsLabeled() const 
		{
			_ASSERT( (m_Samples.size() == m_Targets.size()) || m_Targets.empty() );
			return !m_Targets.empty();
		}
	private:
		vector<CDoubleFeature> m_Samples; //the samples
		vector<CDoubleVector> m_Targets; //the targets. the assumption must hold: target num is equal to sample num or 0.
	};
}

namespace VxCore
{
	CDataSet::CDataSet()
	{
		m_pImp = new VxCoreHelper::CDataSetImp();
	}

	CDataSet::~CDataSet()
	{
		delete m_pImp;
	}

	CDataSet::CDataSet(const CDataSet& p)
	{
		m_pImp = new VxCoreHelper::CDataSetImp(*p.m_pImp);
	}

	CDataSet& CDataSet::operator=(const CDataSet& p)
	{
		if(m_pImp != p.m_pImp)
		{
			*m_pImp = *p.m_pImp;
		}
		return *this;
	}

	HRESULT CDataSet::Load(const wchar_t* fileName, bool bTextMode)
	{
		return m_pImp->Load(fileName, bTextMode);
	}
	
	HRESULT CDataSet::Save(const wchar_t* fileName, bool bTextMode) const
	{
		return m_pImp->Save(fileName, bTextMode);
	}

	CDoubleFeature& CDataSet::GetSample(unsigned int i)
	{
		return m_pImp->GetSample(i);
	}

	const CDoubleFeature& CDataSet::GetSample(unsigned int i) const
	{
		return m_pImp->GetSample(i);
	}

	CDoubleVector&  CDataSet::GetTarget(unsigned int i)
	{
		return m_pImp->GetTarget(i);
	}

	const CDoubleVector&  CDataSet::GetTarget(unsigned int i) const
	{
		return m_pImp->GetTarget(i);
	}

	CDoubleFeature* CDataSet::GetSampleById(unsigned int id)
	{
		return m_pImp->GetSampleById(id);
	}

	const CDoubleFeature* CDataSet::GetSampleById(unsigned int id) const
	{
		return m_pImp->GetSampleById(id);
	}

	CDoubleVector*  CDataSet::GetTargetById(unsigned int id)
	{	
		return m_pImp->GetTargetById(id);
	}

	const CDoubleVector*  CDataSet::GetTargetById(unsigned int id) const
	{
		return m_pImp->GetTargetById(id);
	}

	void CDataSet::AddSample(const CDoubleFeature& sample)
	{
		return m_pImp->AddSample(sample);
	}

	void CDataSet::AddSample(const CDoubleFeature& sample, const CDoubleVector& target)
	{
		return m_pImp->AddSample(sample, target);
	}

	void CDataSet::SetTargets(const CDoubleVector* pTargets, unsigned int num)
	{
		return m_pImp->SetTargets(pTargets, num);
	}

	void CDataSet::SetTargets(const double* pTargets, unsigned int num)
	{
		return m_pImp->SetTargets(pTargets, num);
	}

	void CDataSet::Clear()
	{
		return m_pImp->Clear();
	}

	unsigned int CDataSet::GetSampleNum() const
	{
		return m_pImp->GetSampleNum();
	}

	bool CDataSet::IsLabeled() const 
	{
		return m_pImp->IsLabeled();
	}
}