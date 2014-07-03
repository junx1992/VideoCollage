#include<vector>
#include<map>
//#include<cv\cv.h>
#include "imagehist.h"
#include "imagefeature.h"
#include <opencv2/opencv.hpp>
using namespace std;
typedef struct _SUBSHOTINFO
{
	long lShotID;
	float fImport;
	double dStart;
	double dEnd;

} SUBSHOTINFO;

typedef struct _KEYFRAMEINFO
{
	long lFrameNum;
	SUBSHOTINFO SubShot;
//	CRect rect;
} KEYFRAMEINFO;

typedef struct _ROIINFO
{
	int index;
	long lFrmNo;
	RECT rect;
	float fImport;
	int ImptIdx;
} ROIINFO;

class VideoCollage
{
public:
	VideoCollage(string videopath, int rio_num, BOOL isselect);
	void Showllage(int num);
	int ReadKeyFrameIndex(void);
	void GenerateCollage(int num, BOOL oneD = FALSE);
	ROIINFO* m_pRoiInfo;
	ROIINFO* m_pRoiInfo_1D;
	void GenerateNoBlend(int num, BOOL oneD, IplImage* pCollage);
	IplImage* MergeImage(vector<ROIINFO> vtLine, BOOL oneD = FALSE);
	IplImage* Merge2(IplImage* image1, IplImage* image2);
	IplImage* Merge2(IplImage* image1, IplImage* image2, bool bLine);

private:
	string m_strVideoFile;
	string m_resultfolder;
	int _rionum;

	int m_nKeyFrameNum;

	void Loadprofile();
	void GenerateNoBlend();
	void LoadArrange();
	void ReadVideoInfo();

	int leastImageNum, maxImageNum, oriMaxImageNum;
	map<int, vector<int>> m_picArrange;
	IplImage* m_keyframeImage;
	int h_skip;
	int m_keyframeWidth;
	int m_keyframeHeight;
	int m_edgeWidth;
	BOOL m_select;


	

	


};