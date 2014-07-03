/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the interface of feature class 

Notes:
  fearture id starts from 0

History:
  Created on 04/16/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once

#include "VxComm.h"
#include "VxCoreException.h"

namespace VxCore
{
#define UNDEFINED_FEATURE_ID -1

#define GENERATED_VECTOR_CLASS_DECLARATION(VectorClass, T) \
	class DLL_IN_EXPORT VectorClass \
	{	\
	public: \
		typedef T ElementType;\
		VectorClass();\
		explicit VectorClass(const int size, const T init_val = 0);\
		VectorClass(const VectorClass& feature);\
		VectorClass& operator=(const VectorClass& feature);\
		virtual ~VectorClass();\
		T& GetAt(const int i);\
		const T& GetAt(const int i) const;		\
		T& operator[](const int i); \
		const T& operator[](const int i) const;\
		const int Size() const;\
		const T* DataPtr() const;\
		T* DataPtr();\
        void Clear();\
		void PushBack(const T val);\
		void Resize(const int size, const T init_val = 0);\
	private:\
		T* m_data;\
		int m_dataSize;\
		int m_capacity;\
	};

	#define GENERATED_FEATURE_CLASS_DECLARATION(FeatureClass, BaseVectorClass) \
	class DLL_IN_EXPORT FeatureClass: public BaseVectorClass \
	{	\
	public: \
		typedef BaseVectorClass::ElementType T;\
		explicit FeatureClass(const int id=UNDEFINED_FEATURE_ID);\
		explicit FeatureClass(const int id, const int size, const T init_val = 0);\
		FeatureClass(const FeatureClass& feature);\
		FeatureClass& operator=(const FeatureClass& feature);\
		virtual ~FeatureClass();\
		const int Id() const;\
		void Id(int id);\
        void Clear();\
	private:\
		int m_id;\
	};

//#define GENERATED_LABELED_FEATURE_CLASS_DECLARATION(LabeledFeatureClass, BaseFeatureClass) \
//	class DLL_IN_EXPORT LabeledFeatureClass: public BaseFeatureClass \
//	{	\
//	public: \
//		static const int UNDEFINED_LABEL = -1;\
//		LabeledFeatureClass(const int id=UNDEFINED_FEATURE_ID);\
//		explicit LabeledFeatureClass(const int id, const int size, const T init_val = 0);\
//		const int Label() const;\
//		void Label(const int label);\
//	private:\
//		int m_label;\
//	};

///vector type with the type of element is double
GENERATED_VECTOR_CLASS_DECLARATION(CDoubleVector, double)
///vector type with the type of element is int
GENERATED_VECTOR_CLASS_DECLARATION(CIntVector, int)
///vector type with the type of element is float
GENERATED_VECTOR_CLASS_DECLARATION(CFloatVector, float)
///feature type with the type of element is double
GENERATED_FEATURE_CLASS_DECLARATION(CDoubleFeature, CDoubleVector)
///feature type with the type of element is int
GENERATED_FEATURE_CLASS_DECLARATION(CIntFeature, CIntVector)
///feature type with the type of element is float
GENERATED_FEATURE_CLASS_DECLARATION(CFloatFeature, CFloatVector)
/////labeld feature type with the type of element is double
//GENERATED_LABELED_FEATURE_CLASS_DECLARATION(CLabeledDoubleFeature, CDoubleFeature)
/////labeld feature type with the type of element is int
//GENERATED_LABELED_FEATURE_CLASS_DECLARATION(CLabeledIntFeature, CIntFeature)
/////labeld feature type with the type of element is float
//GENERATED_LABELED_FEATURE_CLASS_DECLARATION(CLabeledFloatFeature, CFloatFeature)
}