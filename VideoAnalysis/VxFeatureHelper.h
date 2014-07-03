/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the helper functions for image analysis.

Notes:

History:
  Created on 04/17/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxFeature.h"
#include <iostream>
#include <iomanip>
#include <vector>

namespace VxCore
{
	///copy feature of one type to another, only copy date
	///ensure that the size of src and dest must be the same
	template<class T1, class T2>
	void CopyFeatureData(T2& dest, const T1& src)
	{
		_ASSERT(dest.Size() == src.Size());
		for(int i=0; i<src.Size(); ++i)
			dest[i] = src[i];
	}

	///clone one feature to another type
	///copy not only the data but also the id
	template<class TR, class T>
	TR CloneFeature(const T& src)
	{
		TR ret(src.Id(), src.Size());
		CopyFeatureData(ret, src);
		return ret;

	}


	///load a vector into a VectorClass
	template<class TFeature,class _Elem,
			 class _Traits> 
	void LoadVector(TFeature& feature, std::basic_istream<_Elem, _Traits>& in, bool text_mode=true)
	{
		if(text_mode)
		{
			int size = 0;
			in >> size;		
			if(!in.good() || size==0 )
				throw persistence_exception("the feature file is bad");
			feature.Resize(size);
			
			_Elem c = 0;
			in >>c;
			_ASSERT(in.good() && c == in.widen(':'));
			if(!(in.good() && c == in.widen(':')))
				throw persistence_exception("the feature file is bad");
			for(int i = 0; i<feature.Size(); ++i)
			{
				typename TFeature::ElementType d;
				in >> d;
				if(in.good())
				{
					feature[i] = d;
				}
				else
					throw persistence_exception("the feature file is bad");
			}
		}
		else
		{
			_ASSERT(sizeof(_Elem) == 1);
			int size = 0;
			in.read((_Elem*)&size, sizeof(int) );		
						
			if(!in.good())
				throw persistence_exception("the feature file is bad");

			feature.Resize(size);
			in.read((_Elem*)feature.DataPtr(), sizeof(typename TFeature::ElementType)*feature.Size() );
			if(!in.good())
				throw persistence_exception("the feature file is bad");
					
		}
	}
	///load a feature into a FeatureClass
	template<class TFeature,class _Elem,
			 class _Traits> 
	void LoadFeature(TFeature& feature, std::basic_istream<_Elem, _Traits>& in, bool text_mode=true)
	{
		if(text_mode)
		{
			int size = 0, id = 0;
			in >> id;
			feature.Id(id);

			_Elem c = 0;
			in >>c;
			if(!in.good() || c != in.widen(':'))
				throw persistence_exception("the feature file is bad");
		}
		else
		{
			int id = 0;
			in.read((_Elem*)&id, sizeof(id));
			if(!in.good())
				throw persistence_exception("the feature file is bad");
			/*_Elem c = 0;
			in.read((_Elem*)&c, sizeof(c));
			_ASSERT(c == CharDictionary<_Elem>::COLON);*/
			feature.Id(id);
		}
		
		LoadVector(feature, in, text_mode);
	}

	///load the date into a std::vector of VectorClass
	template<class TFeature,class _Elem,
			 class _Traits> 
	void LoadVectorVector(std::vector<TFeature>& features, std::basic_istream<_Elem, _Traits>& in, bool text_mode=true)
	{
		int count = 0;
		if(text_mode)
		{
			in >> count;
		}
		else
		{
			in.read((_Elem*)&count, sizeof(count));
		}
		if(!in)
			throw persistence_exception("the feature file is bad");

		features.resize(count);
		for(int i = 0; i < count; ++i)
		{
			LoadVector(features[i], in, text_mode);
		}
	}

	///load a feature vector into a FeatureClass vector
	template<class TFeature,class _Elem,
			 class _Traits> 
	void LoadFeatureVector(std::vector<TFeature>& features, std::basic_istream<_Elem, _Traits>& in, bool text_mode=true)
	{
		int count = 0;
		if(text_mode)
		{
			in >> count;
		}
		else
		{
			in.read((_Elem*)&count, sizeof(count));
		}
		if(!in)
			throw persistence_exception("the feature file is bad");
		features.resize(count);
		for(int i = 0; i < count; ++i)
		{
			LoadFeature(features[i], in, text_mode);
		}
	}

	 ///save a vector into a stream
	template<class TFeature,class _Elem,
			 class _Traits> 
	void SaveVector(const TFeature& feature, std::basic_ostream<_Elem, _Traits>& out, bool text_mode=true)
	{
		if(text_mode)
		{
			out<< std::setiosflags(std::ios_base::left)<< std::setw( 3 ) << feature.Size() << ":";
			for(int i=0; i<feature.Size(); ++i)
				out << std::setiosflags( std::ios::fixed ) 
					<< std::setiosflags(std::ios_base::left)
					<< std::setw( 10 ) 
					<< std::setprecision( 6 )
					<< feature[i] << " ";
			out<<std::endl;
		}
		else
		{
			_ASSERT(sizeof(_Elem) == 1);
			int s = feature.Size();
			out.write((_Elem*)&s, sizeof(s) );
			out.write((_Elem*)feature.DataPtr(), sizeof(typename TFeature::ElementType)*feature.Size() );
		}
		if(!out)
				throw persistence_exception("the feature file is bad");
	}

	///save a feature into a stream
	template<class TFeature,class _Elem,
			 class _Traits> 
	void SaveFeature(const TFeature& feature, std::basic_ostream<_Elem, _Traits>& out, bool text_mode=true)
	{
		if(text_mode)
			out<< std::setiosflags(std::ios_base::left) << std::setw( 6 ) <<feature.Id() << ": ";
		else
		{
			int id = feature.Id();
			out.write((_Elem*)&id, sizeof(id));
			/*_Elem c =  CharDictionary<_Elem>::COLON;
			out.write((_Elem*)&c, sizeof(c));*/
		}
		if(!out)
				throw persistence_exception("the feature file is bad");
		SaveVector(feature, out, text_mode);
	}

	///save a std::vector of VectorClass into a stream
	template<class TFeature,class _Elem,
			 class _Traits> 
	void SaveVectorVector(const std::vector<TFeature>& features, std::basic_ostream<_Elem, _Traits>& out, bool text_mode=true)
	{
		//in the beginning of this file, output the feature dimensionality and the feature count
		if(features.size() > 0)
		{
			if(text_mode)
				out<< features.size() << std::endl;
			else
			{
				size_t s = features.size();
				out.write((_Elem*)&s, sizeof(s) );
			}
		}
		if(!out)
				throw persistence_exception("the feature file is bad");
		for(std::vector<TFeature>::size_type i=0; i<features.size(); ++i)
		{
			SaveVector(features[i], out, text_mode);
		}
	}

	///save a FeatureClass vector into a stream
	template<class TFeature,class _Elem,
			 class _Traits> 
	void SaveFeatureVector(const std::vector<TFeature>& features, std::basic_ostream<_Elem, _Traits>& out, bool text_mode=true)
	{
		//in the beginning of this file, output the feature dimensionality and the feature count
		if(features.size() > 0)
		{
			if(text_mode)
				out<< features.size() << std::endl;
			else
			{
				size_t s = features.size();
				out.write((_Elem*)&s, sizeof(s) );
			}
		}
		if(!out)
				throw persistence_exception("the feature file is bad");
		for(std::vector<TFeature>::size_type i=0; i<features.size(); ++i)
		{
			SaveFeature(features[i], out, text_mode);
		}
	}
}
