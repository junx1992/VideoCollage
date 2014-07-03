#include "stdafx.h"
#include "VxFeature.h"

namespace VxCoreHelper
{
	int GetLarger2PowThan(int s)
	{
		unsigned int b = 0;
		while(s > 0)
		{
			s = s >> 1;
			++b;
		}
		return 1<<b;
	}
}
namespace VxCore
{ 
#define GENERATED_VECTOR_CLASS_IMPLEMENTATION(VectorClass, T)\
	VectorClass::VectorClass()\
	{m_data = NULL; m_dataSize = 0; m_capacity = 0;}\
	VectorClass::VectorClass(const int size, const T init_val)\
	{\
		m_data = new T[size];\
		for(int i=0; i<size; ++i) m_data[i] = init_val;\
		m_dataSize = size;\
		m_capacity = m_dataSize;\
	}\
	VectorClass::~VectorClass()\
	{\
		delete[] m_data;\
	}\
	VectorClass::VectorClass(const VectorClass& feature)\
	{\
		m_dataSize = feature.m_dataSize;\
		m_data = new T[m_dataSize];\
		memcpy(m_data, feature.m_data, m_dataSize*sizeof(T));\
		m_capacity = m_dataSize; \
	}\
	VectorClass& VectorClass::operator=(const VectorClass& feature)\
	{\
		if(this == &feature || m_data == feature.m_data)\
			return *this;\
		delete[] m_data;\
		m_dataSize = feature.m_dataSize;\
		m_data = new T[m_dataSize];\
		memcpy(m_data, feature.m_data, m_dataSize*sizeof(T));\
		m_capacity = m_dataSize;\
		return *this;\
	}\
	T& VectorClass::GetAt(const int i)\
	{\
		if(i < 0 || i>= m_dataSize)\
			throw feature_access_exception("arg i is not in the range");\
		return m_data[i];\
	}\
	const T& VectorClass::GetAt(const int i) const\
	{\
		if(i < 0 || i>= m_dataSize)\
			throw feature_access_exception("arg i is not in the range");\
		return m_data[i];\
	}\
	T& VectorClass::operator[](const int i) \
	{\
		return m_data[i];\
	}\
	const T& VectorClass::operator[](const int i) const\
	{\
		return m_data[i];\
	}\
	const int VectorClass::Size() const\
	{\
		return m_dataSize;\
	}\
	const T* VectorClass::DataPtr() const\
	{\
		return m_data;\
	}\
	T* VectorClass::DataPtr()\
	{\
		return m_data;\
	}\
    void VectorClass::Clear()\
    {\
		m_dataSize = 0;\
	}\
	void VectorClass::PushBack(const T val)\
	{\
		if(m_dataSize >= m_capacity)\
		{\
			Resize(m_dataSize+1);\
			m_data[m_dataSize-1] = val;\
		}\
		else\
			m_data[m_dataSize++] = val;\
	}\
	void VectorClass::Resize(const int size, const T init_val)\
	{\
		if(size > m_capacity )\
		{\
			m_capacity = VxCoreHelper::GetLarger2PowThan(size);\
			T* buf = new T[m_capacity];\
			for(int i=0; i<m_dataSize; ++i)\
				buf[i] = m_data[i];\
			delete[] m_data;\
			m_data = buf;\
		}\
		for(int i=m_dataSize; i<size; ++i)\
			m_data[i] = init_val;	\
		m_dataSize = size;\
	}


#define GENERATED_FEATURE_CLASS_IMPLEMENTATION(FeatureClass, BaseVectorClass)\
	FeatureClass::FeatureClass(const int id): m_id(id)\
	{}\
	FeatureClass::FeatureClass(const int id, const int size, const T init_val)\
		:m_id(id), BaseVectorClass(size, init_val)\
	{\
	}\
	FeatureClass::~FeatureClass()\
	{\
	}\
	FeatureClass::FeatureClass(const FeatureClass& feature)\
		:BaseVectorClass(feature)\
	{\
		m_id = feature.m_id;\
	}\
	FeatureClass& FeatureClass::operator=(const FeatureClass& feature)\
	{\
		BaseVectorClass::operator=(feature);\
		m_id = feature.m_id;\
		return *this;\
	}\
	const int FeatureClass::Id() const\
	{\
		return m_id;\
	}\
	void FeatureClass::Id(int id)\
	{\
		m_id = id;\
	}\
    void FeatureClass::Clear()\
	{\
       BaseVectorClass::Clear();\
	}\
   

//#define GENERATED_LABELED_FEATURE_CLASS_IMPLEMENTATION(LabeledFeatureClass, BaseFeatureClass) \
//	LabeledFeatureClass::LabeledFeatureClass(const int id)\
//		:BaseFeatureClass(id) \
//	{m_label = UNDEFINED_LABEL;}\
//	LabeledFeatureClass::LabeledFeatureClass(const int id, const int size, const T init_val)\
//		:BaseFeatureClass(id, size, init_val) \
//	{m_label = UNDEFINED_LABEL;}\
//	const int LabeledFeatureClass::Label() const\
//	{ return m_label; }\
//	void LabeledFeatureClass::Label(const int label)\
//	{ m_label = label; }

GENERATED_VECTOR_CLASS_IMPLEMENTATION(CDoubleVector, double)
GENERATED_VECTOR_CLASS_IMPLEMENTATION(CIntVector, int)
GENERATED_VECTOR_CLASS_IMPLEMENTATION(CFloatVector, float)
GENERATED_FEATURE_CLASS_IMPLEMENTATION(CDoubleFeature, CDoubleVector)
GENERATED_FEATURE_CLASS_IMPLEMENTATION(CIntFeature, CIntVector)
GENERATED_FEATURE_CLASS_IMPLEMENTATION(CFloatFeature, CFloatVector)
//GENERATED_LABELED_FEATURE_CLASS_IMPLEMENTATION(CLabeledDoubleFeature, CDoubleFeature)
//GENERATED_LABELED_FEATURE_CLASS_IMPLEMENTATION(CLabeledFloatFeature, CFloatFeature)
//GENERATED_LABELED_FEATURE_CLASS_IMPLEMENTATION(CLabeledIntFeature, CIntFeature)

}