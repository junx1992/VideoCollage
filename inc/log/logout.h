

#pragma once
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )

#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_LENGTH		1024
#define MAX_CHANNEL		2

#define CHANNEL_INFO	0
#define CHANNEL_WARN	1
#define CHANNEL_ERRO	2
#define CHANNEL_ALGO	3

typedef enum _TLogPriority
{
	LOG_LOW			= 0,
	LOG_NORMAL		= 1,
	LOG_HIGH		= 2,
	LOG_CRITICAL	= 3
} TLogPriority;

BOOL MakeFullFileNameInBin(WCHAR *wszFullFileName, const WCHAR *wszFileName, DWORD nSize);
BOOL MakeFullFileNameInLog(WCHAR *wszFullFileName, const WCHAR *wszFileName, DWORD nSize);

class CLogChannel
{
protected:
	FILE* _log;

protected:
	UINT _line;
	WCHAR _date[9];
	WCHAR _time[9];
	WCHAR _file[MAX_LENGTH];
	WCHAR _func[MAX_LENGTH];

protected:
	CLogChannel();

	HRESULT Open(const WCHAR *wszFileName);
	HRESULT Write(const TLogPriority tPrior, const WCHAR *wszBuffer);
	HRESULT Close();
	
	BOOL isOpened()
	{
		return (NULL != _log);
	}

	friend class CLogCollection;
};

class CLogCollection
{
protected:
	UINT _iCurrentChannel;
	CLogChannel* _channels;

protected:
	CLogCollection();
	~CLogCollection();
	CLogCollection(const CLogCollection &src);
	CLogCollection& operator=(const CLogCollection &rhs);
	
public:
	static CLogCollection& Instance()
	{
		static CLogCollection _instance;
		return _instance;
	}

public:
	HRESULT Initialize();
	HRESULT Open(const WCHAR *wszFullFileName, const int iChannelIndex);
	HRESULT Write(const TLogPriority tPrior, const WCHAR *wszFormat, ...);
	HRESULT Close(const int iChannelIndex);
	HRESULT WriteHeader(const int iChannelIndex, const WCHAR *_FILE, const LONG _LINE, const WCHAR *_FUNC);
};

#define	WIDEN2(x)	L ## x
#define WIDEN(x)	WIDEN2(x)

#define __WFILE__	WIDEN(__FILE__)
#define __WFUNC__	WIDEN(__FUNCTION__)

#if defined(UNICODE) || defined(_UNICODE)
#define	__TFILE__	__WFILE__
#define	__TFUNC__	__WFUNC__
#else
#define	__TFILE__	__FILE__
#define	__TFUNC__	__FUNCTION__
#endif

#if defined(LOGINFO_ON) || defined(LOGALGO_ON)
	#define LOG_INITIALIZE		CLogCollection::Instance().Initialize();
	#define LOG_SET(idx)		CLogCollection::Instance().WriteHeader(idx, __WFILE__, __LINE__, __WFUNC__);
	#define LOG					CLogCollection::Instance().Write
#else
	#define LOG_INITIALIZE		((void)(0));
	#define LOG_SET(idx)		((void)(0));
	#define LOG					
#endif

#if defined(LOGINFO_ON)
	#define LOGINFO_OPEN(fn)	CLogCollection::Instance().Open(fn, CHANNEL_INFO);
	#define LOGINFO_CLOSE		CLogCollection::Instance().Close(CHANNEL_INFO);	
	#define LOGINFO				LOG_SET(CHANNEL_INFO) LOG
#else
	#define LOGINFO_OPEN(fn)	((void)(0));
	#define LOGINFO_CLOSE		((void)(0));
	#define LOGINFO				
#endif

#if defined(LOGALGO_ON)
	#define LOGALGO_OPEN(fn)	CLogCollection::Instance().Open(fn, CHANNEL_ALGO);
	#define LOGALGO_CLOSE		CLogCollection::Instance().Close(CHANNEL_ALGO);
	#define LOGALGO				LOG_SET(CHANNEL_ALGO) LOG
#else
	#define LOGALGO_OPEN(fn)	((void)(0));	
	#define LOGALGO_CLOSE		((void)(0));
	#define LOGALGO				
#endif

//	Define fail then only log
#define LOG_ERROR(msg, hr)					\
	if ( FAILED(hr) )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##, hr);	\
	}
#define LOG_ERROR_MEM(msg, obj)				\
	if ( NULL == obj )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##);		\
	}
#define LOG_ERROR_PTR(msg, obj)				\
	if ( NULL == obj )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##);		\
	}

//	Define fail then log and break
#define LOG_ERROR_BRK(msg, hr)				\
	if ( FAILED(hr) )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##, hr);	\
		break;								\
	}
#define LOG_ERROR_MEM_BRK(msg, obj)			\
	if ( NULL == obj )						\
	{										\
		hr = E_OUTOFMEMORY;					\
		LOGINFO(LOG_HIGH, L##msg##);		\
		break;								\
	}
#define LOG_ERROR_PTR_BRK(msg, obj)			\
	if ( NULL == obj )						\
	{										\
		hr = E_POINTER;						\
		LOGINFO(LOG_HIGH, L##msg##);		\
		break;								\
	}

//	Define fail then log and return
#define LOG_ERROR_RET(msg, hr)				\
	if ( FAILED(hr) )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##, hr);	\
		return hr;							\
	}
#define LOG_ERROR_PTR_RET(msg, obj)			\
	if ( NULL == obj )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##);		\
		return E_POINTER;					\
	}
#define LOG_ERROR_MEM_RET(msg, obj)			\
	if ( NULL == obj )						\
	{										\
		LOGINFO(LOG_HIGH, L##msg##);		\
		return E_OUTOFMEMORY;				\
	}