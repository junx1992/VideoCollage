/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the helper fucntion for reading and writing xml files

Notes:
  

History:
  Created on 06/26/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include <objbase.h>
#include <vector>
#include "msxml6.h"


namespace VideoAnalysisHelper
{
      ///define safe deallocate
     #define SAFE_COM_RELEASE(x)	   	    \
	 if(x)								    \
	 {										\
		x->Release();						\
		x = NULL;							\
	 }

     //define for goto clean part if a HRESULT call fail
     #define HR_SUCCESSCALL(hr, res)	    \
	 if( hr != S_OK )					    \
     {                                      \
         res = E_FAIL;                      \
         break;			                    \
     }

	 ///define safe bstr release
     #define SAFE_BSTR_RELEASE(bstr)        \
     if( bstr != NULL )                     \
     {                                      \
        SysFreeString(bstr);                \
		bstr = NULL;                        \
     }
    
     ///put xml file element function
	 HRESULT PutXMLFileHead(IXMLDOMDocument* pDOM, IXMLDOMElement** ppRoot);

     ///helper function to append a whitespace text node to a specified element
	 HRESULT AddWhiteSpaceToNode(IXMLDOMDocument* pDom, BSTR bstrWs, IXMLDOMNode *pNode);

     ///helper function to append a child to a parent node:
	 HRESULT AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent);

     ///add boundary to shot or sub shot fragment node
     HRESULT AddBoundaryNodeToDocumentFragment(
		     IXMLDOMDocument* pDOM, 
		     BSTR indentBstr,
		     IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		     unsigned int bgnFrameId, unsigned int endFrameId, 
		     __int64 bgnFrameTime, __int64 endFrameTime);

     ///helper function to append a sub node child to a documentfragment with given name and value:
 	 HRESULT AddSubNodeToDocumentFragment(
		     IXMLDOMDocument* pDOM, 
		     const wchar_t* wszSubNodeName, 
		     const wchar_t* wszSubNodeValue,
		     IXMLDOMDocumentFragment* pdf	// Release is completed outside
		);

     ///add Id array to a document fragment node
	 HRESULT AddIdsNode(
		     IXMLDOMDocument* pDOM, 
		     BSTR indentBstr,
		     const wchar_t* wszIdsNodeName, 
		     const wchar_t* wszIdsNodeAttribute,
		     const wchar_t* wszIdNode,
		     const std::vector<int>& ids, 
		     IXMLDOMDocumentFragment* pdf	// Release is completed outside
		);
    
}