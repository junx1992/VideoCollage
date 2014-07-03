/*****************************************************************************\

Microsoft Research Asia
Copyright (c) 2002 Microsoft Corporation

Module Name:

  FaceDetectorDLL.h: DLL interface for the face detector.

Notes:

History:

  Created on 1/30/2003 by Lei Zhang, i-lzhang@microsoft.com
  Modified mm/dd/yyyy by email-name

\*****************************************************************************/

#ifndef _FACEDETECTORDLL_H_
#define _FACEDETECTORDLL_H_

#ifdef FACEDETECTORDLL_EXPORTS
#define FACEDETECTORDLL_API __declspec(dllexport)
#else
#define FACEDETECTORDLL_API __declspec(dllimport)
#endif

struct FaceRect
{
	RECT	rBox;
	float   fConfidence;
	int		nRotationDegree;
};

// This class is exported from the FaceDetectorDLL.dll
class FACEDETECTORDLL_API CFaceDetectorDLL 
{
	void *m_pFaceDetector;
	void Destroy();

public:
	CFaceDetectorDLL();
	~CFaceDetectorDLL();

	void Init();

	HRESULT DetectFace(	int iWidth,
						int iHeight,
						BYTE * pbImage,
						int iStride,		// default: 0, (for four bytes align)
						int iBytesPerPixels,
						int iColorSequence,	// 0 for RGB, 1 for BGR
						BOOL bTopDown,		// TRUE for topdown, FALSE otherwise
						LPRECT pRect,		// NULL for entire image
						int * pCount,		// count of detected faces
						FaceRect ** ppFaceRect);// pointer of FaceRect array will be returned here

	void FreeFaceRect(FaceRect *pFaceRect);
};

#endif // _FACEDETECTORDLL_H_