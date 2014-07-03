
#pragma once	

#pragma warning ( disable : 4101 )

#include <new>

//	Define Safe deallocate
#define MHL_SAFE_RELEASE(x)						\
	if (x)									\
	{										\
		x->Release();						\
		x = NULL;							\
	}
#define SAFE_DELETE_PTR(x)					\
	if (x)									\
	{										\
		delete x;							\
		x = NULL;							\
	}
#define SAFE_DELETE_ARRAY(x)				\
	if (x)									\
	{										\
		delete[] x;							\
		x = NULL;							\
	}
#define SAFE_CLOSE(x)						\
	if (x)									\
	{										\
		CloseHandle(x);						\
		x = NULL;							\
	}

//	Define Safe allocate					
#define MHL_SAFE_NEW_PTR(x, type)				\
	try										\
	{										\
		x = new type;						\
	}										\
	catch ( std::bad_alloc &ba)				\
	{										\
		SAFE_DELETE_PTR(x);					\
	}										

#define MHL_SAFE_NEW_ARRAY(x, type, size)		\
	try										\
	{										\
		x = new type[size];					\
	}										\
	catch ( std::bad_alloc &ba)				\
	{										\
		SAFE_DELETE_ARRAY(x);				\
	}