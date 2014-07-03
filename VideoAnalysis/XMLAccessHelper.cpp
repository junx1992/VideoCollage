#include "Stdafx.h"
#include <vector>
#include <comutil.h>
#include "XMLAccessHelper.h"

namespace VideoAnalysisHelper
{
    //helper function to append a child to a parent node:
	HRESULT AppendChildToParent(IXMLDOMNode *pChild, IXMLDOMNode *pParent)
	{
		 IXMLDOMNode *pNode=NULL;
         if(  pParent->appendChild(pChild, &pNode) != S_OK )
              return E_FAIL;
         else {
			 SAFE_COM_RELEASE(pNode);
   			 return S_OK;
         }
    }

    //put xml file element function
	HRESULT PutXMLFileHead(IXMLDOMDocument* pDOM, IXMLDOMElement** ppRoot)
	{
		  IXMLDOMProcessingInstruction *pi = NULL;
          HRESULT hr = S_OK;
		  do
		  {
			    //create a processing instruction element.
			    _bstr_t header = L"xml";
			    _bstr_t content = L"version='1.0' encoding='utf-8'";
			    HR_SUCCESSCALL( pDOM->createProcessingInstruction(header,content, &pi), hr );
                HR_SUCCESSCALL( AppendChildToParent(pi, pDOM), hr );

			    //create the root element.
			    _bstr_t root = L"VideoStrcuture";
			    HR_SUCCESSCALL( pDOM->createElement(root, ppRoot), hr );
			    HR_SUCCESSCALL( AppendChildToParent(*ppRoot, pDOM), hr );
		
		}while(0);

        //release the com object
		SAFE_COM_RELEASE(pi);

		return hr;
	}    

    //helper function to append a whitespace text node to a specified element
	HRESULT AddWhiteSpaceToNode(IXMLDOMDocument* pDom, BSTR bstrWs, IXMLDOMNode *pNode)
	{
		IXMLDOMText *pws=NULL;
		IXMLDOMNode *pBuf=NULL;

        HRESULT hr = S_OK;
		do
		{
			HR_SUCCESSCALL( pDom->createTextNode(bstrWs,&pws), hr );
			HR_SUCCESSCALL( pNode->appendChild(pws,&pBuf), hr );

		}while(0);

		SAFE_COM_RELEASE(pws);
		SAFE_COM_RELEASE(pBuf);

		return hr;
	}


    //helper function to append a sub node child to a documentfragment with given name and value:
	HRESULT AddSubNodeToDocumentFragment(
		     IXMLDOMDocument* pDOM, 
		     const wchar_t* wszSubNodeName, 
		     const wchar_t* wszSubNodeValue,
		     IXMLDOMDocumentFragment* pdf	// Release is completed outside
		)
	{
		BSTR bstr = NULL;
		IXMLDOMElement* peSub = NULL;

        HRESULT hr = S_OK;
		do
		{
			bstr = SysAllocString(wszSubNodeName);
			HR_SUCCESSCALL( pDOM->createElement(bstr, &peSub), hr );
            SAFE_BSTR_RELEASE(bstr);
			
			bstr=SysAllocString(wszSubNodeValue);
			HR_SUCCESSCALL( peSub->put_text(bstr), hr );
			HR_SUCCESSCALL( AppendChildToParent(peSub, pdf), hr );
		}while(0);

        //release the com object
        SAFE_COM_RELEASE(peSub);
        //release the bstr
        SAFE_BSTR_RELEASE(bstr);

		return hr;
	}

	//add Id array to a document fragment node
	HRESULT AddIdsNode(
		    IXMLDOMDocument* pDOM, 
		    BSTR indentBstr,
		    const wchar_t* wszIdsNodeName, 
		    const wchar_t* wszIdsNodeAttribute,
		    const wchar_t* wszIdNode,
		    const std::vector<int>& ids, 
		    IXMLDOMDocumentFragment* pdf	// Release is completed outside
		)
	{
		VARIANT var;
		BSTR bstr = NULL;
		BSTR bstr_wst = SysAllocString(L"\t");
		IXMLDOMElement* pe = NULL;
		IXMLDOMDocumentFragment* pdfSub = NULL;
		IXMLDOMAttribute *pa = NULL;
		IXMLDOMAttribute *pa1 = NULL;

        HRESULT hr = S_OK;
		do
		{
			//create a Node to hold ids.
			bstr = SysAllocString(wszIdsNodeName);
			HR_SUCCESSCALL( pDOM->createElement(bstr, &pe), hr );
			SAFE_BSTR_RELEASE(bstr);

			//create a attribute for the <wszIdsNodeName> element, and
			//assign the element num as the attribute value.
			
			//get ids num string
			size_t idsNum = ids.size();
			const int radix = 10;
			const size_t sizeOfstr = 30;
			wchar_t wszIdsNumString[sizeOfstr] = {0};
			_ultow_s(static_cast<unsigned long>(idsNum), wszIdsNumString, sizeOfstr, radix);

			//put num string into attribute
			bstr = SysAllocString(wszIdsNodeAttribute);
			VariantInit(&var);
			V_BSTR(&var) = SysAllocString(wszIdsNumString);
			V_VT(&var) = VT_BSTR;

			HR_SUCCESSCALL( pDOM->createAttribute(bstr, &pa), hr );
			HR_SUCCESSCALL( pa->put_value(var), hr );
			HR_SUCCESSCALL( pe->setAttributeNode(pa, &pa1), hr );

			//create a document fragment to hold ids sub-elements.
			HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSub), hr );

			//add ids to pdfSub
			for( size_t i=0; i < idsNum; ++i )
			{
				 int id = ids[i];
				 WCHAR wszIdString[sizeOfstr] = {0};
				 _itow_s(id, wszIdString, sizeOfstr, radix);

				 //add white space before <id>
				 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
				 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );
				 HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, wszIdNode, wszIdString, pdfSub), hr );
			}

            //test whether it is successful in "add ids to pdfSub"
			HR_SUCCESSCALL( hr, hr );

			//add ids array to document fragment node
			HR_SUCCESSCALL( AppendChildToParent(pdfSub, pe), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pe), hr );
			HR_SUCCESSCALL( AppendChildToParent(pe, pdf), hr );
			
		}while(0);

        //release the com objects
		SAFE_COM_RELEASE(pa1);
		SAFE_COM_RELEASE(pa);
		SAFE_COM_RELEASE(pdfSub);
		SAFE_COM_RELEASE(pe);

		//release the bstr and variant
        SAFE_BSTR_RELEASE(bstr);
        SAFE_BSTR_RELEASE(bstr_wst);
        VariantClear(&var);

		return hr;
	}



    //add boundary to shot or sub shot fragment node
    HRESULT AddBoundaryNodeToDocumentFragment(
		   IXMLDOMDocument* pDOM, 
		   BSTR indentBstr,
		   IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		   unsigned int bgnFrameId, unsigned int endFrameId, 
		   __int64 bgnFrameTime, __int64 endFrameTime)
	{
		BSTR bstr = NULL;
		BSTR bstr_wst = SysAllocString(L"\t");
		IXMLDOMElement* pe = NULL;
		IXMLDOMDocumentFragment* pdfSub = NULL;

        HRESULT hr = S_OK;
		do
		{
			//create a Node to hold boundary.
			bstr = SysAllocString(L"Boundary");
			HR_SUCCESSCALL( pDOM->createElement(bstr, &pe), hr );
			SAFE_BSTR_RELEASE(bstr);

			//create a document fragment to hold boundary's sub-elements.
			HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSub), hr );

			//add sub elements to pdfSub

			//add white space before <BgnFrameId>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

			const int radix = 10;
			const size_t sizeOfstr = 30;
			wchar_t wszBgnFrmIdString[sizeOfstr] = {0};
			_itow_s(bgnFrameId, wszBgnFrmIdString, sizeOfstr, radix);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"BgnFrameId", wszBgnFrmIdString, pdfSub), hr );

			//<EndFrameId>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

			wchar_t wszEndFrmIdString[sizeOfstr] = {0};
			_itow_s(endFrameId, wszEndFrmIdString, sizeOfstr, radix);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"EndFrameId", wszEndFrmIdString, pdfSub), hr );

			//<BgnFrameTime>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

            //begintime
			wchar_t wszBgnFrmTimeString[sizeOfstr*2] = {0};
			_i64tow_s(bgnFrameTime, wszBgnFrmTimeString, sizeOfstr*2, 10);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"BgnFrameTime", wszBgnFrmTimeString, pdfSub), hr );

			//<EndFrameId>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

            //endtime
			wchar_t wszEndFrmTimeString[sizeOfstr*2] = {0};
			_i64tow_s(endFrameTime, wszEndFrmTimeString, sizeOfstr*2, 10);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"EndFrameTime", wszEndFrmTimeString, pdfSub), hr );

			//add Boundary to document fragment node
			HR_SUCCESSCALL( AppendChildToParent(pdfSub, pe), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pe), hr );
			HR_SUCCESSCALL( AppendChildToParent(pe, pdf), hr );
						
		}while(0);

		//release com object
		SAFE_COM_RELEASE(pdfSub);
		SAFE_COM_RELEASE(pe);
        //release bstr
        SAFE_BSTR_RELEASE(bstr);
        SAFE_BSTR_RELEASE(bstr_wst);
		
		return hr;
	}
}