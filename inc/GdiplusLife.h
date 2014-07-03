/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Image Sdk

Abstract:
  This header file provide the gdiplus initialization functions used by image sdk 

Notes:
  

History:
  Created on 08/15/2007 by linjuny@microsoft.com
\******************************************************************************/


namespace ImageAnalysis 
{
	///class to startup and shutdown gdiplus
    class CGdiplusLife
    {
    public:
		///startup gdiplus
        static void Initialize();
		///shutdown gdiplus
        static void Uninitialize();
		///in this ctor, gdiplus startup
		CGdiplusLife();
		///in this dtor, gdiplus shutdown
		~CGdiplusLife();
	};


}   /// namespace helper
