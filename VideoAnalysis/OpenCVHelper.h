/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the helper class for opencv library function calling

Notes:
  

History:
  Created on 06/26/2007 by v-huami@microsoft.com
\******************************************************************************/

#pragma once
#include <opencv\cv.h>

namespace VideoAnalysisHelper
{
    ///the implemention class of opencv helper
    class COpenCVHelperImp;

    ///the open cv wrapper 
    class COpenCVHelper
    {
    public:
        //load from cxcore100.dll
        ///create an image
        static  IplImage*  cvCreateImage( CvSize size, int depth, int channels );
        ///release an image
        static void cvReleaseImage( IplImage** image );
        ///clears all the array elements (sets them to 0) 
        static void cvSetZero( CvArr* arr );
        ///get the matrix's or image ROI's size
        static CvSize cvGetSize( const CvArr* arr );
        ///return the pointer of some special array
        static unsigned char* cvPtr2D( const CvArr* arr, int idx0, int idx1, int* type = NULL );
        ///create matrix
        static CvMat* cvCreateMat( int rows, int cols, int type );
        ///release Mat
        static void cvReleaseMat( CvMat** mat );
        ///return the required item of single channel array
        static double cvGetReal1D( const CvArr* arr, int idx0 );
        ///change the value of the required arrray's item
        static void cvSetReal1D( CvArr* arr, int idx0, double value );


        //load from cv100.dll
        ///color space changement
        static void cvCvtColor( const CvArr* src, CvArr* dst, int code );
        ///picture smooth
        static void cvSmooth( const CvArr* src, CvArr* dst,
                              int smoothtype=CV_GAUSSIAN,
                              int param1=3, int param2=0, double param3=0, double param4=0 );
        ///do pictrue WarpAffine
        static void cvWarpAffine( const CvArr* src, CvArr* dst, const CvMat* map_matrix,
                                  int flags=CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, CvScalar fillval=cvScalarAll(0) );

    private:
         static COpenCVHelperImp * m_Imp;
    };
}