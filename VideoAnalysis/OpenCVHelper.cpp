#include "Stdafx.h"
#include "OpenCVHelper.h"
#include <opencv\cv.h>

namespace VideoAnalysisHelper
{
    typedef IplImage*  (*pCvCreateImage) ( CvSize size, int depth, int channels );
    typedef void (*pcvReleaseImage)( IplImage** image );
    typedef void (*pCvSetZero)( CvArr* arr );
    typedef CvSize (*pCvGetSize)( const CvArr* arr );
    typedef unsigned char* (*pCvPtr2D)( const CvArr* arr, int idx0, int idx1, int* type );
    typedef CvMat* (*pCvCreateMat)( int rows, int cols, int type );
    typedef void (*pCvReleaseMat)( CvMat** mat );
    typedef double (*pcvGetReal1D)( const CvArr* arr, int idx0 );
    typedef void (*pCvSetReal1D)( CvArr* arr, int idx0, double value );
    typedef void (*pCvCvtColor)( const CvArr* src, CvArr* dst, int code );
    typedef void (*pCvSmooth)( const CvArr* src, CvArr* dst, int smoothtype, int param1, int param2, double param3, double param4 );
    typedef void (*pCvWarpAffine)( const CvArr* src, CvArr* dst, const CvMat* map_matrix, int flags, CvScalar fillval);

    ///the open cv wrapper 
    class COpenCVHelperImp
    {
    public:
        static COpenCVHelperImp * GetInstance()
        {
                 static COpenCVHelperImp openCVHelper;
                 return &openCVHelper;
        }
        //cxcore100.dll
        //create an image
        static  IplImage*  cvCreateImage( CvSize size, int depth, int channels );
        //release an image
        static void cvReleaseImage( IplImage** image );
        //clears all the array elements (sets them to 0) 
        static void cvSetZero( CvArr* arr );
        //get the matrix's or image ROI's size
        static CvSize cvGetSize( const CvArr* arr );
        //return the pointer of some special array
        static unsigned char* cvPtr2D( const CvArr* arr, int idx0, int idx1, int* type = NULL );
        //create matrix
        static CvMat* cvCreateMat( int rows, int cols, int type );
        //release Mat
        static void cvReleaseMat( CvMat** mat );
        //return the required item of single channel array
        static double cvGetReal1D( const CvArr* arr, int idx0 );
        //change the value of the required arrray's item
        static void cvSetReal1D( CvArr* arr, int idx0, double value );

        //cv100.dll
        //color space changement
        static void cvCvtColor( const CvArr* src, CvArr* dst, int code );
        //picture smooth
        static void cvSmooth( const CvArr* src, CvArr* dst,
                              int smoothtype=CV_GAUSSIAN,
                              int param1=3, int param2=0, double param3=0, double param4=0 );
        //do pictrue WarpAffine
        static void cvWarpAffine( const CvArr* src, CvArr* dst, const CvMat* map_matrix,
                                  int flags=CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, 
                                  CvScalar fillval=cvScalarAll(0) );

        ~COpenCVHelperImp();
    private:
         COpenCVHelperImp();
         COpenCVHelperImp(const COpenCVHelperImp &);
         COpenCVHelperImp & operator = (const COpenCVHelperImp &);


         static HINSTANCE  hinstLib_CXCore;
         static HINSTANCE  hinstLib_CV;
    };

    HINSTANCE COpenCVHelperImp::hinstLib_CXCore = NULL; 
    HINSTANCE COpenCVHelperImp::hinstLib_CV = NULL;

    COpenCVHelperImp * m_Imp = COpenCVHelperImp::GetInstance();
    IplImage*  COpenCVHelper::cvCreateImage( CvSize size, int depth, int channels )
    {
           return m_Imp->cvCreateImage(size, depth, channels);
    }
    
    void COpenCVHelper::cvReleaseImage( IplImage** image )
    {
           m_Imp->cvReleaseImage(image);
    }
    void COpenCVHelper::cvSetZero( CvArr* arr )
    {
           m_Imp->cvSetZero(arr);
    }
    CvSize COpenCVHelper::cvGetSize( const CvArr* arr )
    {
           return m_Imp->cvGetSize(arr);
    }
    unsigned char* COpenCVHelper::cvPtr2D( const CvArr* arr, int idx0, int idx1, int* type )
    {
           return m_Imp->cvPtr2D(arr, idx0, idx1, type);
    }
    CvMat* COpenCVHelper::cvCreateMat( int rows, int cols, int type )
    {
           return m_Imp->cvCreateMat(rows, cols, type);
    }
    void COpenCVHelper::cvReleaseMat( CvMat** mat )
    {
           m_Imp->cvReleaseMat(mat);
    }
    double COpenCVHelper::cvGetReal1D( const CvArr* arr, int idx0 )
    {
           return  m_Imp->cvGetReal1D(arr, idx0);
    }
    void COpenCVHelper::cvSetReal1D( CvArr* arr, int idx0, double value )
    {
           m_Imp->cvSetReal1D(arr, idx0, value);
    }
    void COpenCVHelper::cvCvtColor( const CvArr* src, CvArr* dst, int code )
    {
          m_Imp->cvCvtColor(src, dst, code);
    }
    void COpenCVHelper::cvSmooth( const CvArr* src, CvArr* dst, int smoothtype, 
                                 int param1, int param2, double param3, double param4 )
    {
           m_Imp->cvSmooth(src, dst, smoothtype, param1, param2, param3, param4);
    }
    void COpenCVHelper::cvWarpAffine( const CvArr* src, CvArr* dst, const CvMat* map_matrix, int flags, CvScalar fillval)
    {
           m_Imp->cvWarpAffine(src, dst, map_matrix, flags, fillval);
    }

    COpenCVHelperImp::COpenCVHelperImp()
    {
           hinstLib_CXCore = LoadLibrary(L"cxcore100.dll");
           hinstLib_CV = LoadLibrary(L"cv100.dll");
    }

    COpenCVHelperImp::~COpenCVHelperImp()
    {
           FreeLibrary(hinstLib_CXCore);
           FreeLibrary(hinstLib_CV);
    }

    //the open cv wrapper 
    IplImage*  COpenCVHelperImp::cvCreateImage( CvSize size, int depth, int channels )
    {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pCvCreateImage ProcCreateImage = NULL; 
            IplImage * tmpImage = NULL;
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcCreateImage = (pCvCreateImage) GetProcAddress(hinstLib_CXCore,  "cvCreateImage");
                if ( ProcCreateImage != NULL ) 
                     tmpImage = ProcCreateImage(size, depth, channels);
            }

            //FreeLibrary(hinstLib); 
            return tmpImage;
    }

    void COpenCVHelperImp::cvReleaseImage( IplImage** image )
    {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pcvReleaseImage ProcReleaseImage = NULL; 
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcReleaseImage = (pcvReleaseImage) GetProcAddress(hinstLib_CXCore,  "cvReleaseImage");
                if ( ProcReleaseImage != NULL ) 
                     ProcReleaseImage(image);
            }

            //FreeLibrary(hinstLib); 
    }

    void COpenCVHelperImp::cvSetZero( CvArr* arr )
    {
            // Get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pCvSetZero ProcSetZero = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcSetZero = (pCvSetZero) GetProcAddress(hinstLib_CXCore,  "cvSetZero");
                if ( ProcSetZero != NULL ) 
                     ProcSetZero(arr);
            }

            //FreeLibrary(hinstLib); 
    }

    CvSize COpenCVHelperImp::cvGetSize( const CvArr* arr )
    {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pCvGetSize ProcCetSize = NULL; 
            CvSize  tmpSize = {0};
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcCetSize = (pCvGetSize) GetProcAddress(hinstLib_CXCore,  "cvGetSize");
                if ( ProcCetSize != NULL ) 
                     tmpSize = ProcCetSize(arr);
            }

            //FreeLibrary(hinstLib); 
            return tmpSize;
    }

    unsigned char* COpenCVHelperImp::cvPtr2D( const CvArr* arr, int idx0, int idx1, int* type )
    {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pCvPtr2D ProcPtr2D = NULL; 
            unsigned char * tmpPtr = NULL;
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcPtr2D = (pCvPtr2D) GetProcAddress(hinstLib_CXCore,  "cvPtr2D");
                if ( ProcPtr2D != NULL ) 
                     tmpPtr = ProcPtr2D(arr, idx0, idx1, type);
            }

            //FreeLibrary(hinstLib); 
            return tmpPtr;    
    }

     CvMat* COpenCVHelperImp::cvCreateMat( int rows, int cols, int type )
     {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CXCore); 
       
            pCvCreateMat ProcCreateMat = NULL; 
            CvMat*  tmpCvMat = NULL;
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcCreateMat = (pCvCreateMat) GetProcAddress(hinstLib_CXCore,  "cvCreateMat");
                if ( ProcCreateMat != NULL ) 
                     tmpCvMat = ProcCreateMat(rows, cols, type);
            }

            //FreeLibrary(hinstLib); 
            return tmpCvMat;         
     }
     
     void COpenCVHelperImp::cvReleaseMat( CvMat** mat )
     {
            pCvReleaseMat ProcReleaseMat = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcReleaseMat = (pCvReleaseMat) GetProcAddress(hinstLib_CXCore,  "cvReleaseMat");
                if ( ProcReleaseMat != NULL ) 
                     ProcReleaseMat(mat);
            }
     }

     double COpenCVHelperImp::cvGetReal1D( const CvArr* arr, int idx0 )
     {
            pcvGetReal1D ProcGetReal1D = NULL; 
            double tmpDbl = 0.0;
            
            //if the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcGetReal1D = (pcvGetReal1D) GetProcAddress(hinstLib_CXCore,  "cvGetReal1D");
                if ( ProcGetReal1D != NULL ) 
                     tmpDbl = ProcGetReal1D(arr, idx0);
            }

            return tmpDbl; 
     }

    
    void COpenCVHelperImp::cvSetReal1D( CvArr* arr, int idx0, double value )
    {
            pCvSetReal1D ProcSetReal1D = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CXCore != NULL ) 
            {    
                ProcSetReal1D = (pCvSetReal1D) GetProcAddress(hinstLib_CXCore,  "cvSetReal1D");
                if ( ProcSetReal1D != NULL ) 
                     ProcSetReal1D(arr, idx0, value);
            }
    }

     //cv100.dll
     void COpenCVHelperImp::cvCvtColor( const CvArr* src, CvArr* dst, int code )
     {
            //get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CV); 
       
            pCvCvtColor ProcCvtColor = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CV != NULL ) 
            {    
                ProcCvtColor = (pCvCvtColor) GetProcAddress(hinstLib_CV,  "cvCvtColor");
                if ( ProcCvtColor != NULL ) 
                     ProcCvtColor(src, dst, code);
            }

            //FreeLibrary(hinstLib); 
     }
     
     void COpenCVHelperImp::cvSmooth( const CvArr* src, CvArr* dst, int smoothtype,
                                      int param1, int param2, double param3, double param4 )
     {
            // Get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CV); 
       
            pCvSmooth ProcSmooth = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CV != NULL ) 
            {    
                ProcSmooth = (pCvSmooth) GetProcAddress(hinstLib_CV,  "cvSmooth");
                if ( ProcSmooth != NULL ) 
                     ProcSmooth(src, dst, smoothtype, param1, param2, param3, param4);
            }

            //FreeLibrary(hinstLib); 
     }
     
     void COpenCVHelperImp::cvWarpAffine( const CvArr* src, CvArr* dst, const CvMat* map_matrix, int flags, CvScalar fillval)
     {
            //Get a handle to the DLL module.
            //HINSTANCE  hinstLib = LoadLibrary(dllName_CV); 
       
            pCvWarpAffine ProcWarpAffine = NULL; 
            
            // If the handle is valid, try to get the function address.
            if( hinstLib_CV != NULL ) 
            {    
                ProcWarpAffine = (pCvWarpAffine) GetProcAddress(hinstLib_CV,  "cvWarpAffine");
                if ( ProcWarpAffine != NULL ) 
                     ProcWarpAffine(src, dst, map_matrix, flags, fillval);
            }

            //FreeLibrary(hinstLib); 
     }
 }