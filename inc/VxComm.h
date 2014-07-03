/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Sdk

Abstract:
  This header file provide the common definitions and functions used by video sdk and image sdk 

Notes:
  

History:
  Created on 04/12/2007 by linjuny@microsoft.com
\******************************************************************************/

#pragma once

//some macros
#define DLL_IN_EXPORT

//#ifdef DLLEXPORT_MODULE
//#define DLL_IN_EXPORT
//#else
//#define DLL_IN_EXPORT __declspec(dllimport)
//#endif

#define SAFE_RELEASE(p) if(p){p->Release(); p=NULL;}

#ifndef _WINDOWS_
typedef unsigned char BYTE;
typedef long		  HRESULT;
#define NULL		  0
#endif

#define EXCEPTION_IF_FAIL(hr, ExceptionClass, exception_message) \
	{if(FAILED(hr)) {throw ExceptionClass(exception_message);} }

#define RETURN_IF_FAIL(hr) \
	{if(FAILED(hr)) {return hr;} }
#define BREAK_IF_FAIL(hr) \
	{if(FAILED(hr)) break; }

#define SAFE_DELETE_ARRAY(x)				 \
	if (x)									                 \
	{														 \
		delete[] x;							             \
		x = NULL;							             \
	}

#define SAFE_NEW_ARRAY(x, type, size)  	\
	try										               \
	{										                   \
		x = new type[size];					       \
	}										                   \
	catch ( std::bad_alloc &)				   \
	{										                   \
		throw;				                               \
	}

///Define Safe allocate					
#define SAFE_NEW_PTR(x, type)			  \
	try													  \
	{														  \
		x = new type;						          \
	}										                  \
	catch ( std::bad_alloc &)				  \
	{										                  \
		throw;										      \
	}

///Define Safe de-allocate
#define SAFE_DELETE_PTR(x)					\
	if (x)													\
	{														\
		delete x;										\
		x = NULL;										\
	}