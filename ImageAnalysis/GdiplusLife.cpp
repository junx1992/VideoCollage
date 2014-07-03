#include "StdAfx.h"

#include "GdiplusLife.h"

#include "unknwn.h"
#include "gdiplus.h"

namespace ImageAnalysis 
{
	static Gdiplus::GdiplusStartupInput gdiSI;
    static Gdiplus::GdiplusStartupOutput gdiSO;
    static ULONG_PTR gdiToken;
    static ULONG_PTR gdiHookToken;
	static bool m_bInitialized = false;

	void CGdiplusLife::Initialize()
    {
		if(m_bInitialized)
			return;
        gdiSI.SuppressBackgroundThread = TRUE;
        Gdiplus::GdiplusStartup(&gdiToken,&gdiSI,&gdiSO);
        gdiSO.NotificationHook(&gdiHookToken);
    }

	void CGdiplusLife::Uninitialize()
    {
		if(!m_bInitialized)
			return;
        gdiSO.NotificationUnhook(gdiHookToken);
        Gdiplus::GdiplusShutdown(gdiToken);
    }

	CGdiplusLife::CGdiplusLife()
	{
		Initialize();
	}

	CGdiplusLife::~CGdiplusLife()
	{
		Uninitialize();
	}
}

        