#include "Stdafx.h"
#include "VaUtils.h"
#include "VideoSegments.h"
#include "VideoSegmentStorage.h"
#include "XMLAccessHelper.h"
#include "VxUtils.h"
#include <atlbase.h>
#include <cassert>
#include <comutil.h>
#include <vector>
#include <fstream>


namespace VideoAnalysis
{
    using namespace std;
    using namespace VideoAnalysisHelper;
    using namespace VxCore;

#pragma region TextSaveAPI
    //save a shot list into text file
    //the save format is <shotId bgnframeId endframeId bgntime endtime keyframenum keyframeid... >
    HRESULT SaveShotsToText(const CVideoSegmentList & ShotList, const wchar_t * const wszFullPathFileName)
    {
           try{
               //open the save file
               ofstream out(wszFullPathFileName);
               if( !out )
                   return E_FAIL;
               
               int size = ShotList.Size();
               out<<size<<endl;
               if( !out.good() )
                   return E_FAIL;

               for( int i = 0; i < size; ++i )
               {
                     //the list MUST include shots
                     const CShot & shot = dynamic_cast<const CShot&>(ShotList[i]);
                     //save the boundary
                     out<<shot.Id()<<"\t"
                        <<shot.BeginFrameId()<<"\t"<<shot.EndFrameId()<<"\t"
                        <<shot.BeginTime()<<"\t"<<shot.EndTime()<<"\t";

                     //save the keyframe ids
                     int keyframeNum = shot.GetKeyframeNum();
                     out<<keyframeNum<<"\t";
			         for( int j = 0; j < keyframeNum; ++j )
			              out<<shot.GetKeyframe(j)->Id()<<"\t";
                     out<<endl;
               }

               if( !out.good() )
                   return E_FAIL;

               //close the save file
               out.close();
           }catch(bad_cast&){
               return E_FAIL;
           }
        
           return S_OK;
    }

    
    //save a subshot list into text file
    //the save format is <subshotId shotId bgnframeId endframeId bgntime endtime keyframeid>
    HRESULT SaveSubshotsToText(const CVideoSegmentList & SubshotList, const wchar_t * const wszFullPathFileName)
    {
           try{
               //open the save file
               ofstream out(wszFullPathFileName);
               if( !out )
                   return E_FAIL;

               int size = SubshotList.Size();
               out<<size<<endl;
               if( !out.good() )
                   return E_FAIL;

               for( int i = 0; i < size; ++i )
               {
                     //the list MUST include subshots
                     const CSubshot & subshot = dynamic_cast<const CSubshot&>(SubshotList[i]);
                     out<<subshot.Id()<<"\t"<<subshot.ShotId()<<"\t"
                          <<subshot.BeginFrameId()<<"\t"<<subshot.EndFrameId()<<"\t"
                          <<subshot.BeginTime()<<"\t"<<subshot.EndTime()<<"\t";

                      int keyframeNum = subshot.GetKeyframeNum();
                      out<<keyframeNum<<"\t";

			          for( int j = 0; j < keyframeNum; ++j )
			                out<<subshot.GetKeyframe(j)->Id()<<"\t";

                      out<<endl;
               }
               if( !out.good() )
                   return E_FAIL;
               //close the save file
               out.close();
           }catch(bad_cast&){
               return E_FAIL;
           }
        
           return S_OK;        
    }

    
    //save a scene list into text file
    //the save format is <sceneid keyframenum keyframeid....>
    HRESULT SaveScenesToText(const CVideoSegmentList & SceneList, const wchar_t * const wszFullPathFileName)
    {
            try{
               //open the save file
               ofstream out(wszFullPathFileName);
               if( !out )
                   return E_FAIL;
               
               //the number of scene
               int size = SceneList.Size();
               out<<size<<endl;
               if( !out.good() )
                   return E_FAIL;

               for( int i = 0; i < size; ++i )
               {
                     //the list MUST include scene
                     const CScene & scene = dynamic_cast<const CScene&>(SceneList[i]);
                     
                     //save the scene id
                     out<<scene.Id()<<"\t";

                     //save the keyframe ids
                     int keyframeNum = scene.GetKeyframeNum();
                     out<<keyframeNum<<"\t";
			         for( int j = 0; j < keyframeNum; ++j )
			               out<<scene.GetKeyframe(j)->Id()<<"\t";
                     out<<endl;
               }

               if( !out.good() )
                   return E_FAIL;

               //close the save file
               out.close();
           }catch(bad_cast&){
               return E_FAIL;
           }
        
           return S_OK;
    }
#pragma endregion

    typedef HRESULT (*pSaveToXML)(IXMLDOMDocument* pDOM, IXMLDOMElement* pRoot, const CVideoSegmentList & SceneList);
    //save structure video feature helper function
    HRESULT SaveStructureFeatureToXMLHelper(const CVideoSegmentList & FeatureList, const wchar_t * const wszFullPathFileName, pSaveToXML SaveFunc)
    {
           assert(SaveFunc != NULL );
           //handler to the xml Dom and root element
           IXMLDOMDocument* pDOM = NULL;     
           IXMLDOMElement* pRoot = NULL;

          HRESULT hr = S_OK;

          do{
               
                // Firstly we need to create a DOM object that contains no data
			    HR_SUCCESSCALL( S_OK != CoCreateInstance(CLSID_DOMDocument30,  
                                                         NULL, 
                                                         CLSCTX_INPROC_SERVER, 
                                                         __uuidof(IXMLDOMDocument),
                                                         (void**)&pDOM), hr);
                                  
                //add an xml head into the xml file
       			HR_SUCCESSCALL( PutXMLFileHead(pDOM, &pRoot), hr );

                //save the subshot list into xml file
                HR_SUCCESSCALL( SaveFunc(pDOM, pRoot, FeatureList), hr );

                //save the xml file
                _variant_t name = wszFullPathFileName;
                HR_SUCCESSCALL( pDOM->save(name), hr );

          }while(false);

          //release the com object			
          SAFE_COM_RELEASE(pRoot);
          SAFE_COM_RELEASE(pDOM);

          return hr;    
    }

    
    //add one keyframe info to KeyFrames fragment node
	HRESULT AddKeyFrameToDocFrag(
		     IXMLDOMDocument* pDOM, 
		     BSTR indentBstr,
		     IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		     const CFrame & Keyframe)
	{
		BSTR bstr = NULL;
		BSTR bstr_wst = SysAllocString(L"\t");
		IXMLDOMElement* pe = NULL;
		IXMLDOMDocumentFragment* pdfSub = NULL;


        HRESULT hr = S_OK;

		do
		{
			//create a Node to hold frame.
			bstr = SysAllocString(L"KeyFrame");
			HR_SUCCESSCALL( pDOM->createElement(bstr, &pe), hr );
			SAFE_BSTR_RELEASE(bstr);
			

			//create a document fragment to hold boundary's sub-elements.
			HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSub), hr );

			// Add sub elements to pdfSub

			// Add white space before <FrameId>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

			const int radix = 10;
			const size_t sizeOfstr = 30;
			wchar_t wszFrmIdString[sizeOfstr] = {0};
			_itow_s(Keyframe.Id(), wszFrmIdString, sizeOfstr, radix);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FrameId", wszFrmIdString, pdfSub), hr );

		
			// <BeginTime>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

   			wchar_t wszBgnFrmTimeString[_CVTBUFSIZE] = {0};
			_i64tow_s(Keyframe.BeginTime(), wszBgnFrmTimeString, _CVTBUFSIZE, 10);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"BeginTime", wszBgnFrmTimeString, pdfSub), hr );

			// <EndTime>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pdfSub), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wst, pdfSub), hr );

   			wchar_t wszEndFrmTimeString[_CVTBUFSIZE] = {0};
			_i64tow_s(Keyframe.BeginTime(), wszEndFrmTimeString, _CVTBUFSIZE, 10);
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"EndTime", wszEndFrmTimeString, pdfSub), hr );

			// Add Frame to document fragment node
			HR_SUCCESSCALL( AppendChildToParent(pdfSub, pe), hr );
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, indentBstr, pe), hr );
			HR_SUCCESSCALL( AppendChildToParent(pe, pdf), hr );
			
		}while(0);

		
        SAFE_COM_RELEASE(pdfSub);
		SAFE_RELEASE(pe);
		SAFE_BSTR_RELEASE(bstr);
		SAFE_BSTR_RELEASE(bstr_wst);
		
		return hr;
	}


    
    //add one subshot info to Subshots fragment node
    HRESULT AddSubshotToDocFrag(
    		 IXMLDOMDocument* pDOM,
		     IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		     const CSubshot & subshot)
    {
		     CComPtr<IXMLDOMDocumentFragment>pdfSubshot;
             CComPtr<IXMLDOMDocumentFragment>pdfKeyframe;

             CComPtr<IXMLDOMElement>pEleSub;
             CComPtr<IXMLDOMElement>pEleKeyframe;
   		     CComPtr<IXMLDOMAttribute>pa1;
		     CComPtr<IXMLDOMAttribute>pa2;

             _bstr_t bstr_wsnt = L"\n\t";
		     _bstr_t bstr_wsntt = L"\n\t\t";
		     _bstr_t bstr_wsnttt = L"\n\t\t\t";
  		     _bstr_t bstr_wsntttt = L"\n\t\t\t\t";

             HRESULT hr = S_OK;
             do
			 {
                     //create subshot element
  				     _bstr_t nodeName = L"SubShot";
				     HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleSub), hr );

                     //create a document fragment to hold <SubShot> sub-elements.
				     HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSubshot), hr );
				     //add NEWLINE+TAB+TAB for identation before <SubShotId>.
				     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSubshot), hr );
    				  
                     //add the subshot id
                     const size_t sizeOfstr = 30;
                     wchar_t wszSubShotIdString[sizeOfstr] = {0};
				     _itow_s(subshot.Id(), wszSubShotIdString, sizeOfstr, 10);
				     HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"SubShotId", wszSubShotIdString, pdfSubshot), hr );
				     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSubshot), hr );
    				
                     //add the shot id;
                     wchar_t wszShotIdString[sizeOfstr]={0};
                     _itow_s(subshot.ShotId(), wszShotIdString, sizeOfstr, 10);
				     HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"ShotId", wszShotIdString, pdfSubshot), hr );
				     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSubshot), hr );

                     //add the subshot boundary
				     HR_SUCCESSCALL( AddBoundaryNodeToDocumentFragment( pDOM, bstr_wsnttt, pdfSubshot, 
					                                                    subshot.BeginFrameId(),subshot.EndFrameId(), 
					                                                    subshot.BeginTime(), subshot.EndTime()), hr );
			         HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSubshot), hr );
    				  
    				
                     //key frames
                     //add NEWLINE+TAB+TAB for identation before <KeyFrames>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSubshot), hr );

                     //create a document fragment to hold <KeyFrames> sub-elements.
	                 HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfKeyframe), hr );
            	    
                     //create KeyFrames node
                     nodeName = L"KeyFrames";
			         HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleKeyframe), hr );

			         //put num string into attribute
                     _bstr_t attriName = L"KeyFramesNum";
                     _variant_t attriValue = subshot.GetKeyframeNum();

			         HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			         HR_SUCCESSCALL( pa1->put_value(attriValue), hr );
			         HR_SUCCESSCALL( pEleKeyframe->setAttributeNode(pa1, &pa2), hr );

  	                 //add NEWLINE+TAB+TAB+TAB for identation before first <KeyFrame>.
                     int kfNum = subshot.GetKeyframeNum();
                     for( int kf_index = 0; kf_index < kfNum; ++kf_index )
                     {
                           HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntttt, pdfKeyframe), hr );
                           HR_SUCCESSCALL( AddKeyFrameToDocFrag(pDOM, bstr_wsntttt, pdfKeyframe, *subshot.GetKeyframe(kf_index)), hr ); 
                     }
                     //test wether add key frame is OK
                     HR_SUCCESSCALL( hr, hr );
                       	                 
                     //add NEWLINE+TAB+TAB+TAB for identation before </KeyFrames>.
                     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfKeyframe), hr );

                     //add all children to <KeyFrames>
			         HR_SUCCESSCALL( AppendChildToParent(pdfKeyframe, pEleKeyframe), hr );
                     HR_SUCCESSCALL( AppendChildToParent(pEleKeyframe, pdfSubshot), hr );

                      //add subShot to <Subshots>
				      HR_SUCCESSCALL( AppendChildToParent(pdfSubshot, pEleSub), hr );
				      HR_SUCCESSCALL( AppendChildToParent(pEleSub, pdf), hr );
    				  
			 }while(false);	

             return hr;
    }


    //add one shot info to Shots fragment node
    HRESULT AddShotToDocFrag(
    		 IXMLDOMDocument* pDOM,
		     IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		     const CShot & shot)
    {
		     CComPtr<IXMLDOMDocumentFragment>pdfSub;
             CComPtr<IXMLDOMDocumentFragment>pdfKeyframe;
             CComPtr<IXMLDOMDocumentFragment>pdfSubshot;
             
             CComPtr<IXMLDOMElement>pEleShot;
             CComPtr<IXMLDOMElement>pEleSubshot;
             CComPtr<IXMLDOMElement>pEleKeyframe;
   		     CComPtr<IXMLDOMAttribute>pa1;
		     CComPtr<IXMLDOMAttribute>pa2;
             CComPtr<IXMLDOMAttribute>pa3;
		     CComPtr<IXMLDOMAttribute>pa4;

		     _bstr_t bstr_wsntt = L"\n\t\t";
		     _bstr_t bstr_wsnttt = L"\n\t\t\t";
		     _bstr_t bstr_wsntttt = L"\n\t\t\t\t";

             HRESULT hr = S_OK;
             do
             {
                     //add the "Shot" node
                     _bstr_t nodeName = L"Shot";
	                 HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleShot), hr );
        				
	                 //create a document fragment to hold <Shot> sub-elements.
	                 HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSub), hr );
            	    
                     //add NEWLINE+TAB+TAB for identation before <ShotId>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSub), hr );
            	    
                     //add the "ShotId"
                     const size_t sizeOfstr = 30;
                     wchar_t wszShotIdString[sizeOfstr] = {0};
	                 _itow_s(shot.Id(), wszShotIdString, sizeOfstr, 10);
	                 HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"ShotId", wszShotIdString, pdfSub), hr );
            	   
                     //add NEWLINE+TAB+TAB for identation before <Boundary>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSub), hr );
            	
                     //and the boundary  node
	                 HR_SUCCESSCALL( AddBoundaryNodeToDocumentFragment( pDOM, bstr_wsnttt, pdfSub, 
		                                                                shot.BeginFrameId(),shot.EndFrameId(), 
		                                                                shot.BeginTime(), shot.EndTime()), hr );
                     

	                 //Subshots
                     //add NEWLINE+TAB+TAB for identation before <Subshots>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSub), hr );
            	    
                     //create a document fragment to hold <Subshots> sub-elements.
	                 HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfSubshot), hr );
            	    
                     //create Subshots node
                     nodeName = L"Subshots";
			         HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleSubshot), hr );

			         //put num string into attribute
                     _bstr_t attriName = L"SubshotsNum";
                     _variant_t attriValue = shot.GetChildrenNum();

			        HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			        HR_SUCCESSCALL( pa1->put_value(attriValue), hr );
			        HR_SUCCESSCALL( pEleSubshot->setAttributeNode(pa1, &pa2), hr );

	                 int subShotNum = shot.GetChildrenNum();
                     for( int i = 0; i < subShotNum; ++i )
                     {
                            const CSubshot & subshot = dynamic_cast<const CSubshot&>(*shot.GetChild(i));
                            HR_SUCCESSCALL( AddSubshotToDocFrag(pDOM, pdfSubshot, subshot), hr ); 
                     }

                     //test wether add sub shot is OK
                     HR_SUCCESSCALL( hr, hr );
                                          
                     HR_SUCCESSCALL( AppendChildToParent(pdfSubshot, pEleSubshot), hr );
                     HR_SUCCESSCALL( AppendChildToParent(pEleSubshot, pdfSub), hr );

                     //key frames
                     //add NEWLINE+TAB+TAB for identation before <KeyFrames>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfSub), hr );

                     //create a document fragment to hold <KeyFrames> sub-elements.
	                 HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfKeyframe), hr );
            	    
                     //create KeyFrames node
                     nodeName = L"KeyFrames";
			         HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleKeyframe), hr );

			         //put num string into attribute
                     attriName = L"KeyFramesNum";
                     attriValue = shot.GetKeyframeNum();

			         HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa3), hr );
			         HR_SUCCESSCALL( pa3->put_value(attriValue), hr );
			         HR_SUCCESSCALL( pEleKeyframe->setAttributeNode(pa3, &pa4), hr );

  	                 //add NEWLINE+TAB+TAB+TAB for identation before first <KeyFrame>.
                     int kfNum = shot.GetKeyframeNum();
                     for( int kf_index = 0; kf_index < kfNum; ++kf_index )
                     {
                           HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntttt, pdfKeyframe), hr );
                           HR_SUCCESSCALL( AddKeyFrameToDocFrag(pDOM, bstr_wsntttt, pdfKeyframe, *shot.GetKeyframe(kf_index)), hr ); 
                     }
                     //test wether add key frame is OK
                     HR_SUCCESSCALL( hr, hr );
                       	                 
                     //add NEWLINE+TAB+TAB+TAB for identation before </KeyFrames>.
                     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfKeyframe), hr );

                     //add all children to <KeyFrames>
			         HR_SUCCESSCALL( AppendChildToParent(pdfKeyframe, pEleKeyframe), hr );
                     HR_SUCCESSCALL( AppendChildToParent(pEleKeyframe, pdfSub), hr );


  	                 //add NEWLINE+TAB+TAB+TAB for identation before </Shot>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdfSub), hr );

	                 //add shot to <Shots>
                     HR_SUCCESSCALL( AppendChildToParent(pdfSub, pEleShot), hr );
	                 HR_SUCCESSCALL( AppendChildToParent(pEleShot, pdf), hr );

             }while(false);

             return hr;
    }




    //add one scene info to Scene fragment node
    HRESULT AddSceneToDocFrag(
    		 IXMLDOMDocument* pDOM,
		     IXMLDOMDocumentFragment* pdf,   // Release is completed outside
		     const CScene & scene)
     {
                  
             CComPtr<IXMLDOMDocumentFragment> pdfScene;
             CComPtr<IXMLDOMDocumentFragment>pdfShot;
             CComPtr<IXMLDOMDocumentFragment>pdfKeyframe;

             CComPtr<IXMLDOMElement>pEleShot;
             CComPtr<IXMLDOMElement>pEleScene;
             CComPtr<IXMLDOMElement>pEleKeyframe;
   		     CComPtr<IXMLDOMAttribute>pa1;
		     CComPtr<IXMLDOMAttribute>pa2;
             CComPtr<IXMLDOMAttribute>pa3;
		     CComPtr<IXMLDOMAttribute>pa4;

		     _bstr_t bstr_wsntt = L"\n\t\t";
		     _bstr_t bstr_wsnttt = L"\n\t\t\t";
		     _bstr_t bstr_wsntttt = L"\n\t\t\t\t";

             HRESULT hr = S_OK;
             do
			 {
                      _bstr_t nodeName = SysAllocString(L"Scene");
				      HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleScene), hr );

				      //create a document fragment to hold <Scene> sub-elements.
				      HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfScene), hr );
				      //add NEWLINE+TAB+TAB for identation before <SceneId>.
				      HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfScene), hr );
				
                       const size_t sizeOfstr = 30;
                       wchar_t wszSceneIdString[sizeOfstr] = {0};
                       _itow_s(scene.Id(), wszSceneIdString, sizeOfstr, 10);
				       HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"SceneId", wszSceneIdString, pdfScene), hr );
				        
                       //Shots
                       //add NEWLINE+TAB+TAB for identation before <Shots>.
	                   HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfScene), hr );
            	    
                       //create a document fragment to hold <Shots> sub-elements.
	                   HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfShot), hr );
            	    
                       //create Subshots node
                       nodeName = L"Shots";
			           HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleShot), hr );

			           //put num string into attribute
                       _bstr_t attriName = L"ShotsNum";
                       _variant_t attriValue = scene.GetChildrenNum();

			          HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			          HR_SUCCESSCALL( pa1->put_value(attriValue), hr );
			          HR_SUCCESSCALL( pEleShot->setAttributeNode(pa1, &pa2), hr );

                     //add NEWLINE+TAB+TAB for identation before <Shot>.
	                 HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfShot), hr );

	                  int shotNum = scene.GetChildrenNum();
                      for( int i = 0; i < shotNum; ++i )
                      {
                            const CShot & shot = dynamic_cast<const CShot&>(*scene.GetChild(i));
                            HR_SUCCESSCALL( AddShotToDocFrag(pDOM, pdfShot, shot), hr ); 
                      }

                      //test wether add shot is OK
                      HR_SUCCESSCALL( hr, hr );
                                          
                      HR_SUCCESSCALL( AppendChildToParent(pdfShot, pEleShot), hr );
                      HR_SUCCESSCALL( AppendChildToParent(pEleShot, pdfScene), hr );

                      //key frames
                      //add NEWLINE+TAB+TAB for identation before <KeyFrames>.
	                  HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfScene), hr );

                      //create a document fragment to hold <KeyFrames> sub-elements.
	                  HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdfKeyframe), hr );
            	    
                      //create KeyFrames node
                      nodeName = L"KeyFrames";
			          HR_SUCCESSCALL( pDOM->createElement(nodeName, &pEleKeyframe), hr );

			          //put num string into attribute
                      attriName = L"KeyFramesNum";
                      attriValue = scene.GetKeyframeNum();

			          HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa3), hr );
			          HR_SUCCESSCALL( pa3->put_value(attriValue), hr );
			          HR_SUCCESSCALL( pEleKeyframe->setAttributeNode(pa3, &pa4), hr );

  	                  //add NEWLINE+TAB+TAB+TAB for identation before first <KeyFrame>.
                      int kfNum = scene.GetKeyframeNum();
                      for( int kf_index = 0; kf_index < kfNum; ++kf_index )
                      {
                            HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntttt, pdfKeyframe), hr );
                            HR_SUCCESSCALL( AddKeyFrameToDocFrag(pDOM, bstr_wsntttt, pdfKeyframe, *scene.GetKeyframe(kf_index)), hr ); 
                      }
                     
                      //test wether add key frame is OK
                      HR_SUCCESSCALL( hr, hr );
                       	                 
                      //add NEWLINE+TAB+TAB+TAB for identation before </KeyFrames>.
                      HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnttt, pdfKeyframe), hr );

                      //add all children to <KeyFrames>
			          HR_SUCCESSCALL( AppendChildToParent(pdfKeyframe, pEleKeyframe), hr );
                      HR_SUCCESSCALL( AppendChildToParent(pEleKeyframe, pdfScene), hr );

                      //add scene to <Scenes>
				      HR_SUCCESSCALL( AppendChildToParent(pdfScene, pEleScene), hr );
				      HR_SUCCESSCALL( AppendChildToParent(pEleScene, pdf), hr );
				    
			}while(false);

            return hr;   
     }
    //save the scene list into a dom
    HRESULT PutSceneList(IXMLDOMDocument* pDOM, IXMLDOMElement* pRoot, const CVideoSegmentList & SceneList)
	{
		
		   CComPtr<IXMLDOMElement>pe;
		   CComPtr<IXMLDOMDocumentFragment>pdf;
		   CComPtr<IXMLDOMAttribute>pa1;
		   CComPtr<IXMLDOMAttribute>pa2;

		   _bstr_t bstr_wsnt = L"\n\t";
		   _bstr_t bstr_wsntt = L"\n\t\t";

          HRESULT hr = S_OK;
		  do
		  {
			   //add NEWLINE+TAB for identation before <Scenes>.
			   HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pRoot), hr );

			   //create scenes node
			   _bstr_t nodeName = L"Scenes";
			   HR_SUCCESSCALL( pDOM->createElement(nodeName, &pe), hr );
			   
                //create a attribute for the <Scenes>, and assign the scenes num as the attribute value.
			    //put num string into attribute
			    _bstr_t attriName = L"ScenesNum";
                _variant_t attriVal = SceneList.Size();

			    HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			    HR_SUCCESSCALL( pa1->put_value(attriVal), hr );
			    HR_SUCCESSCALL( pe->setAttributeNode(pa1, &pa2), hr );

			    //create a document fragment to hold <Scenes> sub-elements <Scene>.
			    HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdf), hr );

		        //add each scene
                int scenesNum = SceneList.Size();
			    for( int i = 0; i < scenesNum; ++i )
			    {
                      const CScene & scene = dynamic_cast<const CScene&>(SceneList[i]);
				      
                      //add scene one by one into the document fragment
                      HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
                      HR_SUCCESSCALL( AddSceneToDocFrag(pDOM, pdf, scene), hr);
			    }	

               //test wether it is successful in put every scene
			   HR_SUCCESSCALL( hr, hr );
			
			   //add all children to <Scenes>
			   HR_SUCCESSCALL( AppendChildToParent(pdf, pe), hr );
			   //add NEWLINE+TAB+TAB for identation before </Scenes>.
			   HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pe), hr );
			   //add <Scenes> to <VideoStrcuture>
			  HR_SUCCESSCALL( AppendChildToParent(pe, pRoot), hr );

		}while(false);
        
		return hr;
	}

    //save the shot list into a dom
    HRESULT PutShotList(IXMLDOMDocument* pDOM, IXMLDOMElement* pRoot, const CVideoSegmentList & ShotList)
	{
		   CComPtr<IXMLDOMElement> pe;
		   CComPtr<IXMLDOMDocumentFragment>pdf;
		   CComPtr<IXMLDOMAttribute> pa1;
		   CComPtr<IXMLDOMAttribute> pa2;

		   _bstr_t bstr_wsnt = L"\n\t";
		   _bstr_t bstr_wsntt = L"\n\t\t";

           HRESULT hr = S_OK;
		   do
		   { 
			    //add NEWLINE+TAB for identation before <Shots>.
			    HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pRoot), hr );

			    //create shots node
			    _bstr_t nodeName = L"Shots";
			    HR_SUCCESSCALL( pDOM->createElement(nodeName, &pe), hr );

                //put num string into attribute
			    _bstr_t attriName = L"ShotsNum";
                _variant_t attriVal = ShotList.Size();

			    HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			    HR_SUCCESSCALL( pa1->put_value(attriVal), hr );
			    HR_SUCCESSCALL( pe->setAttributeNode(pa1, &pa2), hr );
			   
                //create a document fragment to hold <Shots> sub-elements <Shot>.
			    HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdf), hr );

			    //add each shot
                int shotsNum = ShotList.Size();
			    for( int i=0; i < shotsNum; ++i )
			    {
                     const CShot & shot = dynamic_cast<const CShot&>(ShotList[i]);
                     //add shot one by one into the document fragment
                     HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
                     HR_SUCCESSCALL( AddShotToDocFrag(pDOM, pdf, shot), hr);
                }	

                //test whether it is successful in "add each shot" 
			    HR_SUCCESSCALL( hr, hr );
			
			    //add all children to <Shots>
                HR_SUCCESSCALL( AppendChildToParent(pdf, pe), hr );
			    //add <Shots> to <VideoStrcuture>
			    HR_SUCCESSCALL( AppendChildToParent(pe, pRoot), hr );

		}while(false);
		        
		return hr;
	}

    //save the sub shot list into a dom
    HRESULT PutSubshotList(IXMLDOMDocument* pDOM, IXMLDOMElement* pRoot, const CVideoSegmentList & SubshotList)
	{
        CComPtr<IXMLDOMElement> pe;
		CComPtr<IXMLDOMDocumentFragment> pdf;
		CComPtr<IXMLDOMAttribute>pa1;
		CComPtr<IXMLDOMAttribute>pa2;

		_bstr_t bstr_wsnt = L"\n\t";
		_bstr_t bstr_wsntt = L"\n\t\t";
         
        HRESULT hr = S_OK;
		do
		{
			//add NEWLINE+TAB for identation before <Subshots>.
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pRoot), hr );

			//create Subshots Node
			_bstr_t nodeName = L"Subshots";
			HR_SUCCESSCALL( pDOM->createElement(nodeName, &pe), hr );
			
			//create a attribute for the <Subshots>, and  assign the sub shots num as the attribute value.
			//put num string into attribute
			_bstr_t attriName = L"SubshotsNum";
            _variant_t attriVal = SubshotList.Size();

			HR_SUCCESSCALL( pDOM->createAttribute(attriName, &pa1), hr );
			HR_SUCCESSCALL( pa1->put_value(attriVal), hr );
			HR_SUCCESSCALL( pe->setAttributeNode(pa1, &pa2), hr );

			//create a document fragment to hold <Shots> sub-elements <Shot>.
			HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdf), hr );

			//add each sub shot one by one 
            int subShotsNum = SubshotList.Size();
			for( int i = 0; i < subShotsNum; ++i )
			{
                 const CSubshot & subshot = dynamic_cast<const CSubshot&>(SubshotList[i]);
				   
                //add shot one by one into the document fragment
                HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
                HR_SUCCESSCALL( AddSubshotToDocFrag(pDOM, pdf, subshot), hr);
			}	

            //test whether it is successful in "add each sub shot"
			HR_SUCCESSCALL( hr, hr );
			
			//add all children to <Subshots>
			HR_SUCCESSCALL( AppendChildToParent(pdf, pe), hr );
			//add NEWLINE+TAB+TAB for identation before </Subshots>.
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pe), hr );
			//add <Subshots> to <VideoStrcuture>
			HR_SUCCESSCALL( AppendChildToParent(pe, pRoot), hr );

		}while(false);

     	return hr;
	}


    //save the video information into a dom
    HRESULT PutVideoInfo(IXMLDOMDocument* pDOM, IXMLDOMElement* pRoot, const CVideoInfo & videoInfo, const FileInfo & fileInfo)
	{
		
		IXMLDOMElement *pe = NULL;
		IXMLDOMDocumentFragment *pdf=NULL;
       
        BSTR bstr = NULL;
		BSTR bstr_wsnt = SysAllocString(L"\n\t");
		BSTR bstr_wsntt = SysAllocString(L"\n\t\t");

        HRESULT hr = S_OK;
		do
		{
		 	// Add NEWLINE+TAB for identation before <VideoFileInfo>.
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsnt, pRoot), hr );
	
			 //create a document fragment to hold <VideoFileInfo> sub-elements.
			HR_SUCCESSCALL( pDOM->createDocumentFragment(&pdf), hr );

			//add NEWLINE+TAB+TAB for identation before <FileName>.
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
            HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FileName", fileInfo.m_FileName, pdf), hr );

			// Get duration string
			double duration = videoInfo.GetDuration();
			char cDurationString[_CVTBUFSIZE] = {0};
			_gcvt_s(cDurationString, _CVTBUFSIZE, duration, 6);
			// Convert char to wchar
			wchar_t wszDurationString[_CVTBUFSIZE] = {0};
			MultiByteToWideChar(CP_ACP, 0, cDurationString, -1, wszDurationString, _CVTBUFSIZE);

			// <Duration>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"Duration", wszDurationString, pdf), hr );
			
			// Get file bit size string
			unsigned long fileByteSize = videoInfo.GetFileSize();
			const int radix = 10;
			const size_t sizeOfstr = 30;
			wchar_t wszFileByteSizeString[sizeOfstr] = {0};
			_ultow_s(fileByteSize, wszFileByteSizeString, sizeOfstr, radix);
			
			// <FileByteSize>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FileByteSize", wszFileByteSizeString, pdf), hr );

			// Get frames num string
			int framesNum = videoInfo.GetFrameNum();
			wchar_t wszFramesNumString[sizeOfstr] = {0};
			_itow_s(framesNum, wszFramesNumString, sizeOfstr, radix);

			// <FramesNum>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FramesNum", wszFramesNumString, pdf), hr );

			// Get frame width string
			int frameWidth = videoInfo.GetFrameWidth();
			wchar_t wszFrameWidthString[sizeOfstr] = {0};
			_itow_s(static_cast<ULONG>(frameWidth), wszFrameWidthString, sizeOfstr, radix);

			// <FrameWidth>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FrameWidth", wszFrameWidthString, pdf), hr );

			// Get frame height string
			int frameHeight = videoInfo.GetFrameHeight();
			wchar_t wszFrameHeightString[sizeOfstr] = {0};
			_itow_s(static_cast<ULONG>(frameHeight), wszFrameHeightString, sizeOfstr, radix);

			// <FrameHeight>
			HR_SUCCESSCALL( AddWhiteSpaceToNode(pDOM, bstr_wsntt, pdf), hr );
			HR_SUCCESSCALL( AddSubNodeToDocumentFragment(pDOM, L"FrameHeight", wszFrameHeightString, pdf), hr );

			// Add the document fragment to <VideoFileInfo> node
			HR_SUCCESSCALL( AppendChildToParent(pdf, pRoot), hr );
		
		}while(false);

       
        SAFE_COM_RELEASE(pdf);
		SAFE_COM_RELEASE(pe);		

		SAFE_BSTR_RELEASE(bstr)
		SAFE_BSTR_RELEASE(bstr_wsnt)
		SAFE_BSTR_RELEASE(bstr_wsntt)

		return hr;
	}


#pragma region XMLSaveAPI
    //save a shot list into xml file
    HRESULT SaveShotsToXML(const CVideoSegmentList & ShotList, const wchar_t * const wszFullPathFileName)
    {
          return SaveStructureFeatureToXMLHelper(ShotList, wszFullPathFileName, PutShotList);
    }
    
    //save a subshot list into xml file
    HRESULT SaveSubshotsToXML(const CVideoSegmentList & SubshotList, const wchar_t * const wszFullPathFileName)
    {
          return SaveStructureFeatureToXMLHelper(SubshotList, wszFullPathFileName, PutSubshotList);
    }

    //save a scene into xml file
    HRESULT SaveScenesToXML(const CVideoSegmentList & SceneList, const wchar_t * const wszFullPathFileName)
    {
          return SaveStructureFeatureToXMLHelper(SceneList, wszFullPathFileName, PutSceneList);
    }

    //save video info to xml file
    HRESULT SaveVideoInfoToXML(const CVideoInfo & videoInfo, const FileInfo & fileInfo, const wchar_t * const wszFullPathFileName)
    {
           //handler to the xml Dom and root element
           IXMLDOMDocument* pDOM = NULL;     
           IXMLDOMElement* pRoot = NULL;
		   IXMLDOMProcessingInstruction *pi = NULL;

           HRESULT hr = S_OK;
           do{
               
                // Firstly we need to create a DOM object that contains no data
			    HR_SUCCESSCALL( S_OK != CoCreateInstance(CLSID_DOMDocument30,  
                                                         NULL, 
                                                         CLSCTX_INPROC_SERVER, 
                                                         __uuidof(IXMLDOMDocument),
                                                         (void**)&pDOM), hr);
                                  
                //add an xml head into the xml file
			    //create a processing instruction element.
			    _bstr_t header = L"xml";
			    _bstr_t content = L"version='1.0' encoding='utf-8'";
			    HR_SUCCESSCALL( pDOM->createProcessingInstruction(header,content, &pi), hr );
                HR_SUCCESSCALL( AppendChildToParent(pi, pDOM), hr );

                //create Video Node
			    _bstr_t bstr = L"VideoFileInfo";
			    HR_SUCCESSCALL( pDOM->createElement(bstr, &pRoot), hr );
  			    HR_SUCCESSCALL( AppendChildToParent(pRoot, pDOM), hr );

                //save the subshot list into xml file
                HR_SUCCESSCALL( PutVideoInfo(pDOM, pRoot, videoInfo, fileInfo), hr );

                //save the xml file
                _variant_t name = wszFullPathFileName;
                HR_SUCCESSCALL( pDOM->save(name), hr );

          }while(false);

          //release the com object			
          SAFE_COM_RELEASE(pRoot);
          SAFE_COM_RELEASE(pDOM);

          return hr;    
    }

#pragma endregion

#pragma region TextLoadAPI
    //load a shot list from text file
    //the save format is <shotId bgnframeId endframeId bgntime endtime subshotnum subshotid... keyframenum keyframeid... >
    HRESULT LoadShotsFromText(CVideoSegmentList & ShotList, const wchar_t * const wszFullPathFileName)
    {
           //open the save file
           ifstream in(wszFullPathFileName);
           if( !in )    return E_FAIL;
               
           int size = 0;
           in>>size;
           if( !in.good() )
                return E_FAIL;

           for( int i = 0; i < size; ++i )
           {
                 CShot shot;
                        
                 //load the boundary
                 int id, bgnid, endid;
                 __int64 bgntime, endtime;
                 in>>id>>bgnid>>endid>>bgntime>>endtime;

                 shot.Id(id);
                 shot.BeginFrameId(bgnid);
                 shot.EndFrameId(endid);
                 shot.BeginTime(bgntime);
                 shot.EndTime(endtime);
                        
                 //load the keyframe ids
                 int keyframeNum = shot.GetKeyframeNum();
                 in>>keyframeNum;
	             for( int j = 0; j < keyframeNum; ++j )
                 {
                       CFrame keyframe;
	                    in>>id;
                        keyframe.Id(id);

                        //!!!the key frame only id is validate!!!
                        shot.AddKeyframe(keyframe);
                 }

                 ShotList.Add(shot);                      
         }
         if( !in.good() )
             return E_FAIL;
      
         //close the loaded file
         in.close();
         return S_OK;
    }

    //load a subshot list from text file
    //the save format is <subshotId shotId bgnframeId endframeId bgntime endtime keyframeid>
    HRESULT LoadSubshotsFromText(CVideoSegmentList & SubshotList, const wchar_t * const wszFullPathFileName)
    {
            //open the save file
            ifstream in(wszFullPathFileName);
            if( !in )    return E_FAIL;

            int size = 0;
            in>>size;
            if( !in.good() )
                return E_FAIL;

            for( int i = 0; i < size; ++i )
            {
                  CSubshot subshot;

                  //load the boundary
                  int id, shotid, bgnid, endid;
                  __int64 bgntime, endtime;
                  in>>id>>shotid>>bgnid>>endid>>bgntime>>endtime;

                  subshot.Id(id);
                  subshot.ShotId(shotid);
                  subshot.BeginFrameId(bgnid);
                  subshot.EndFrameId(endid);
                  subshot.BeginTime(bgntime);
                  subshot.EndTime(endtime);

                  int keyframeNum = 0;
                  in>>keyframeNum;
	              for( int j = 0; j < keyframeNum; ++j )
                  {
                        CFrame keyframe;
	                    in>>id;
                        keyframe.Id(id);

                        //!!!the key frame only id is validate!!!
                        subshot.AddKeyframe(keyframe);
                  }

                  SubshotList.Add(subshot);
          }

          if( !in.good() )
              return E_FAIL;
 
           //close the load file
           in.close();
           return S_OK;        
    }

    ///load a scene list from text file
    HRESULT LoadScenesFromText(CVideoSegmentList & SceneList, const wchar_t * const wszFullPathFileName)
    {
            //open the save file
            ifstream in(wszFullPathFileName);
            if( !in )    return E_FAIL;

            int size = 0;
            in>>size;
            if( !in.good() )
                return E_FAIL;

            for( int i = 0; i < size; ++i )
            {
                   CScene scene;

                   int id, bgnid, endid;
                   __int64 bgntime, endtime;
                   in>>id>>bgnid>>endid>>bgntime>>endtime;

                   scene.Id(id);
                   scene.BeginFrameId(bgnid);
                   scene.EndFrameId(endid);
                   scene.BeginTime(bgntime);
                   scene.EndTime(endtime);

                    //load key frame 
                    int keyframeNum;
                    in>>keyframeNum;
			        for( int j = 0; j < keyframeNum; ++j )
                    {
                           CFrame keyframe;
			               in>>id;
                           keyframe.Id(id);

                           //!!!the key frame only id is validate!!!
                           scene.AddKeyframe(keyframe);
                    }

                    SceneList.Add(scene);
           }
          
            if( !in.good() )
                return E_FAIL;

           //close the load file
           in.close();
           return S_OK;       
   }
#pragma endregion
   
   HRESULT GetStringFromNode(IXMLDOMNode *pNode, BSTR* pbstr)
   {
          //test, whether the pointer is valid
		  if( pNode == NULL )
			  return E_INVALIDARG;
		
          //get Node value
		  return pNode->get_text(pbstr);
	}

	// Get IdArrays from given XPath Node
	HRESULT GetIdsArray(
		    IXMLDOMDocument* pDOM,
		    const wchar_t* wszNodeXPath,			//ids Node XPath
		    const wchar_t* wszNodeAttributePath,	//ids Node attribute to show elem num
		    vector<int>& idsArray			//output vector for saving ids
		)
	{
		 idsArray.clear();

         //test wether the pointer arg is not valid
		 if( pDOM==NULL )
			 return E_INVALIDARG;	
		

		 BSTR bstr = NULL;
		 IXMLDOMNode *pNode=NULL;
		 IXMLDOMNamedNodeMap* pAttributes = NULL;
		 IXMLDOMNodeList* pNodeList = NULL; 

         HRESULT hr = S_OK;
		 do
		 {
			  //read element number
			  bstr = SysAllocString(wszNodeXPath);
			  HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
			  pNode->get_attributes(&pAttributes);
			  SAFE_BSTR_RELEASE(bstr)
              SAFE_COM_RELEASE(pNode);

			  bstr = SysAllocString(wszNodeAttributePath);
			  pAttributes->getNamedItem(bstr, &pNode);
			  SAFE_BSTR_RELEASE(bstr);
		  	  HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
              // ids element number
			  long elemNum = _wtoi(bstr);	
			
			  SAFE_COM_RELEASE(pNode);
			  SAFE_BSTR_RELEASE(bstr)
			
			  bstr = SysAllocString(wszNodeXPath);
		 	  HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
		 	  HR_SUCCESSCALL( pNode->get_childNodes(&pNodeList), hr );

              //ids element number in list
			  long length = 0;
			  HR_SUCCESSCALL( pNodeList->get_length(&length), hr );	

              //test whether The xml file data has error. It is not consistent
			  if( elemNum != length )
			      break;
			
			  for( long index = 0; index < length; ++index )
			  {
				    SAFE_RELEASE(pNode);
				    HR_SUCCESSCALL( pNodeList->get_item(index, &pNode), hr );
				 
                    //get id string
                    SAFE_BSTR_RELEASE(bstr);
				    HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );

                    //push id into array
				    idsArray.push_back(_wtoi(bstr));
                    
		     }

             //test whether fail to get Ids list
			 HR_SUCCESSCALL( hr, hr );

		}while(false);
		
		SAFE_BSTR_RELEASE(bstr)

		SAFE_COM_RELEASE(pNode);
		SAFE_COM_RELEASE(pAttributes);
		SAFE_COM_RELEASE(pNodeList);

		return hr;
	}


    IXMLDOMDocument * DOMFromXMLFile(const wchar_t* fileName)
   {
		   IXMLDOMDocument *pxmldoc = NULL;
		   VARIANT_BOOL status;

           HRESULT  hr = S_OK;
		   do
		   {
			       //create XML DOM instance from COM
			       HR_SUCCESSCALL( CoCreateInstance(CLSID_DOMDocument30, 
	                                                                         NULL,
	                                                                         CLSCTX_INPROC_SERVER,
	                                                                         IID_IXMLDOMDocument,
	                                                                         (void**)&pxmldoc), hr );

			      HR_SUCCESSCALL( pxmldoc->put_async(VARIANT_FALSE), hr );
			      HR_SUCCESSCALL( pxmldoc->put_validateOnParse(VARIANT_FALSE), hr );
			      HR_SUCCESSCALL( pxmldoc->put_resolveExternals(VARIANT_FALSE), hr );

			      //load data from given XML file
                  _variant_t name = fileName;
                  HR_SUCCESSCALL( pxmldoc->load(name, &status), hr );

			      if( status != VARIANT_TRUE ) 
			          break;

		  }while(false);

         return pxmldoc;
	}
    
    HRESULT LoadOneKeyframe(IXMLDOMNode *pNode, CFrame & keyframe)
    {
         IXMLDOMNode *pChildNode = NULL;
         HRESULT hr = S_OK;
         do
         {
                 BSTR value = NULL;
                 HR_SUCCESSCALL( pNode->selectSingleNode(L"./FrameId", &pChildNode), hr );
	             HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
                 keyframe.Id(_wtoi(value));
                 SAFE_BSTR_RELEASE(value);
                 SAFE_COM_RELEASE(pChildNode);

                 HR_SUCCESSCALL( pNode->selectSingleNode(L"./BeginTime", &pChildNode), hr );
	             HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
                 keyframe.BeginTime(_wtoi64(value));
                 SAFE_BSTR_RELEASE(value);
                 SAFE_COM_RELEASE(pChildNode);

                 HR_SUCCESSCALL( pNode->selectSingleNode(L"./EndTime", &pChildNode), hr );
	             HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
                 keyframe.EndTime(_wtoi64(value));
                 SAFE_BSTR_RELEASE(value);
                 SAFE_COM_RELEASE(pChildNode);

         }while(false);

         return hr;
    }

    HRESULT LoadKeyframes(IXMLDOMNode* pParent, const wchar_t * xPath, CFrameList & keyframeList)
    {
         HRESULT hr = S_OK;

  		 IXMLDOMNode *pNode=NULL;
         IXMLDOMNode *pChildNode=NULL;
		 IXMLDOMNamedNodeMap* pAttributes = NULL;
		 IXMLDOMNodeList* pNodeList = NULL; 

         //read keyframe
         do
         {
              _bstr_t  path = xPath;
			  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );

			  pNode->get_attributes(&pAttributes);
			  SAFE_COM_RELEASE(pNode);

			  _bstr_t attribute = L"KeyFramesNum";
              BSTR value = NULL;
			  pAttributes->getNamedItem(attribute, &pNode);
		  	  HR_SUCCESSCALL( pNode->get_text(&value), hr );
              //keyframes element number
			  long elemNum = _wtoi(value);
              SAFE_BSTR_RELEASE(value);

               //get the <KeyFrames KeyFramesNum=" "> node
		 	  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );
		 	  HR_SUCCESSCALL( pNode->get_childNodes(&pNodeList), hr );

              //ids element number in list
			  long length = 0;
			  HR_SUCCESSCALL( pNodeList->get_length(&length), hr );	

              //test whether The xml file data has error. It is not consistent
			  if( elemNum != length )
			      return E_FAIL;
			
              //get key frame one by one
			  for( long i = 0; i < length; ++i )
			  {
				    SAFE_RELEASE(pNode);
				    HR_SUCCESSCALL( pNodeList->get_item(i, &pNode), hr );

                    CFrame keyframe;
                    HR_SUCCESSCALL( LoadOneKeyframe(pNode, keyframe), hr );
                    keyframeList.Add(keyframe);
              }

              //test whether fail to get key frames
              HR_SUCCESSCALL( hr, hr );

         }while(false);

         return hr;        
    }

    HRESULT LoadOneSubshot(IXMLDOMNode *pNode, CSubshot & subshot)
    {
         IXMLDOMNode *pChildNode = NULL;
         HRESULT hr = S_OK;
         //read one sub shot
         do
         {
                  //get subShot id
                  BSTR value = NULL;
				  HR_SUCCESSCALL( pNode->selectSingleNode(L"./SubShotId", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.Id(_wtoi(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);

                  //get Shot id
				  HR_SUCCESSCALL( pNode->selectSingleNode(L"./ShotId", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.ShotId(_wtoi(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);
				
				  //get subShot boundary
                  //get the begin frame id
                  HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/BgnFrameId", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.BeginFrameId(_wtoi(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);

                  //get the end frame id
                  HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/EndFrameId", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.EndFrameId(_wtoi(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);

                  //get the begin time
                  HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/BgnFrameTime", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.BeginTime(_wtoi64(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);

                  //get the end time
                  HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/EndFrameTime", &pChildNode), hr );
				  HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				  subshot.EndTime(_wtoi64(value));
				  SAFE_BSTR_RELEASE(value)
				  SAFE_COM_RELEASE(pChildNode);

				   CFrameList keyframeList;
				   HR_SUCCESSCALL( LoadKeyframes(pNode, L"./KeyFrames", keyframeList), hr );
				  
                   int size =  keyframeList.Size();
                   for( int kf_index = 0; kf_index < size; ++kf_index)
                          subshot.AddKeyframe(keyframeList[kf_index]);

         }while(false);

         return hr;
    }
    HRESULT LoadSubshots(IXMLDOMNode* pParent, const wchar_t * xPath, CVideoSegmentList & subshotList)
    {
         HRESULT hr = S_OK;

  		 IXMLDOMNode *pNode=NULL;
         IXMLDOMNode *pChildNode=NULL;
		 IXMLDOMNamedNodeMap* pAttributes = NULL;
		 IXMLDOMNodeList* pNodeList = NULL; 

         //read sub shot
         do
         {
              //select  Subshots
              _bstr_t  path = xPath;
			  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );

			  pNode->get_attributes(&pAttributes);
			  SAFE_COM_RELEASE(pNode);

              //read sub shot number
			  _bstr_t attribute = L"SubshotsNum";
              BSTR value = NULL;
			  pAttributes->getNamedItem(attribute, &pNode);
		  	  HR_SUCCESSCALL( pNode->get_text(&value), hr );
              //sub shot element number
			  long elemNum = _wtoi(value);
              SAFE_BSTR_RELEASE(value);

              //get the <Subshots SubshotsNum=" "/> node
		 	  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );
		 	  HR_SUCCESSCALL( pNode->get_childNodes(&pNodeList), hr );

              //ids element number in list
			  long length = 0;
			  HR_SUCCESSCALL( pNodeList->get_length(&length), hr );	

              //test whether The xml file data has error. It is not consistent
			  if( elemNum != length )
			      return E_FAIL;
			
              //get sub shot one by one
			  for( long i = 0; i < length; ++i )
			  {
				    SAFE_RELEASE(pNode);
				    HR_SUCCESSCALL( pNodeList->get_item(i, &pNode), hr );

                    CSubshot subshot;
                    HR_SUCCESSCALL( LoadOneSubshot(pNode, subshot), hr );
                    subshotList.Add(subshot);
              }

              //test whether fail to get sub shots
              HR_SUCCESSCALL( hr, hr );

         }while(false);

         return hr;        
    }
    
    HRESULT LoadOneShot(IXMLDOMNode *pNode, CShot & shot)
    {
         IXMLDOMNode *pChildNode = NULL;
         HRESULT hr = S_OK;
         //read one sub shot
         do
         {
                //get Shot id
                BSTR value = NULL;
			    HR_SUCCESSCALL( pNode->selectSingleNode(L"./ShotId", &pChildNode), hr );
			    HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
			    shot.Id(_wtoi(value));
                SAFE_BSTR_RELEASE(value);
                SAFE_COM_RELEASE(pChildNode);
			
			    //get Shot boundary
                //the begin frame id
			    HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/BgnFrameId", &pChildNode), hr );
			    HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
			    shot.BeginFrameId(_wtoi(value));
			    SAFE_BSTR_RELEASE(value);
			    SAFE_RELEASE(pChildNode);

                //the end frame id
			    HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/EndFrameId", &pChildNode), hr );
			    HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				shot.EndFrameId(_wtoi(value));
			    SAFE_BSTR_RELEASE(value);
			    SAFE_RELEASE(pChildNode);

                 //the begin time
			     HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/BgnFrameTime", &pChildNode), hr );
			     HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
			     shot.BeginTime(_wtoi64(value));
			     SAFE_BSTR_RELEASE(value);
			     SAFE_RELEASE(pChildNode);
                
                 //the end time
			     HR_SUCCESSCALL( pNode->selectSingleNode(L"./Boundary/EndFrameTime", &pChildNode), hr );
			     HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
				 shot.EndTime(_wtoi64(value));
			     SAFE_BSTR_RELEASE(value);
			     SAFE_RELEASE(pChildNode);

				 //load the sub shots
                 CVideoSegmentList subshotList;
                 HR_SUCCESSCALL( LoadSubshots(pNode, L"./Subshots", subshotList), hr );

                 int size =  subshotList.Size();
                 for( int i = 0; i < size; ++i )
                       shot.AddChild(subshotList[i]);

                 //load the key frames
                 CFrameList keyframeList;
				 HR_SUCCESSCALL( LoadKeyframes(pNode, L"./KeyFrames", keyframeList), hr );
				  
                 size =  keyframeList.Size();
                 for( int i = 0; i < size; ++i )
                       shot.AddKeyframe(keyframeList[i]);

         }while(false);

         return hr;
    }

    HRESULT LoadShots(IXMLDOMNode* pParent, const wchar_t * xPath, CVideoSegmentList & shotList)
    {
         HRESULT hr = S_OK;

  		 IXMLDOMNode *pNode=NULL;
         IXMLDOMNode *pChildNode=NULL;
		 IXMLDOMNamedNodeMap* pAttributes = NULL;
		 IXMLDOMNodeList* pNodeList = NULL; 

         //read shot
         do
         {
              //select  Shots
              _bstr_t  path = xPath;
			  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );

			  pNode->get_attributes(&pAttributes);
			  SAFE_COM_RELEASE(pNode);

              //read shot number
			  _bstr_t attribute = L"ShotsNum";
              BSTR value = NULL;
			  pAttributes->getNamedItem(attribute, &pNode);
		  	  HR_SUCCESSCALL( pNode->get_text(&value), hr );
              //shot element number
			  long elemNum = _wtoi(value);
              SAFE_BSTR_RELEASE(value);

              //get the <Shots ShotsNum=""> node
		 	  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );
		 	  HR_SUCCESSCALL( pNode->get_childNodes(&pNodeList), hr );

              //ids element number in list
			  long length = 0;
			  HR_SUCCESSCALL( pNodeList->get_length(&length), hr );	

              //test whether The xml file data has error. It is not consistent
			  if( elemNum != length )
			      return E_FAIL;
			
              //get shot one by one
			  for( long i = 0; i < length; ++i )
			  {
				    SAFE_RELEASE(pNode);
				    HR_SUCCESSCALL( pNodeList->get_item(i, &pNode), hr );

                    CShot shot;
                    HR_SUCCESSCALL( LoadOneShot(pNode, shot), hr);
                    shotList.Add(shot);
              }

              //test whether fail to get shots
              HR_SUCCESSCALL( hr, hr );

         }while(false);

         return hr;        
    }


    HRESULT LoadOneScene(IXMLDOMNode *pNode, CScene & scene)
    {
         IXMLDOMNode *pChildNode = NULL;
         HRESULT hr = S_OK;

         //read one scene
         do
         {
				//get Scene id
                BSTR value = NULL;
				HR_SUCCESSCALL( pNode->selectSingleNode(L"./SceneId", &pChildNode), hr );
				HR_SUCCESSCALL( GetStringFromNode(pChildNode, &value), hr );
			    scene.Id(_wtoi(value));
                SAFE_BSTR_RELEASE(value);
                SAFE_COM_RELEASE(pChildNode);
			
				 //load the shots
                 CVideoSegmentList shotList;
                 HR_SUCCESSCALL( LoadShots(pNode, L"./Shots", shotList), hr );

                 int size =  shotList.Size();
                 for( int i = 0; i < size; ++i )
                       scene.AddChild(shotList[i]);

                 //load the key frames
                 CFrameList keyframeList;
				 HR_SUCCESSCALL( LoadKeyframes(pNode, L"./KeyFrames", keyframeList), hr );
				  
                 size =  keyframeList.Size();
                 for( int i = 0; i < size; ++i )
                       scene.AddKeyframe(keyframeList[i]);

         }while(false);

         return hr;
    }


    HRESULT LoadScenes(IXMLDOMNode* pParent, const wchar_t * xPath, CVideoSegmentList & sceneList)
    {
         HRESULT hr = S_OK;

  		 IXMLDOMNode *pNode=NULL;
         IXMLDOMNode *pChildNode=NULL;
		 IXMLDOMNamedNodeMap* pAttributes = NULL;
		 IXMLDOMNodeList* pNodeList = NULL; 

         //read scene
         do
         {
              //select Scenes
              _bstr_t  path = xPath;
			  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );

			  pNode->get_attributes(&pAttributes);
			  SAFE_COM_RELEASE(pNode);

              //read scene number
			  _bstr_t attribute = L"ScenesNum";
              BSTR value;
			  pAttributes->getNamedItem(attribute, &pNode);
		  	  HR_SUCCESSCALL( pNode->get_text(&value), hr );
              //scenes element number
			  long elemNum = _wtoi(value);
              SAFE_BSTR_RELEASE(value);

              //get the Scenes ScenesNum=""> node
		 	  HR_SUCCESSCALL( pParent->selectSingleNode(path, &pNode), hr );
		 	  HR_SUCCESSCALL( pNode->get_childNodes(&pNodeList), hr );

              //ids element number in list
			  long length = 0;
			  HR_SUCCESSCALL( pNodeList->get_length(&length), hr );	

              //test whether The xml file data has error. It is not consistent
			  if( elemNum != length )
			      return E_FAIL;
			
              //get scene one by one
			  for( long i = 0; i < length; ++i )
			  {
				    SAFE_RELEASE(pNode);
				    HR_SUCCESSCALL( pNodeList->get_item(i, &pNode), hr );

                    CScene scene;
                    HR_SUCCESSCALL( LoadOneScene(pNode, scene), hr);
                    sceneList.Add(scene);
              }

              //test whether fail to scenes
              HR_SUCCESSCALL( hr, hr );

         }while(false);

         return hr;        
    }


    //get sub shot list from DOM
	HRESULT GetSubshotList(IXMLDOMDocument* pDOM, CVideoSegmentList & subshotList)
	{
         //test wether the pointer arg is not valid
		 if( pDOM == NULL )
		 	 return E_INVALIDARG;	
		
         HRESULT hr = S_OK;

		 do
		 {
            subshotList.Clear();
			//read sub shot one by one
            HR_SUCCESSCALL( LoadSubshots(pDOM, L"//Subshots",  subshotList), hr );
   
		}while(false);

		return hr;
	}


    //get shot list from DOM
	HRESULT GetShotList(IXMLDOMDocument* pDOM, CVideoSegmentList & shotList)
	{
          //pointer arg is not valid
		  if( pDOM==NULL )
		      return E_INVALIDARG;	
		 
          HRESULT hr = S_OK;
          do
		  {
               shotList.Clear();
		       //read shot one by one
               HR_SUCCESSCALL( LoadShots(pDOM, L"//Shots",  shotList), hr );
          	
		  }while(false);

		  return hr;
	}


    //get scene list from DOM
    HRESULT GetSceneList(IXMLDOMDocument* pDOM, CVideoSegmentList & sceneList)
	{
          //test whether the pointer arg is not valid
		  if( pDOM == NULL )
		      return E_INVALIDARG;	
		  
          HRESULT hr = S_OK;
          do
		  {
               sceneList.Clear();
		       //read scene one by one
               HR_SUCCESSCALL( LoadScenes(pDOM, L"//Scenes",  sceneList), hr );
          	
		  }while(false);

		  return hr;
   }

   	//get Video information from DOM
	HRESULT GetVideoInfo(IXMLDOMDocument* pDOM, CVideoInfo & videoInfo, FileInfo & fileInfo )
	{
        //test whether the pointer arg is not valid
		if( pDOM == NULL )
		    return E_INVALIDARG;	
		
		IXMLDOMNode *pNode=NULL;
        BSTR bstr = NULL;

        HRESULT hr = S_OK;
		do
		{
			// File Name
			BSTR bstr = SysAllocString(L"//FileName");
			HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
            SAFE_BSTR_RELEASE(bstr);
			HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
            wcscpy_s(fileInfo.m_FileName, MAX_PATH, bstr);
			SAFE_RELEASE(pNode);
			SAFE_BSTR_RELEASE(bstr);


			// File Duration
			bstr = SysAllocString(L"//Duration");
			HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
			HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
            videoInfo.SetDuration( _wtof(bstr));
			SAFE_RELEASE(pNode);
			SAFE_BSTR_RELEASE(bstr);

             // FileByteSize
			 bstr = SysAllocString(L"//FileByteSize");
			 HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
			 SAFE_BSTR_RELEASE(bstr);
             HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
             videoInfo.SetFileSize(_wtoi(bstr));
			 SAFE_RELEASE(pNode);
			 SAFE_BSTR_RELEASE(bstr);

			 // Frame Number
			 bstr = SysAllocString(L"//FramesNum");
			 HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
			 SAFE_BSTR_RELEASE(bstr);
             HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
             videoInfo.SetFrameNum(_wtoi(bstr));
			 SAFE_RELEASE(pNode);
			 SAFE_BSTR_RELEASE(bstr);

			 // Frame Width
			 bstr = SysAllocString(L"//FrameWidth");
			 HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
             SAFE_BSTR_RELEASE(bstr);
             HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
             videoInfo.SetFrameWidth(_wtoi(bstr));
			 SAFE_RELEASE(pNode);
			 SAFE_BSTR_RELEASE(bstr);

			 // Frame Height
			 bstr = SysAllocString(L"//FrameHeight");
			 HR_SUCCESSCALL( pDOM->selectSingleNode(bstr, &pNode), hr );
             SAFE_BSTR_RELEASE(bstr);
			 HR_SUCCESSCALL( GetStringFromNode(pNode, &bstr), hr );
			 videoInfo.SetFrameHeight(_wtoi(bstr));
			 SAFE_RELEASE(pNode);
			 SAFE_BSTR_RELEASE(bstr);


		}while(false);

		SAFE_BSTR_RELEASE(bstr);
		SAFE_RELEASE(pNode);

		return E_FAIL;	// Get failure
	}

#pragma region XMLLoadAPI
    //load a shot list from xml file
    HRESULT LoadShotsFromXML(CVideoSegmentList & ShotList, const wchar_t * const wszFullPathFileName)
    {
           		
		  //create DOM from xml file
		  IXMLDOMDocument *pDOM = DOMFromXMLFile(wszFullPathFileName);
          if( pDOM == NULL ) 
		      return E_FAIL;
		
          HRESULT hr = GetShotList(pDOM, ShotList);

		  SAFE_COM_RELEASE(pDOM);

          return hr;
    }

    HRESULT LoadSubshotsFromXML(CVideoSegmentList & SubshotList, const wchar_t * const wszFullPathFileName)
    {
		  //create DOM from xml file
		  IXMLDOMDocument *pDOM = DOMFromXMLFile(wszFullPathFileName);
          if( pDOM == NULL ) 
		      return E_FAIL;
		
          HRESULT hr = GetSubshotList(pDOM, SubshotList);

		  SAFE_COM_RELEASE(pDOM);

          return hr;
    }

    HRESULT LoadScenesFromXML(CVideoSegmentList & SceneList, const wchar_t * const wszFullPathFileName)
    {
		  //create DOM from xml file
		  IXMLDOMDocument *pDOM = DOMFromXMLFile(wszFullPathFileName);
          if( pDOM == NULL ) 
		      return E_FAIL;
		
          HRESULT hr = GetSceneList(pDOM, SceneList);

		  SAFE_COM_RELEASE(pDOM);

          return hr;    
    }

    //load the video info from the xml
    HRESULT LoadVideoInfoFromXML(CVideoInfo & videoInfo, FileInfo & fileInfo, const wchar_t * const wszFullPathFileName)
    {
		  //create DOM from xml file
		  IXMLDOMDocument *pDOM = DOMFromXMLFile(wszFullPathFileName);
          if( pDOM == NULL ) 
		      return E_FAIL;
		
          HRESULT hr = GetVideoInfo(pDOM, videoInfo, fileInfo);

		  SAFE_COM_RELEASE(pDOM);

          return hr;
   }

#pragma endregion
}