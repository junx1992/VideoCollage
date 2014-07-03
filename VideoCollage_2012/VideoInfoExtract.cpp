#include "Stdafx.h"
#include "Common.h"
#include "VideoAnalysisEngine.h"
#include "VideoSegmentStorage.h"
#include "featureExtract.h"
#include "VaUtils.h"
#include "VxUtils.h"
#include <Windows.h>
#include <errno.h>
#include <string>
#include <iostream>
using namespace std;
using namespace VxCore;
using namespace VideoAnalysis;

bool VideoInfoSave(const wstring & srcFile, const wstring & dstDir, const MetaDataExtractConfig & extConfig)
{
	//parse video file name
	CVideoInfo videoinfo(srcFile.c_str());
	FileInfo fileinfo(srcFile.c_str());
	//get the video info
	videoinfo.SetFrameWidth(extConfig.vaEngine.FrameWidth());
	videoinfo.SetFrameHeight(extConfig.vaEngine.FrameHeight());
	videoinfo.SetFrameNum(extConfig.vaEngine.FrameNum());

	//save the vidoe info
	wstring savedXMLFile = dstDir + L"\\videoinfo.xml";
	return (SaveVideoInfoToXML(videoinfo, fileinfo, savedXMLFile.c_str()) == S_OK);
}