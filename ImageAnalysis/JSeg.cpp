#include "StdAfx.h"
#include "JSeg.h"
#include "GrayImage.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <crtdbg.h>
//#include "jseg/segment.h"
//#include "jseg/ioutil.h"
//#include "jseg/imgutil.h"
//#include "jseg/mathutil.h"
//#include "jseg/quan.h"
//#include "jseg/memutil.h"

extern "C" int segment(unsigned char *rmap,unsigned char *cmap,int N,int nt,int ny,int nx,
    unsigned char *RGB,char *outfname,char *exten,int type,int dim,int NSCALEi,
    float displayintensity,int verbose,int tt);
extern "C" int merge1(unsigned char *rmap,unsigned char *cmap,int N,int nt,int ny,int nx,int TR,
    float threshcolor);
extern "C" void rgb2luv(unsigned char *RGB,float *LUV,int size);
extern "C" int quantize(float *B,float **cb,int nt,int ny,int nx,int dim,float thresh);
extern "C" void getcmap(float *B,unsigned char *cmap,float **cb,int npt,int dim,int N);
extern "C" void free_fmatrix(float **m, int nr);
extern "C"float **fmatrix(int nr, int nc);


namespace ImageAnalysis
{
	CJSeg::CJSeg(int numberOfScale, float quanThresh, float mergeThresh)
		:m_numberOfScale(numberOfScale), m_quanThresh(quanThresh), m_mergeThresh(mergeThresh)
	{

	}

	unsigned int CJSeg::Segment(const IImage& img, IGrayImage** ppRegionMap) const
	{
		unsigned char *RGB,*cmap;
		int dim,N,TR,imgsize,mapsize,l;
		float *LUV,**cb;
		unsigned char *rmap;

		if(img.PixelFormat() == IImage::Format24bppRgb)
			dim = 3;
		else if (img.PixelFormat() == IImage::Format8bppGray)
			dim = 1;
		else
		{
			*ppRegionMap = NULL; return 0;
		}
		int NX = img.Width();
		int NY = img.Height();
		imgsize = NX*NY*dim;
		RGB = (unsigned char *) malloc(imgsize*sizeof(unsigned char));
		if(dim == 3)
		{
			for(int i=0; i<img.Height(); ++i)
			{
				for(int j=0; j<img.Width(); ++j)
				{
					RGB[i*(img.Width())*3 + j*3] = img.PixelPtr(j, i)[2];
					RGB[i*(img.Width())*3 + j*3+1] = img.PixelPtr(j, i)[1];
					RGB[i*(img.Width())*3 + j*3+2] = img.PixelPtr(j, i)[0];
				}
			}
		}
		else
		{
			_ASSERT(dim == 1);
			for(int i=0; i<img.Height(); ++i)
			{
				for(int j=0; j<img.Width(); ++j)
				{
					RGB[i*(img.Width()) + j] = *img.PixelPtr(j, i);
				}
			}
		}

		mapsize = NY*NX;

		cb = (float **)fmatrix(256,dim);
		LUV = (float *) malloc(imgsize*sizeof(float));
		if (dim==3) rgb2luv(RGB,LUV,imgsize);
		else if (dim==1) { for (l=0;l<imgsize;l++) LUV[l]=RGB[l]; }
		else { free(RGB); *ppRegionMap = NULL; return 0; }

		N=quantize(LUV,cb,1,NY,NX,dim,m_quanThresh);
		//printf("N=%d\n",N);
		cmap = (unsigned char *) calloc(mapsize,sizeof(unsigned char));
		if (dim==3) rgb2luv(RGB,LUV,imgsize);
		else if (dim==1) { for (l=0;l<imgsize;l++) LUV[l]=RGB[l]; }
		getcmap(LUV,cmap,cb,mapsize,dim,N);
		free_fmatrix(cb,256);
		free (LUV);

		rmap = (unsigned char *)calloc(NY*NX,sizeof(unsigned char));
		TR = segment(rmap,cmap,N,1,NY,NX,RGB,NULL,NULL,0,dim,m_numberOfScale,
		  0,false,1);
		TR = merge1(rmap,cmap,N,1,NY,NX,TR,m_mergeThresh);
		//printf("merge TR=%d\n",TR);
		free(cmap);
	
		CGrayImage* pRMap = new CGrayImage();
		pRMap->Allocate(img.Width(), img.Height() );
		for(int i=0; i<pRMap->Height(); ++i)
		{
			for(int j=0; j<pRMap->Width(); ++j)
			{
				*(pRMap->PixelPtr(j, i)) = rmap[i*pRMap->Width()+j];
			}
		}

		free(RGB);
		free(rmap);		
		*ppRegionMap = pRMap;
		return TR;
	}
}