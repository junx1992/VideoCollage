
/***************************************************************************************************\
Microsoft Research Asia
Copyright (c) 2006 Microsoft Corporation

Module Name:
	media helper library for direct show

Note:
	1. Don't use assignment operator on interface pointer, if you can't make sure left-hand scalor is CComPtr.
	2. If p is CComPtr, (*p) is not CComPtr!!! If use (*p) as left-hand, you have to add ref count mannully.
	3. If p is CComPtr, &p is not CComPtr either, we should add ref or release it mannully.
	4. So suggest if you want to send some pointers outward, please use DShow methods to get these 
	pointers since they can help you count ref definitly right.

History:
	Created	on 14/9/2005	by hwei@microsoft.com

\***************************************************************************************************/

#pragma once

#include <streams.h>    //  For direct show
#include <dmoreg.h>     //  For dmo wrapper guid
#include <dmodshow.h>   //  For dmo wrapper interface

namespace mhl
{
    //  @group UI 
    HRESULT SetVideoWindow(
        __deref_in IGraphBuilder *pGraph,
        __in UINT uiIdx,
        __in HWND hVideoWindow
        );

    //  @group add filter into graph
    HRESULT AddDMOToGraph(
        __out IDMOWrapperFilter **ppDMOWrapperFilter,
        __out IPin **ppOut,
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IPin *pIn,
        __in REFGUID guidOfCategory,
        __in REFCLSID clsidOfDMO,
        __in LPCWSTR wszName
        );
    HRESULT AddTeeToGraph(
        __out IPin **ppPrePin,
        __out IPin **ppCapPin,
        __deref_in IGraphBuilder *pGraph,
        __deref_in IPin *pIn
        );

	//	@group connect methods
	HRESULT ConnectFilters(
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IPin *pOut, 
        __deref_in IBaseFilter *pDst
        );
	HRESULT ConnectFilters(
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IBaseFilter *pSrc, 
        __deref_in IBaseFilter *pDst
        );
    HRESULT ConnectToNullRender(
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IPin *pPin 
        );
    HRESULT ConnectToNullRender(
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IBaseFilter *pFilter 
        );
	
	//	@group find methods
	HRESULT FindRender(
        __deref_in IGraphBuilder *pGraph, 
        __in REFGUID guidMediaType, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );
	HRESULT FindAudioRender(
        __deref_in IGraphBuilder *pGraph, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );
	HRESULT FindVideoRender(
        __deref_in IGraphBuilder *pGraph, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );
	HRESULT FindFilterByInterface(
        __deref_in IGraphBuilder *pGraph, 
        __in REFIID riid, 
        __out IBaseFilter **ppFilter
        );

	//	@group delete methods
	HRESULT NukeRender(
        __deref_in IGraphBuilder *pGraph, 
        __out IPin **ppAudioPin, 
        __out IPin **ppVideoPin
        );
	HRESULT NukeDownStream(
        __deref_in IGraphBuilder *pGraph, 
        __deref_in IBaseFilter *pFilter
        );

	//	@grop count methods
	HRESULT CountFilterPins(
        __deref_in IBaseFilter *pFilter, 
        __out ULONG &ulInPins, 
        __out ULONG &ulOutPins
        );

	//	@group get methods
	HRESULT GetPin(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __in UINT uiNum, 
        __out IPin **ppPin
        );
	HRESULT GetInPin(
        __deref_in IBaseFilter *pFilter, 
        __in UINT uiNum, 
        __out IPin **ppPin
        );
	HRESULT GetOutPin(
        __deref_in IBaseFilter *pFilter, 
        __in UINT uiNum, 
        __out IPin **ppPin
        );
	HRESULT GetUnconnectedPin(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __out IPin **ppPin
        );
	HRESULT GetPin(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __in REFGUID guidMediaType, 
        __out IPin **ppPin
        );
	HRESULT GetVideoPin(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __out IPin **ppVideoPin
        );
	HRESULT GetAudioPin(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __out IPin **ppAudioPin
        );
    HRESULT GetAVPinOnSourceFilter(
        __deref_in IGraphBuilder *pGraph,
        __in_z LPCWSTR wszFullPathName,
        __out IPin **ppVideoPin, 
        __out IPin **ppAudioPin
        );

	HRESULT GetNextStream(
        __deref_in IBaseFilter *pFilter, 
        __in PIN_DIRECTION dirrequired, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );
	HRESULT GetNextUpStream(
        __deref_in IBaseFilter *pFilter, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );
	HRESULT GetNextDownStream(
        __deref_in IBaseFilter *pFilter, 
        __in UINT uiNum, 
        __out IBaseFilter **ppFilter
        );

	//	@group debug methods
	HRESULT AppendGraphToRot(
        __deref_in IUnknown *pUnkGraph, 
        __out DWORD &dwRegister
        );
	HRESULT RemoveGraphFromRot(
        __in DWORD dwRegister
        );

	//	@group persistent storage
	HRESULT SaveGraphFile(
        __deref_in IGraphBuilder *pGraph, 
        __in_z LPCWSTR wszPath
        );
	HRESULT LoadGraphFile(
        __deref_in IGraphBuilder *pGraph, 
        __in_z LPCWSTR wszPath
        );

	//	@group enum methods
	HRESULT FindCLSIDByFriendlyName(
        __out CLSID &item, 
        __in REFCLSID category, 
        __in_z LPCWSTR wszFriendlyName
        );
}


