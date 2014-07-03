#include<stdio.h>
#include<string>
#include<iostream>
#include<fstream>
#include"VideoCollage.h"
#include<cstring>
#include<Windows.h>
#include<sstream>
#include<direct.h>
#include "global.h"
#include <opencv2/opencv.hpp>
#include<io.h>
#include"common.h"
#include"featureExtract.h"
#include"FrameJSegmentor.h"
#include "GdiplusLife.h"
#include "SabOnlineSubshotDetector.h"
#include "VideoSegments.h"
#include "VideoSegmentStorage.h"
#include "FrameJSegmentor.h"
#include "VaUtils.h"
#include<weight.h>
#include"ColorEntropyFeatureExtractor.h";
#include"DifferenceFeatureExtractor.h";
#include"ShotFeatureExtractor.h";
#include"SkinFeatureExtractor.h";
#include "EntropyOfflineSceneDetector.h";
#include"EnhanceImage.h";
#include"CUser.h";
#include "DThreshShotDetector.h"
#include"AdvancedKeyframeExtractor.h"


#define MAX_SCENE 1000
#define MAX_TOPFRAME 36

using namespace VxCore;
using namespace ImageAnalysis;
using namespace VideoAnalysis;
using namespace std;
using namespace cv;

POINT g_curPos;

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::string ws2s(const std::wstring& ws)
{
     std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
     setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
     size_t _Dsize = 2 * ws.size() + 1;
    char *_Dest = new char[_Dsize];
     memset(_Dest,0,_Dsize);
     wcstombs(_Dest,_Source,_Dsize);
     std::string result = _Dest;
     delete []_Dest;
     setlocale(LC_ALL, curLocale.c_str());
    return result;
}

VideoCollage::VideoCollage(string videopath, int rio_num,BOOL isselect)
{
	m_strVideoFile = videopath;
	_rionum = rio_num;
	h_skip = 30;
	m_keyframeImage = NULL;
	int pos=m_strVideoFile.find_last_of('.');
	m_select=isselect;
	if(m_select)
		m_resultfolder=m_strVideoFile.substr(0,pos)+"_resultfolder_feature";
	else
		m_resultfolder=m_strVideoFile.substr(0,pos)+"_resultfolder";
	LoadArrange();
	Loadprofile();
	int num=m_nKeyFrameNum>_rionum?_rionum:m_nKeyFrameNum;
	GenerateCollage(num,FALSE);
	GenerateCollage(num, TRUE);
	
}

void VideoCollage::LoadArrange()
{
	FILE* f = fopen("arrange.ini", "r");
	if (f == NULL)
	{
		cout << "Error! Cannot open arrange.ini!" << endl;
		return;
	}
	char temp[100];
	int num = 0;
	leastImageNum = 100000;
	maxImageNum = 0;
	while (!(EOF == fscanf(f, "%d\t%s\n", &num, temp)))
	{
		if (num<leastImageNum)
			leastImageNum = num;
		if (num>maxImageNum)
			maxImageNum = num;

		int lineNum = 0;
		while (1)
		{
			sscanf(temp, "%d,%s\n", &lineNum, temp);
			m_picArrange[num].push_back(lineNum);
			if (strchr(temp, ',') == NULL)
			{
				m_picArrange[num].push_back(atoi(temp));
				break;
			}
		}
	}
	fclose(f);
	oriMaxImageNum = maxImageNum;
}

void VideoCollage::Loadprofile()
{
	int n = ReadKeyFrameIndex();
	ReadVideoInfo();
	if (n <0)
	{
		return;
	}

	int heigh = m_keyframeHeight;// to load heigh
	int width = m_keyframeWidth;// to load width
	int x, y;
	if (m_keyframeImage != NULL)
	{
		cvReleaseImage(&m_keyframeImage);
		m_keyframeImage = NULL;
	}

	m_keyframeImage = cvCreateImage(cvSize((width + h_skip)*n, heigh), 8, 3);

	cvZero(m_keyframeImage);
	IplImage* pTemp = cvCreateImage(cvSize(width, heigh), 8, 3);
	IplImage* pNewSrc = cvCreateImage(cvSize(320, 240), 8, 3);
	pNewSrc->origin = 1;

	int iAttWidth = pNewSrc->width;
	int iAttHeight = pNewSrc->height;
	int iStride = iAttWidth * 3, iBits = 24;
	int iAttNum = 0, iSaliencyWidth = 0, iSaliencyHeight = 0;
	BOOL fFaceAvailable = TRUE;
	BYTE *pData = new BYTE[iAttWidth*iAttHeight * 3];
	string str;

	//str = m_resultfolder+"\\CLIPS\\";                  //create the clipps dir;
	//_mkdir(str.c_str());

							
	str = m_resultfolder+"\\ROI\\";  //create the roi dir;
	_mkdir(str.c_str());

	CFocusDetector *pFocusDetector = new CFocusDetector;
	int i = 0;
	int x_offset = h_skip / 2;
	while (i<m_nKeyFrameNum)
	{
		long temp = m_pRoiInfo[i].lFrmNo;
		m_pRoiInfo[i].index = i;
		x = i / 4; y = i % 4;
		//m_KeyFrameInfo[i].rect = CRect(y*width,x*heigh,(y+1)*width,(x+1)*heigh); 
		char temp_char[100];
		_itoa(temp, temp_char, 10);
		string temp_str = temp_char;
		string image;
		if(m_select==TRUE)
			image = m_resultfolder+"\\Selected\\" + temp_str + (string)".jpg";		
		else
			image = m_resultfolder+"\\Keyframe\\" + temp_str + (string)".jpg";		
		//IplImage* pImage = m_pVideo->GetIplImage(temp);
		IplImage* pImage = cvLoadImage(image.c_str());
		string str1;
		
#ifdef _SAVETEMP		
		//str1.Format("%d_test.bmp", temp);
		str1 = "" + temp + (string)"_test.bmp";
		str1 = str + str1;
		cvSaveImage(str1, pImage);
#endif	
		cvZero(pTemp);
		cvResize(pImage, pTemp);
		CvRect r;
		r.x = x_offset; r.y = 0; r.width = width; r.height = heigh;
		cvSetImageROI(m_keyframeImage, r);
		cvCopy(pTemp, m_keyframeImage);
		cvResetImageROI(m_keyframeImage);

		x_offset += h_skip + width;

		cvZero(pNewSrc);
		cvResize(pImage, pNewSrc, CV_INTER_LINEAR);

#ifdef _SAVETEMP
		//str1.Format("%d_test1.bmp", temp); //draw ROI
		str1 = "" + temp + (string)"_test1.bmp";
		str1 = str + str1;
		cvSaveImage(str1, pNewSrc);
#endif

		memcpy(pData, pNewSrc->imageData, iAttWidth*iAttHeight * 3);

		HRESULT hr = pFocusDetector->Initialize(pData, iAttWidth, iAttHeight, iStride, iBits);
		if (S_OK != hr)
		{
			//( "\nError: failed to initialize focusdetector!\n" );
			delete pFocusDetector;
			return;
		}
		pFocusDetector->SetFaceInformation(NULL, 0);
		RECT rect;
		//rect.bottom =rect.left=rect.right=rect.top =0;
		pFocusDetector->ExtractAttendedView(&rect);
		pFocusDetector->DeInitialize();

		r.x = rect.left; r.y = rect.top; r.width = rect.right - rect.left; r.height = rect.bottom - rect.top;

		double widthRatio = ((double)pImage->width) / ((double)pNewSrc->width);
		double heightRatio = ((double)pImage->height) / ((double)pNewSrc->height);
		r.x *= widthRatio;
		r.y *= heightRatio;
		r.width *= widthRatio;
		r.height *= heightRatio;

		cvSetImageROI(pImage, r);

		//str1.Format("%d.jpg", temp); //draw ROI
		str1 =temp_str + (string)"_roi.jpg";
		str1 = str + str1;
		cvSaveImage(str1.c_str(), pImage);
		cvResetImageROI(pNewSrc);
		cvReleaseImage(&pImage);
		i++;
	}
	delete pFocusDetector;
	cvReleaseImage(&pTemp);
	cvReleaseImage(&pNewSrc);

	IplImage* pKeyframe = cvCreateImage(cvSize(width, heigh), 8, 3);

	/*CvMat tempMat;
	CvMat* tempP;
	tempP = cvGetSubRect(m_keyframeImage, &tempMat, cvRect(0, 0, rect.right, heigh));
	cvCopy(tempP, pKeyframe);*/
	//	cvReleaseMat(&tempP);
	for (int y = 0; y<heigh; y++)
	{
		int index = y*pKeyframe->widthStep;
		int ori_index = y*m_keyframeImage->widthStep;
		for (int x = 0; x<width * 3; x++)
		{
			pKeyframe->imageData[index + x] = m_keyframeImage->imageData[ori_index + x];
		}
	}
	cvReleaseImage(&pKeyframe);
	delete[] pData;
}

int VideoCollage::ReadKeyFrameIndex(void)
{
	string strKeyFrameFile; //read the keyframe profile and get the index of keyframe
	//strKeyFrameFile += "_KEYFRAME.Txt";
	int KeyFrame = 0;
	std::vector<long> vtKeyFrame;

	if(m_select==TRUE)
	{

		strKeyFrameFile+=m_resultfolder+"\\Selected\\SelectedList.txt";
		ifstream in(strKeyFrameFile);
		string line;
		while(getline(in,line))
		{
			if(line!="")
			{
				string temp=line;			
				KeyFrame=atoi(temp.c_str());
				vtKeyFrame.push_back(KeyFrame);
			}
		}
		in.close();
	}
	else
	{
		strKeyFrameFile+=m_resultfolder+"\\Structure\\Shot.txt";
		ifstream in(strKeyFrameFile);
		string line;
		getline(in,line);
		while(getline(in,line))
		{		
			int pos=line.find_last_of('\t');
			string temp=line.substr(0,pos);
			pos=temp.find_last_of('\t');
			temp=temp.substr(pos+1,temp.length()-1);
			KeyFrame=atoi(temp.c_str());
			vtKeyFrame.push_back(KeyFrame);
		}
		in.close();
	}
	
	m_nKeyFrameNum = vtKeyFrame.size();
	m_pRoiInfo = new ROIINFO[m_nKeyFrameNum];
	m_pRoiInfo_1D = new ROIINFO[m_nKeyFrameNum];
	for (int i = 0; i<m_nKeyFrameNum; i++)
	{
		m_pRoiInfo[i].lFrmNo = vtKeyFrame[i];
		m_pRoiInfo[i].index = i;
		m_pRoiInfo_1D[i].lFrmNo = vtKeyFrame[i];
		m_pRoiInfo_1D[i].index = i;
	}

	if (m_nKeyFrameNum < maxImageNum)
		maxImageNum = m_nKeyFrameNum;
	return m_nKeyFrameNum;
}

void VideoCollage::ReadVideoInfo()
{
	string info_file=m_resultfolder+"\\Structure\\size.txt";
	ifstream in_size(info_file);
	if(in_size)
	{
	in_size>>m_keyframeHeight;
	in_size>>m_keyframeWidth;
	in_size.close();
	}
	else
	{
		cout<<"Video Info File Not Found"<<endl;
		in_size.close();
		system("pause");
		exit(0);
	}
	

}

void VideoCollage::GenerateCollage(int num, BOOL oneD)
{
	int* Line;
	int nLine;
	if (oneD)
	{
		Line = new int[1]; Line[0] = num;
		nLine = 1;
	}
	else
	{
		nLine = m_picArrange[num].size();
		Line = new int[nLine];
		for (int i = 0; i<nLine; i++)
			Line[i] = m_picArrange[num][i];
	}


	string dir;

	dir = m_resultfolder+"\\ROI\\";


	string file1;
	for (int i = 0; i<m_nKeyFrameNum; i++) //the roi of the keyframe;
	{
		int temp = m_pRoiInfo[i].lFrmNo;
		char temp_char[100];
		_itoa(temp, temp_char, 10);
		string temp_str = temp_char;
		file1 =temp_str + (string)"_roi.jpg";

		//file1.Format("%d.jpg", m_pRoiInfo[i].lFrmNo);
		file1 = dir + file1;
		IplImage *pImage = cvLoadImage(file1.c_str(), 1);
		float fImpt = GetImportance((BYTE*)pImage->imageData, pImage->width,
			pImage->height, pImage->width * 3);
		m_pRoiInfo[i].fImport = fImpt;
		m_pRoiInfo_1D[i].fImport = fImpt;
		cvReleaseImage(&pImage);
	}

	//对所有的ROI,按import进行排序
	if (oneD)
	{
		for (int i = 0; i<m_nKeyFrameNum; i++)
		{
			int t = 0;
			for (int j = 0; j<m_nKeyFrameNum; j++)
			{
				if (m_pRoiInfo_1D[i].fImport < m_pRoiInfo_1D[j].fImport)
				{
					t++;
				}
				m_pRoiInfo_1D[i].ImptIdx = t;
			}
		}
	}
	else
	{
		for (int i = 0; i<m_nKeyFrameNum; i++)
		{
			int t = 0;
			for (int j = 0; j<m_nKeyFrameNum; j++)
			{
				if (m_pRoiInfo[i].fImport < m_pRoiInfo[j].fImport)
				{
					t++;
				}
				m_pRoiInfo[i].ImptIdx = t;
			}
		}
	}

	// 选择前面num个roi,生成collage
	int index = -1;
	std::vector<ROIINFO> vtline;

	IplImage** result = new IplImage*[nLine];

	int j = 0;
	g_curPos.x = 0;
	g_curPos.y = 0;
	RECT rect;
	//GetDlgItem(IDC_STATIC_COLLAGE)->GetClientRect(&rect);

	if (oneD)
	{
		for (int i = 0; i<nLine; i++)
		{
			vtline.clear();
			j = 0;
			while (j<Line[i])
			{
				index++;
				if (m_pRoiInfo_1D[index].ImptIdx >= num)
				{
					m_pRoiInfo_1D[index].rect.left =
						m_pRoiInfo_1D[index].rect.right =
						m_pRoiInfo_1D[index].rect.bottom =
						m_pRoiInfo_1D[index].rect.top = 0;
					continue;
				}
				vtline.push_back(m_pRoiInfo_1D[index]);
				j++;
			}
			result[i] = MergeImage(vtline, TRUE);
			float ratio = result[0]->width / (float)result[i]->width;
			for (int k = 0; k<vtline.size(); k++)
			{
				int idx = vtline[k].index;
				m_pRoiInfo_1D[idx].rect.left *= ratio;
				m_pRoiInfo_1D[idx].rect.right *= ratio;
				m_pRoiInfo_1D[idx].rect.bottom = m_pRoiInfo_1D[idx].rect.top +
					(m_pRoiInfo_1D[idx].rect.bottom - m_pRoiInfo_1D[idx].rect.top)*ratio;
			}

			g_curPos.x = 0; //当前点的位置
			g_curPos.y += m_pRoiInfo_1D[vtline[0].index].rect.bottom -
				m_pRoiInfo_1D[vtline[0].index].rect.top/*result[i]->height/ratio*/;

#ifdef _SAVETEMP
			CString str;
			str.Format("%d.bmp", i);
			cvSaveImage(str, result[i]);
#endif
		}
	}
	else
	{

		for (int i = 0; i<nLine; i++)
		{
			vtline.clear();
			j = 0;
			while (j<Line[i])
			{
				index++;
				if (m_pRoiInfo[index].ImptIdx >= num)
				{
					m_pRoiInfo[index].rect.left =
						m_pRoiInfo[index].rect.right =
						m_pRoiInfo[index].rect.bottom =
						m_pRoiInfo[index].rect.top = 0;
					continue;
				}
				vtline.push_back(m_pRoiInfo[index]);
				j++;
			}
			result[i] = MergeImage(vtline, FALSE);
			float ratio = result[0]->width / (float)result[i]->width;
			for (int k = 0; k<vtline.size(); k++)
			{
				int idx = vtline[k].index;
				m_pRoiInfo[idx].rect.left *= ratio;
				m_pRoiInfo[idx].rect.right *= ratio;
				m_pRoiInfo[idx].rect.bottom = m_pRoiInfo[idx].rect.top +
					(m_pRoiInfo[idx].rect.bottom - m_pRoiInfo[idx].rect.top)*ratio;
			}

			g_curPos.x = 0; //当前点的位置
			g_curPos.y += m_pRoiInfo[vtline[0].index].rect.bottom -
				m_pRoiInfo[vtline[0].index].rect.top/*result[i]->height/ratio*/;

#ifdef _SAVETEMP
			CString str;
			str.Format("%d.bmp", i);
			cvSaveImage(str, result[i]);
#endif
		}
	}


	IplImage* pCollage = NULL;
	if (nLine>1)
		pCollage = Merge2(result[0], result[1], false);
	else
		pCollage = cvCloneImage(result[0]);

#ifdef _SAVETEMP
	CString str;
	str.Format("p_%d.bmp", i);
	cvSaveImage(str, pCollage);
#endif _SAVETEMP

	for (int i = 2; i<nLine; i++)
	{
		IplImage* tmp = pCollage;
		pCollage = Merge2(pCollage, result[i], false);

#ifdef _SAVETEMP
		CString str;
		str.Format("p_%d.bmp", i);
		cvSaveImage(str, pCollage);
#endif

		cvReleaseImage(&tmp);
	}
	for (int i = 0; i<nLine; i++)
	{
		cvReleaseImage(&result[i]);
	}

	//写文件
	string file;
	//if (m_pVideo != NULL)
	//{
	//	int pos = m_strVideoFile.ReverseFind('\\');
	//	dir = m_strVideoFile.Left(pos + 1);
	//	CString name = m_strVideoFile.Mid(pos + 1, m_strVideoFile.ReverseFind('.') - pos - 1);
	//	dir += name + "_collage\\";
	//	if (_access(dir, 0) == -1)
	//		_mkdir(dir);
	//	file = dir + name;
	//}
	//else
	//{
	//	dir = m_strPhotoPath + "collage\\";
	//	if (_access(dir, 0) == -1)
	//		_mkdir(dir);
	//	file = dir + "photo";
	//}

    int pos = m_strVideoFile.find_last_of('\\');
	string name = m_strVideoFile.substr(pos + 1, m_strVideoFile.find_last_of('.') - pos - 1);
	dir =m_resultfolder + "\\CollageResult\\";
	_mkdir(dir.c_str());
	file = dir + name+"_collage";

	//CString temp;
	//if (oneD)
	//	temp.Format("_%d_1D.jpg", num);
	//else
	//	temp.Format("_%d_2D.jpg", num);
	//file += temp;

	char num_char[100];
	_itoa(num, num_char, 10);
	string num_str = num_char;
	string temp;
	if (oneD)
		temp = num_str + "_1D.jpg";
	else
		temp = num_str + "_2D.jpg";
	file += temp;
	cvSaveImage(file.c_str(), pCollage);
	if (oneD)
		GenerateNoBlend(num, TRUE, pCollage);
	else
		GenerateNoBlend(num, FALSE, pCollage);

	pCollage->origin = 0;

	//if (oneD)
	//{
	//	RECT rect = rRect_Collage_1D;
	//	double ratio = ((double)pCollage->width) / ((double)pCollage->height);
	//	double cutLen = rRect_Collage_1D.right - rRect_Collage_1D.left
	//		- ((double)(rRect_Collage_1D.bottom - rRect_Collage_1D.top))*ratio;
	//	if (cutLen>2)
	//	{
	//		rect.left += cutLen / 2;
	//		rect.right -= cutLen / 2;
	//		int dis = rect.right - rect.left;
	//		m_widthRatio = ((double)dis) / ((double)(dis + cutLen));
	//		m_heightRatio = 1.0;
	//	}
	//	else if (cutLen < -1)
	//	{
	//		cutLen /= -ratio;
	//		rect.top += cutLen / 2;
	//		rect.bottom -= cutLen / 2;
	//		int dis = rect.bottom - rect.top;
	//		m_heightRatio = ((double)dis) / ((double)(dis + cutLen));
	//		m_widthRatio = 1.0;
	//	}
	//	GetDlgItem(IDC_STATIC_COLLAGE_1D)->MoveWindow(rect.left, rect.top,
	//		rect.right - rect.left, rect.bottom - rect.top);

	//}
	//else
	//{
	//	RECT rect = rRect_Collage;
	//	double ratio = ((double)pCollage->width) / ((double)pCollage->height);
	//	double cutLen = rRect_Collage.right - rRect_Collage.left
	//		- ((double)(rRect_Collage.bottom - rRect_Collage.top))*ratio;
	//	if (cutLen>2)
	//	{
	//		rect.left += cutLen / 2;
	//		rect.right -= cutLen / 2;
	//	}
	//	else if (cutLen < -1)
	//	{
	//		cutLen /= -ratio;
	//		rect.top += cutLen / 2;
	//		rect.bottom -= cutLen / 2;
	//	}
	//	GetDlgItem(IDC_STATIC_COLLAGE)->MoveWindow(rect.left, rect.top,
	//		rect.right - rect.left, rect.bottom - rect.top);
	//}


	//// adjust the roi rect to the picture client rect;
	//if (oneD)
	//	AdjROIInfo(pCollage->width, pCollage->height, TRUE);
	//else
	//	AdjROIInfo(pCollage->width, pCollage->height, FALSE);

	////	if(m_pVideo!=NULL)
	//cvMirror(pCollage, NULL, 0);

	//if (oneD)
	//{
	//	if (m_Bmp_1D)
	//	{
	//		delete m_Bmp_1D; m_Bmp_1D = NULL;
	//	}
	//	m_Bmp_1D = new CBmp(pCollage);
	//	m_bCollageValid = TRUE;
	//}
	//else
	//{
	//	if (m_Bmp)
	//	{
	//		delete m_Bmp; m_Bmp = NULL;
	//	}
	//	m_Bmp = new CBmp(pCollage);
	//	m_bCollageValid = TRUE;
	//}

	cvReleaseImage(&pCollage);

	//SaveROIInfo(num, oneD);

	delete[]Line;
	delete[]result;
}

IplImage* VideoCollage::MergeImage(std::vector<ROIINFO> vtLine, BOOL oneD)
{
	string dir;
	//if (m_pVideo != NULL)
	//{
	//	int pos = m_strVideoFile.ReverseFind('\\');
	//	dir = m_strVideoFile.Left(pos + 1);
	//	dir += "\\ROI\\";
	//}
	//else
	//	dir = m_strPhotoPath + "ROI\\";
	string file1;
	int num = vtLine.size();
	IplImage** Line = new IplImage*[num];
	for (int i = 0; i<vtLine.size(); i++) //the roi of the keyframe;
	{
		//file1.Format("%d.jpg", vtLine[i].lFrmNo);
		char temp_char[100];
		_itoa(vtLine[i].lFrmNo, temp_char, 10);
		string temp_str = temp_char;
		file1 = temp_str + "_roi.jpg";
		file1 = m_resultfolder+"\\ROI\\" + file1;
		Line[i] = cvLoadImage(file1.c_str(), 1);
	}

	IplImage* pImage = NULL;
	if (oneD)
	{
		pImage = Merge2(Line[0], Line[1]);
		pImage->origin = 0;
		int idx = vtLine[0].index;
		m_pRoiInfo_1D[idx].rect.left = g_curPos.x;
		m_pRoiInfo_1D[idx].rect.top = g_curPos.y;
		m_pRoiInfo_1D[idx].rect.right = m_pRoiInfo_1D[idx].rect.left + Line[0]->width;
		m_pRoiInfo_1D[idx].rect.bottom = m_pRoiInfo_1D[idx].rect.top + Line[0]->height;

		idx = vtLine[1].index;
		m_pRoiInfo_1D[idx].rect.left = m_pRoiInfo_1D[vtLine[0].index].rect.right;
		m_pRoiInfo_1D[idx].rect.top = g_curPos.y;
		m_pRoiInfo_1D[idx].rect.right = m_pRoiInfo_1D[idx].rect.left + (pImage->width - Line[0]->width);
		m_pRoiInfo_1D[idx].rect.bottom = m_pRoiInfo_1D[idx].rect.top + Line[0]->height;


		for (int i = 2; i<vtLine.size(); i++)
		{
			IplImage* tmp = pImage;
			pImage = Merge2(pImage, Line[i]);

			idx = vtLine[i].index;
			m_pRoiInfo_1D[idx].rect.top = g_curPos.y;
			m_pRoiInfo_1D[idx].rect.bottom = m_pRoiInfo_1D[idx].rect.top + Line[0]->height;

			m_pRoiInfo_1D[idx].rect.left = m_pRoiInfo_1D[vtLine[i - 1].index].rect.right;
			m_pRoiInfo_1D[idx].rect.right = m_pRoiInfo_1D[idx].rect.left + (pImage->width - tmp->width);

			cvReleaseImage(&tmp);
		}
	}
	else
	{
		pImage = Merge2(Line[0], Line[1]);
		pImage->origin = 0;
		int idx = vtLine[0].index;
		m_pRoiInfo[idx].rect.left = g_curPos.x;
		m_pRoiInfo[idx].rect.top = g_curPos.y;
		m_pRoiInfo[idx].rect.right = m_pRoiInfo[idx].rect.left + Line[0]->width;
		m_pRoiInfo[idx].rect.bottom = m_pRoiInfo[idx].rect.top + Line[0]->height;

		idx = vtLine[1].index;
		m_pRoiInfo[idx].rect.left = m_pRoiInfo[vtLine[0].index].rect.right;
		m_pRoiInfo[idx].rect.top = g_curPos.y;
		m_pRoiInfo[idx].rect.right = m_pRoiInfo[idx].rect.left + (pImage->width - Line[0]->width);
		m_pRoiInfo[idx].rect.bottom = m_pRoiInfo[idx].rect.top + Line[0]->height;


		for (int i = 2; i<vtLine.size(); i++)
		{
			IplImage* tmp = pImage;
			pImage = Merge2(pImage, Line[i]);

			idx = vtLine[i].index;
			m_pRoiInfo[idx].rect.top = g_curPos.y;
			m_pRoiInfo[idx].rect.bottom = m_pRoiInfo[idx].rect.top + Line[0]->height;

			m_pRoiInfo[idx].rect.left = m_pRoiInfo[vtLine[i - 1].index].rect.right;
			m_pRoiInfo[idx].rect.right = m_pRoiInfo[idx].rect.left + (pImage->width - tmp->width);

			cvReleaseImage(&tmp);
		}
	}

	for (int i = 0; i<num; i++)
	{
		cvReleaseImage(&Line[i]);
	}
	delete[] Line;
	return pImage;
}

IplImage* VideoCollage::Merge2(IplImage* image1, IplImage* image2)
{

	float ratio = image2->height / (float)image1->height;
	int width = image2->width / ratio;
	int height = image1->height;
	IplImage* image2tmp = cvCreateImage(cvSize(width, height), 8, 3);
	cvZero(image2tmp);
	cvResize(image2, image2tmp);

	IplImage* tmp = cvCreateImage(cvSize(width + image1->width, image1->height), 8, 3);
	cvZero(tmp);
	tmp->origin = 0;

	CvRect rect;
	rect.x = 0; rect.y = 0; rect.height = image1->height; rect.width = image1->width - 20;
	cvSetImageROI(image1, rect);
	cvSetImageROI(tmp, rect);
	cvCopy(image1, tmp);
	cvResetImageROI(tmp);
	cvResetImageROI(image1);

	rect.x = 20;; rect.width = image2tmp->width - 20;
	cvSetImageROI(image2tmp, rect);
	rect.x = 20 + image1->width;
	cvSetImageROI(tmp, rect);
	cvCopy(image2tmp, tmp);
	cvResetImageROI(tmp);
	cvResetImageROI(image2tmp);

	BYTE* pImage1 = (BYTE*)image1->imageData;
	BYTE* pImage2 = (BYTE*)image2tmp->imageData;
	BYTE* pResult = (BYTE*)tmp->imageData;

	for (int i = 0; i < image1->height; i++)
	{
		for (int j = image1->width - 21; j < image1->width + 20; j++)
		{
			float x = (image1->width - j) / 12.0;
			float p =/*1-(j-image1->width)*0.025*/ normcdf(x);
			pResult[i*tmp->widthStep + j * 3] = 255;
			pResult[i*tmp->widthStep + j * 3 + 1] = 255;
			pResult[i*tmp->widthStep + j * 3 + 2] = 255;
			if (j < image1->width)
			{
				pResult[i*tmp->widthStep + j * 3] = pImage1[i*image1->widthStep + j * 3] * p/**255*/
					+ pImage2[i*image2tmp->widthStep + (image1->width - j - 1) * 3] * (1 - p)/**255*/;

				pResult[i*tmp->widthStep + j * 3 + 1] = pImage1[i*image1->widthStep + j * 3 + 1] * p
					+ pImage2[i*image2tmp->widthStep + (image1->width - j - 1) * 3 + 1] * (1 - p);
				pResult[i*tmp->widthStep + j * 3 + 2] = pImage1[i*image1->widthStep + j * 3 + 2] * p
					+ pImage2[i*image2tmp->widthStep + (image1->width - j - 1) * 3 + 2] * (1 - p);
			}
			else
			{
				pResult[i*tmp->widthStep + 3 * (j)] = pImage1[i*image1->widthStep + 3 * (2 * image1->width - j - 1)] * (1 - p)/**255*/
					+ pImage2[i*image2tmp->widthStep + 3 * (j - image1->width)] * (p);
				pResult[i*tmp->widthStep + 3 * (j)+1] = pImage1[i*image1->widthStep + 3 * (2 * image1->width - j - 1) + 1] * (1 - p)/**255*/
					+ pImage2[i*image2tmp->widthStep + 3 * (j - image1->width) + 1] * (p);
				pResult[i*tmp->widthStep + 3 * (j)+2] = pImage1[i*image1->widthStep + 3 * (2 * image1->width - j - 1) + 2] * (1 - p)/**255*/
					+ pImage2[i*image2tmp->widthStep + 3 * (j - image1->width) + 2] * (p);
			}
		}
	}

	return tmp;

}

IplImage* VideoCollage::Merge2(IplImage* image1, IplImage* image2, bool bLine)
//when bLine = false, merge in the up-down mode
{
	if (bLine == true)
	{
		return Merge2(image1, image2);
	}
	float ratio = image2->width / (float)image1->width;
	int height = image2->height / ratio;
	int width = image1->width;
	IplImage* image2tmp = cvCreateImage(cvSize(width, height), 8, 3);
	cvZero(image2tmp);
	image2tmp->origin = 0;

	cvResize(image2, image2tmp);

	IplImage* tmp = cvCreateImage(cvSize(width, image1->height + image2tmp->height), 8, 3);
	cvZero(tmp);
	tmp->origin = 0;
	CvRect rect;
	rect.x = 0; rect.y = 0; rect.height = image1->height - 20; rect.width = image1->width;
	cvSetImageROI(image1, rect);
	cvSetImageROI(tmp, rect);
	cvCopy(image1, tmp);
	cvResetImageROI(image1);
	cvResetImageROI(tmp);

	rect.y = 20; rect.height = image2tmp->height - 20;
	cvSetImageROI(image2tmp, rect);

	rect.y = image1->height + 20;
	cvSetImageROI(tmp, rect);
	cvCopy(image2tmp, tmp);
	cvResetImageROI(tmp);
	cvResetImageROI(image2tmp);

	BYTE* pImage1 = (BYTE*)image1->imageData;
	BYTE* pImage2 = (BYTE*)image2tmp->imageData;
	BYTE* pResult = (BYTE*)tmp->imageData;

	for (int i = image1->height - 20; i<image1->height + 20; i++)
	{
		for (int j = 0; j<image1->width; j++)
		{
			float x = (image1->height - i) / 12.0;
			float p = normcdf(x);

			if (i<image1->height)
			{
				pResult[i*tmp->widthStep + j * 3] = pImage1[(i)*image1->widthStep + j * 3] * p
					+ pImage2[(image1->height - i - 1)*image2tmp->widthStep + j * 3] * (1 - p);

				pResult[i*tmp->widthStep + j * 3 + 1] = pImage1[i*image1->widthStep + j * 3 + 1] * p
					+ pImage2[(image1->height - i - 1)*image2tmp->widthStep + j * 3 + 1] * (1 - p);
				pResult[i*tmp->widthStep + j * 3 + 2] = pImage1[i*image1->widthStep + j * 3 + 2] * p
					+ pImage2[(image1->height - i - 1)*image2tmp->widthStep + j * 3 + 2] * (1 - p);
			}
			else
			{
				pResult[i*tmp->widthStep + j * 3] =
					pImage1[(2 * image1->height - i - 1)*image1->widthStep + j * 3] * (1 - p)
					+ pImage2[(i - image1->height)*image2tmp->widthStep + j * 3] * (p);
				pResult[i*tmp->widthStep + j * 3 + 1] =
					pImage1[(2 * image1->height - i - 1)*image1->widthStep + j * 3 + 1] * (1 - p)
					+ pImage2[(i - image1->height)*image2tmp->widthStep + j * 3 + 1] * (p);
				pResult[i*tmp->widthStep + j * 3 + 2] =
					pImage1[(2 * image1->height - i - 1)*image1->widthStep + j * 3 + 2] * (1 - p)
					+ pImage2[(i - image1->height)*image2tmp->widthStep + j * 3 + 2] * (p);
			}
		}
	}
	return tmp;
}

void VideoCollage::GenerateNoBlend(int num, BOOL oneD, IplImage* pCollage)
{
	//CString dir;
	//if (m_pVideo != NULL)
	//{
	//	int pos = m_strVideoFile.ReverseFind('\\');
	//	dir = m_strVideoFile.Left(pos + 1);
	//	dir += "\\ROI\\";
	//}
	//else
	//	dir = m_strPhotoPath + "ROI\\";

	string dir=m_resultfolder+"\\ROI\\";

	//CString file;
	//if (m_pVideo != NULL)
	//{
	//	int pos = m_strVideoFile.ReverseFind('\\');
	//	CString temp = m_strVideoFile.Left(pos + 1);
	//	CString name = m_strVideoFile.Mid(pos + 1, m_strVideoFile.ReverseFind('.') - pos - 1);
	//	temp += name + "_collage\\";
	//	file = temp + name;
	//}
	//else
	//	file = m_strPhotoPath + "collage\\photo";

	//CString temp;
	//if (oneD)
	//	temp.Format("_%d_1D_noblend.jpg", num);
	//else
	//	temp.Format("_%d_2D_noblend.jpg", num);
	//file += temp;

	string file;
    int pos = m_strVideoFile.find_last_of('\\');
	string name = m_strVideoFile.substr(pos + 1, m_strVideoFile.find_last_of('.') - pos - 1);
	dir =m_resultfolder + "\\CollageResult\\";
	_mkdir(dir.c_str());
	file = dir + name+"_collage";

	//CString temp;
	//if (oneD)
	//	temp.Format("_%d_1D.jpg", num);
	//else
	//	temp.Format("_%d_2D.jpg", num);
	//file += temp;

	char num_char[100];
	_itoa(num, num_char, 10);
	string num_str = num_char;
	string temp;
	if (oneD)
		temp = num_str + "_1D_noblend.jpg";
	else
		temp = num_str + "_2D_noblend.jpg";

	file += temp;



	IplImage* edgeImage = cvCloneImage(pCollage);
	cvZero(edgeImage);

	if (oneD)
	{
		m_edgeWidth = (edgeImage->width + edgeImage->height) * 6 / 1000;
		if (m_edgeWidth < 5)
			m_edgeWidth = 5;

		string file1;
		vector<CvRect> picPos;
		for (int i = 0; i<m_nKeyFrameNum; i++) //the roi of the keyframe;
		{
			if (m_pRoiInfo_1D[i].ImptIdx<num)
			{
				int temp = m_pRoiInfo[i].lFrmNo;
				char temp_char[100];
				_itoa(temp, temp_char, 10);
				string temp_str = temp_char;
				file1 =temp_str + (string)"_roi.jpg";
				dir = m_resultfolder+"\\ROI\\";
				file1 = dir + file1;
				IplImage* tempImage = cvLoadImage(file1.c_str(), 1);
				CvRect r;
				r.x = m_pRoiInfo_1D[i].rect.left;
				r.y = m_pRoiInfo_1D[i].rect.top;
				r.width = m_pRoiInfo_1D[i].rect.right - m_pRoiInfo_1D[i].rect.left;
				r.height = m_pRoiInfo_1D[i].rect.bottom - m_pRoiInfo_1D[i].rect.top;
				picPos.push_back(r);
				IplImage* resizedImage = cvCreateImage(cvSize(r.width, r.height), 8, 3);
				cvResize(tempImage, resizedImage);
				cvSetImageROI(edgeImage, r);
				cvCopy(resizedImage, edgeImage);
				cvResetImageROI(edgeImage);
				cvReleaseImage(&tempImage);
				cvReleaseImage(&resizedImage);
			}
			for (int i = 0; i<picPos.size(); i++)
			{
				if (picPos[i].x>0)
					cvLine(edgeImage, cvPoint(picPos[i].x, picPos[i].y), cvPoint(picPos[i].x, picPos[i].y + picPos[i].height), CV_RGB(255, 0, 0), m_edgeWidth);
			}
		}
	}
	else
	{
		m_edgeWidth = (edgeImage->width + edgeImage->height) * 8 / 1000;
		if (m_edgeWidth < 5)
			m_edgeWidth = 5;

		string file1;
		vector<CvRect> picPos;
		for (int i = 0; i<m_nKeyFrameNum; i++) //the roi of the keyframe;
		{
			if (m_pRoiInfo[i].ImptIdx<num)
			{
				int temp = m_pRoiInfo[i].lFrmNo;
				char temp_char[100];
				_itoa(temp, temp_char, 10);
				string temp_str = temp_char;
				file1 =temp_str + (string)"_roi.jpg";
				dir = m_resultfolder+"\\ROI\\";
				file1 = dir + file1;
				IplImage* tempImage = cvLoadImage(file1.c_str(), 1);
				CvRect r;
				r.x = m_pRoiInfo[i].rect.left;
				r.y = m_pRoiInfo[i].rect.top;
				r.width = m_pRoiInfo[i].rect.right - m_pRoiInfo[i].rect.left;
				r.height = m_pRoiInfo[i].rect.bottom - m_pRoiInfo[i].rect.top;
				picPos.push_back(r);
				IplImage* resizedImage = cvCreateImage(cvSize(r.width, r.height), 8, 3);
				cvResize(tempImage, resizedImage);
				cvSetImageROI(edgeImage, r);
				cvCopy(resizedImage, edgeImage);
				cvResetImageROI(edgeImage);
				cvReleaseImage(&tempImage);
				cvReleaseImage(&resizedImage);
			}
		}

		for (int i = 0; i<picPos.size(); i++)
		{
			if (picPos[i].x>0)
				cvLine(edgeImage, cvPoint(picPos[i].x, picPos[i].y), cvPoint(picPos[i].x, picPos[i].y + picPos[i].height), CV_RGB(255, 0, 0), m_edgeWidth);
			if (picPos[i].y>0)
				cvLine(edgeImage, cvPoint(picPos[i].x, picPos[i].y), cvPoint(picPos[i].x + picPos[i].width, picPos[i].y), CV_RGB(255, 0, 0), m_edgeWidth);
		}
	}
	cvSaveImage(file.c_str(), edgeImage);
}

void GetKeyFrames(const wstring & srcFile)
{
	int pos=srcFile.find_last_of('.');
	wstring filefolder=srcFile.substr(0,pos);
	filefolder+=L"_resultfolder";
	MetaDataExtractConfig extractConfig(srcFile);
	extractConfig.ShotDetector=new CDThreshShotDetector;
	extractConfig.vaEngine.AddReceiver(extractConfig.ShotDetector);
	if(_access(ws2s(filefolder).c_str(),0)==-1)// keyframes not exist
	{
		ExtractConfig gblConfig;
		gblConfig.m_SrcFile=srcFile;
		int pos=srcFile.find_last_of('.');
		gblConfig.m_SavedFloder=filefolder;	
		gblConfig.m_Structures=VA_KEYFRAME|VA_SCENE;
		gblConfig.m_Features=0x00;
//		MetaDataExtractConfig extractConfig(srcFile);
		CFrameJSegmentor::Initialize(3,-1,0.6f);
		//create floders
		CreateSavedFloders(gblConfig);
		//config the structure feature
		ConfigStructureFeatureExtract(gblConfig, extractConfig);
		//config the global feature
		ConfigGlobalFeatureExtract(gblConfig, extractConfig);
		////config the region feature
		//ConfigRegionFeatureExtract(gblConfig, extractConfig);
		//config the thumb nail 
		//ConfigThumbnailExtract(gblConfig, extractConfig);
		//the gdi initialize
		CGdiplusLife m_gdiLife;

		//run the engine
		cout<<"\tShot detection"; 
		if( gblConfig.m_Features ) cout<<", Feature extractioin";
		if( gblConfig.m_Regions ) cout<<", Region extraction";
		if( gblConfig.m_GME ) cout<<", GME";
		cout<<" : "<<endl;

		extractConfig.vaEngine.Run(PrintProgress, 100);
		cout<<endl;

		//second round
		ExtractSubshot(gblConfig, extractConfig);
		ExtractScene(gblConfig, extractConfig);
		//ExtractMotionThumbnail(gblConfig, extractConfig);

		//save
		SaveAll(gblConfig, extractConfig);


	}
}

void GetKeyFrames_Select(const wstring & srcFile)
{
	int nKeyframes=MAX_TOPFRAME;
	int pos=srcFile.find_last_of('.');
	wstring filefolder=srcFile.substr(0,pos);
	filefolder+=L"_resultfolder_feature";
	MetaDataExtractConfig extractConfig(srcFile);
	CRGB256HistogramFeatureExtractor rgb256HistogramFeatureExtractor;
	extractConfig.ShotDetector=new CDThreshShotDetector(&extractConfig.keyframeExtractor, false);	
	extractConfig.pRGBFrameFeatureExtractor = new CFrameFeatureExtractor(&rgb256HistogramFeatureExtractor); // register into frame feature extractor which is a receiver
	(*extractConfig.ShotDetector).AddReceiver(extractConfig.pRGBFrameFeatureExtractor); // it is just for shot, not each frame. What a shame.
	extractConfig.vaEngine.AddReceiver(extractConfig.ShotDetector);
	if(_access(ws2s(filefolder+L"\\Selected").c_str(),0)==-1)// Selectframes not exist
	{
		ExtractConfig gblConfig;
		gblConfig.m_SrcFile=srcFile;
		int pos=srcFile.find_last_of('.');
		gblConfig.m_SavedFloder=filefolder;	
		gblConfig.m_Structures=VA_KEYFRAME|VA_SCENE;
		gblConfig.m_Features=0x00;
//		MetaDataExtractConfig extractConfig(srcFile);
		CFrameJSegmentor::Initialize(3,-1,0.6f);
		//create floders
		CreateSavedFloders(gblConfig);
		//config the structure feature
		ConfigStructureFeatureExtract(gblConfig, extractConfig);
		//config the global feature
		ConfigGlobalFeatureExtract(gblConfig, extractConfig);
		////config the region feature
		//ConfigRegionFeatureExtract(gblConfig, extractConfig);
		//config the thumb nail 
		//ConfigThumbnailExtract(gblConfig, extractConfig);
		//the gdi initialize
		CGdiplusLife m_gdiLife;

		//run the engine
		cout<<"\tShot detection"; 
		if( gblConfig.m_Features ) cout<<", Feature extractioin";
		if( gblConfig.m_Regions ) cout<<", Region extraction";
		if( gblConfig.m_GME ) cout<<", GME";
		cout<<" : "<<endl;

		extractConfig.vaEngine.Run(PrintProgress, 100);
		cout<<endl;

		////second round
		ExtractSubshot(gblConfig, extractConfig);
		ExtractScene(gblConfig, extractConfig);
		////ExtractMotionThumbnail(gblConfig, extractConfig);

		//save
		SaveAll(gblConfig, extractConfig);


		const int nFrames = extractConfig.vaEngine.FrameNum();
		const CVideoSegmentList *pShots = &extractConfig.ShotDetector->GetShots();
		const int nShots = pShots->GetSegmentNum();

	    const CDataSet &differenceFeature = extractConfig.keyframeExtractor.m_DifferenceExtractor.GetData(); // frame difference
	    const CDataSet &skinFeature = extractConfig.keyframeExtractor.m_SkinExtractor.GetData();
	    const CDataSet &entropyFeature = extractConfig.keyframeExtractor.m_EntropyExtractor.GetData();

			// statistics:
		vector<double> duration(nShots), face(nShots), skin(nShots), motion(nShots), score(nShots), entropy(nShots);
		vector<int> keyframe(nShots), bestSubshot(nShots), mainShot(nShots);
		double mduration = 10e8, Mduration = 0, mskin = 10e8, Mskin = 0, mmotion = 10e8, Mmotion = 0, mentropy = 10e8, Mentropy = 0;
		for (int i = 0; i < nShots; ++i) {
		duration[i] = (*pShots)[i].EndFrameId() - (*pShots)[i].BeginFrameId();
		mduration = min(duration[i], mduration);
		Mduration = max(duration[i], Mduration);
		face[i] = skinFeature.GetSample(i)[1];
		skin[i] = skinFeature.GetSample(i)[0];
		mskin = min(skin[i], mskin);
		Mskin = max(skin[i], Mskin);
		motion[i] = differenceFeature.GetSample(i)[0];
		mmotion = min(motion[i], mmotion);
		Mmotion = max(motion[i], Mmotion);
		entropy[i] = entropyFeature.GetSample(i)[0];
		mentropy = min(entropy[i], mentropy);
		Mentropy = max(entropy[i], Mentropy);
		keyframe[i] = (*pShots)[i].GetKeyframe(0)->Id();
	}
		PRE_WEIGHT;
	for (int i = 0; i < nShots; ++i) { // merge the scores
		score[i] = WEIGHT(face[i], skin[i], duration[i], motion[i], entropy[i]);
	}

	const CVideoSegmentList *pScenes = pShots;
	int nScenes = nShots;
	int MaxNumOfScenes = max(MAX_SCENE, nFrames / (20 * 60));
	if (nShots > MaxNumOfScenes) {
		CEntropyOfflineSceneDetector sceneDetector((*pShots), extractConfig.pRGBFrameFeatureExtractor->GetData(), MaxNumOfScenes + 1);
		pScenes = &sceneDetector.DetectSegmentOffline(); // in Children
		nScenes = pScenes->GetSegmentNum();
		// chose best shot for each scene
		double MDuration = 0;
		for (int shot = 0, scene = 0; scene < nScenes; ++scene) { // for each scene
			int nSubshots = pScenes->GetSegment(scene)->GetChildrenNum();
			double m = 0;
			int bestShot;
			double duration_new = 0;
			for (int subshot = 0; subshot < nSubshots; ++subshot, ++shot) { // for each shot in scene
				duration_new += duration[shot];
				if (m < score[shot]) {
					m = score[shot];
					bestShot = shot;
					bestSubshot[scene] = subshot;
				}
			}
			/*
			int originKey = (*pScenes)[scene].GetKeyframe(0)->Id();
			if (keyframe[bestShot] != originKey) { // not the shot with the highest score
				// replace keyframe
				(*pScenes)[scene].RemoveKeyframe(originKey);
				(*pScenes)[scene].AddKeyframe((*pShots)[shot].GetKeyframe(0));
			}
			*/
			// update information
			mainShot[scene] = bestShot;
			duration[scene] = duration_new;
			MDuration = max(MDuration, duration_new);
			face[scene] = face[bestShot];
			skin[scene] = skin[bestShot];
			motion[scene] = motion[bestShot];
			entropy[scene] = entropy[bestShot];
		}
		// update score to scene
		PRE_WEIGHT
		for (int i = 0; i < nScenes; ++i) {
			score[i] = WEIGHT(face[i], skin[i],  duration[i], motion[i], entropy[i]);
		}
	} else {
		for (int i = 0; i < nShots; ++i)
			mainShot[i] = i;
	}

	// distances of all scenes (similarity metrics)
	const IDataSet &data = extractConfig.pRGBFrameFeatureExtractor->GetData();
	double *dist = new double[nScenes * nScenes];
	for (int i = 0; i < nScenes; ++i) {
		dist[i * nScenes + i] = 0;
		for (int j = i + 1; j < nScenes; ++j) {
			double d = 0;
			int n = data.GetSample(mainShot[i]).Size();
			for (int k = 0; k < n; ++k) {
				d += data.GetSample(mainShot[i])[k] * data.GetSample(mainShot[j])[k];
			}
			dist[i * nScenes + j] = dist[j * nScenes + i] =  1 - d * d;
		}
	}

	// 3. Save keyframes for all scenes
	wstring targetFileName=filefolder;
////	CreateDirectory(targetFileName, NULL);
//	_wsystem((wstring(L"del ") + targetFileName + L"\\*.jpg /Q").c_str());
//	std::cout << '\n' << nScenes << " scenes detected." << endl;
//	for (int i = 0; i < nScenes; ++i) {
//		wchar_t outFile[MAX_PATH];
//		wsprintf(outFile, L"%s\\%i.jpg", targetFileName, (*pShots).GetSegment(mainShot[i])->GetKeyframe(0)->Id());
//		CRgbImage image(*pShots->GetSegment(mainShot[i])->GetKeyframe(0)->GetImage());
//		//EnhanceImage(image);
//		cout<<outFile<<endl;
//		image.Save(outFile);
//	}


	// 5. write keyframe candidate list to file, one frame per shot
	ofstream outFile((wstring(targetFileName) + L"\\Keyframe\\list.txt").c_str());
	outFile << nScenes << endl;
	// id	begintime	score	skin	face	duration	motion	entropy
	for (int i = 0; i < nScenes; ++i)
		outFile << keyframe[mainShot[i]] << '\t' << (*pShots)[mainShot[i]].BeginTime() / 10000 << '\t' << skin[i] << '\t' << face[i] << '\t' << duration[i] << '\t' << motion[i] << '\t' << entropy[i] << endl;
	outFile << nFrames << endl;
	for (int i = 0; i < nScenes; ++i) {
		for (int j = i; j < nScenes; ++j)
			outFile << dist[i * nScenes + j] << '\t';
		outFile << endl;
	}
	outFile.close();
	
	// 6. algorithm
	// local normalization
	vector<double> newscore(nScenes);
	int range = nFrames / (nKeyframes * 2);
	for (int i = 0; i < nScenes; ++i) {
		double Mscore = score[i];
		for (int j = i - 1; j >= 0 && keyframe[j] >= range; --j)
			Mscore = max(Mscore, score[j]);
		for (int j = i + 1; j < nScenes && keyframe[j] <= range; ++j)
			Mscore = max(Mscore, score[j]);
		newscore[i] = score[i] / Mscore;
	}
	// find & save top nKeyframes
	CreateDirectory((wstring(targetFileName) + L"\\Selected").c_str(), NULL);
	nKeyframes = min(nKeyframes, nScenes);
	std::cout << "N: " << nKeyframes << endl;
	vector<int> selectedFrames;
	string selectlist_file=ws2s(targetFileName)+"\\Selected\\SelectedList.txt";
	ofstream out_select(selectlist_file);
	vector<int> selectlist;	

	for (int n = 0; n < nKeyframes; ++n) {
		double m = -1;
		int mi = 0;
		for (int i = 0; i < nScenes; ++i) {
			if (m < newscore[i] && std::find(selectedFrames.begin(), selectedFrames.end(), i) == selectedFrames.end()) {
				m = newscore[i];
				mi = i;
			}
		}
		std::cout << "m = " << m << ", mi = " << mi << endl;
		if (m > 0) {
			//wchar_t outfile[MAX_PATH];
			//wsprintf(outfile, L"%s\\selected\\%d.%d.jpg", targetFileName, keyframe[mi], n + 1);
			char temp1_char[100],temp2_char[100];
			_itoa(keyframe[mi], temp1_char, 10);
			_itoa(n+1,temp2_char,10);
			selectlist.push_back(keyframe[mi]);
			//wstring outfile=targetFileName+L"\\Selected\\"+s2ws(temp1_char)+L"."+s2ws(temp2_char)+L".jpg";			
			//string outfile=ws2s(targetFileName)+"\\Selected\\"+temp1_char+"."+temp2_char+".jpg";
			string outfile=ws2s(targetFileName)+"\\Selected\\"+temp1_char+".jpg";
			CRgbImage image(*(*pShots).GetSegment(mainShot[mi])->GetKeyframe(0)->GetImage());
			EnhanceImage(image);
			CUser use;
			
			image.Save(use.StringToWchar(outfile));
			selectedFrames.push_back(mi);
		}
		// neighbors are supressed in each selection
		for (int i = 0; i < nScenes; ++i)
			newscore[i] *= dist[mi * nScenes + i] * .4 + .6;
	}	
	sort(selectlist.begin(),selectlist.end());
	for(int i=0;i<selectlist.size();i++)
	{
		out_select<<selectlist.at(i)<<endl;
	}

	out_select.close();
	
	delete []dist;
	}

}

void SaveAll(const ExtractConfig & config, MetaDataExtractConfig & extConfig)
{
       wstring srcFile = config.m_SrcFile;
       //save video information
       VideoInfoSave(srcFile, config.m_SavedFloder, extConfig);
       //save structures
       if( config.m_Structures )
       {
            wstring dirName = config.m_SavedFloder+L"\\Structure";
            if( config.m_Structures & VA_SCENE )
            {      
                const CVideoSegmentList & scenes = extConfig.SceneDetector->GetScenes();
                wstring savedTextFile = dirName + L"\\Shot.txt";                       
                wstring savedXMLFile = dirName + L"\\Structure.xml";
                //save the shot result to txt file
                SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
                //save the scene result to xml file
                SaveScenesToXML(scenes, savedXMLFile.c_str());
             } else if( config.m_Structures & VA_SUBSHOT ) {
                wstring savedTextFile = dirName + L"\\Shot.txt";
                wstring savedXMLFile = dirName + L"\\Structure.xml";
                //save the shot result to txt file
                SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
                //save the shot result to xml file
                SaveShotsToXML(dynamic_cast<CSabOnlineSubshotDetector*>(extConfig.SubshotDetector)->GetCompleteShots(), savedXMLFile.c_str());
            }  else if( config.m_Structures & VA_SHOT ) {
                wstring savedTextFile = dirName + L"\\Shot.txt";
                wstring savedXMLFile = dirName + L"\\Structure.xml";
                //save the shot result to txt file
                SaveShotsToText(extConfig.ShotDetector->GetShots(), savedTextFile.c_str());
                //save the shot result to xml file
                SaveShotsToXML(extConfig.ShotDetector->GetShots(), savedXMLFile.c_str());
            }
        }

        //save global and region feature
        size_t size1 = extConfig.FrameFeatureHelpers.size();
        size_t size2 = extConfig.RegionFeatureHelpers.size();
        wstring dirName = config.m_SavedFloder+L"\\Feature";

        //save the GME result
        //if( config.m_GME )
        //    (dynamic_cast<CAffineGlobalMotionEstimation*>(extConfig.AffineGMEExtractor))->GetData().Save(wstring(dirName+L"\\GME.txt").c_str());

        //save global feature
        for( size_t i = 0; i < size1; ++i )
        {
              wstring name = dirName + L"\\" + extConfig.FrameFeatureHelpers[i]->m_SavedName;
              extConfig.FrameFeatureExtractors[i]->GetData().Save(name.c_str()); 
        }

        //save region feature
        for( size_t i = 0; i < size2; ++i )
         {
               wstring name = dirName + L"\\" + extConfig.RegionFeatureHelpers[i]->m_SavedName;
               extConfig.RegionFeatureExtractors[i]->GetData().Save(name.c_str()); 
         }

		//save video size
		wstring sizefile=config.m_SavedFloder+L"\\Structure\\size.txt";
		ofstream out_size(sizefile);
		out_size<<extConfig.vaEngine.FrameHeight();
		out_size<<" ";
		out_size<<extConfig.vaEngine.FrameWidth();
		out_size.close();
}

bool  CreateSavedFloders(const ExtractConfig & config)
{
         //make a saved floder for the file
         if( _wmkdir(config.m_SavedFloder.c_str()) == -1 )
         {
             if( errno != EEXIST )
             {
                  wcout<<endl<<"Create saving floder for video "<<config.m_SrcFile<<" failed!"<<endl;
                  return false;
             }
         }

         //key frame saved floder
         if( config.m_Structures & VA_KEYFRAME )
         {
             //create key frame saving dir
             wstring keyframeDir = config.m_SavedFloder+L"\\Keyframe";
             if( _wmkdir(keyframeDir.c_str()) == -1 )
             {
                 if( errno != EEXIST )
                 {
                     wcout<<endl<<"Create key frame saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                     return false;
                 }
             }
         }

         //structrue floder
         if( config.m_Structures )
         {
             //create the "Structure" floder
             wstring dirName = config.m_SavedFloder+L"\\Structure";
             if( _wmkdir(dirName.c_str()) == -1 )
             {
                 if( errno != EEXIST )
                 {
                      wcout<<endl<<"Create Structure saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                      return false;
                 }
             }
         }

         //create the feature saved floder
         if( config.m_Features || config.m_Regions || config.m_GME )
         {
             wstring dirName = config.m_SavedFloder+L"\\Feature";
             if( _wmkdir(dirName.c_str()) == -1 )
             {
                 if( errno != EEXIST )
                 {
                     wcout<<endl<<"Create feature saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                     return false;
                 }
             }
         }

         //create the region map saved floder
         if( config.m_Regions & VA_REGIONMAP )
         {
             //create region map saving dir
             wstring regionDir = config.m_SavedFloder+L"\\RegionMap";
             if( _wmkdir(regionDir.c_str()) == -1 )
             {
                 if( errno != EEXIST )
                 {
                     wcout<<endl<<"Create region map saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                     return false;
                 }
              }
         }

         //create static thumbnail saved floder
         if( config.m_Thumbnail & VA_STATIC )
         {
             wstring dirName = config.m_SavedFloder + L"\\StaticThumb";
             if( _wmkdir(dirName.c_str()) == -1 )
             {
                 if( errno != EEXIST )
                 {
                      wcout<<endl<<"Create Static thumb nail saving dir for video "<<config.m_SrcFile<<" failed!"<<endl;
                      return false;
                 }
             }
         }

         //create motion thumbnail saved floder
         if( config.m_Thumbnail & VA_MOTION )
         { 
             //make a saving dir for the file
             wstring dirName = config.m_SavedFloder+L"\\MotionThumb";
             if( _wmkdir(dirName.c_str()) == -1 )
             {
                if( errno != EEXIST )
                {
                    wcout<<endl<<"Create motion thumb nail saving dir for video failed!"<<endl;
                    return false;
                 }
             }
         }
         return true;
}

int main()	
{	
	string videopath;
	int roi_num;
	int video_height,video_width;
	BOOL is_select=FALSE;
	cout<<"Enter the Video Path"<<endl;
	cin>>videopath;
	cout<<"Enter the ROI number"<<endl;
	cin>>roi_num;
	cout<<"Using Feature to select top frames(1/0)"<<endl;
	cin>>is_select;
	//videopath = "D:\\Test\\Video\\55.wmv";
	//roi_num = 20;
	if(is_select==TRUE)
		GetKeyFrames_Select(s2ws(videopath));
	else
		GetKeyFrames(s2ws(videopath));
	VideoCollage(videopath, roi_num, is_select);
}