//#include "Global.h"
#include <math.h>
#include <windows.h>
#include <stdio.h>
#include "canny.h"

#define ORIENT_SCALE 40.0		// define for canny core

//========================================================================
/*            canny_core.c                */

/* gaussian function (centred at the origin and ignoring the factor
   of 1/(s*sqrt(2*PI)) ) */
double gaussian(double x,double s)
{
    return(exp((-x*x)/(2*s*s)));
}


/* core routine for the canny family of programs */


/* hypot can't cope when both it's args are zero, hence this hack.... */
double hypotenuse(double x,double y)
{
    if (x==0.0 && y==0.0) return(0.0);
    else return(_hypot(x,y));
}

void canny_core(double s,int cols,int rows,
				byte *data,byte *derivative_mag,byte *magnitude,byte *orientation)
//     double s;            /* standard deviation */
//     int cols,rows;            /* picture dimensions */
//     unsigned char *data,*derivative_mag,*magnitude,*orientation;
{
  int filter_width;               /* length of 1-D gaussian mask */
  float *gsmooth_x,*gsmooth_y;
  float *derivative_x,*derivative_y;
  int i,k,n;            /* counters */
  int t;                /* temp. grad magnitude variable */
  double a,b,c,d,g0;        /* mask generation intermediate vars*/
  double ux,uy;
  double t1,t2;
  double G[20],dG[20],D2G[20];    /*gaussian & derivative filter masks*/
  double gc,gn,gs,gw,ge,gnw,gne,gsw,gse;
  int picsize,jstart,jlimit;
  int ilimit;
  register int jfactor;
  int kfactor1,kfactor2;
  int kfactor;
  register int cindex,nindex,sindex,windex,eindex,nwindex,neindex,swindex,seindex;
  int low=1,high=255;        /* tracker hysteresis parameters */
//  int cols_plus,cols_minus;    /*cols+1 and cols-1 respectively*/
  int mag_overflow_count=0;    /* used to measure how oft mag array overflows */
  int mag_underflow_count=0;    /* used to measure how oft mag array underflows */

  picsize=cols*rows;        /* picture area */

  /* calc coeffs for 1-dimensional G, dG/dn and
     Delta-squared G filters */
  for(n=0; n<20; ++n)
    {
      a=gaussian(((double) n),s);
      if (a>0.005 || n<2)
        {
	  b=gaussian((double)n-0.5,s);
	  c=gaussian((double)n+0.5,s);
	  d=gaussian((double)n,s*0.5);
#ifdef _DEBUG
	  //fprintf(stderr,"a,b,c: %lf,%lf,%lf\n",a,b,c);
#endif 
	  G[n]=(a+b+c)/3/(6.283185*s*s);
	  dG[n]=c-b;
	  D2G[n]=1.6*d-a; /* DOG */
#ifdef _DEBUG
	  //fprintf(stderr,"G[%d]: %lf\n",n,G[n]);
	  //fprintf(stderr,"dG[%d]: %lf\n",n,dG[n]);
	  //fprintf(stderr,"D2G[%d]: %lf\n",n,D2G[n]);
#endif
                            
        }
      else break;
    }
  filter_width=n;

  //fprintf(stderr,"canny_core: smooth pic\n");
  /* allocate space for gaussian smoothing arrays */
  if ((gsmooth_x=(float *)calloc(picsize,sizeof(float)))==(float *)NULL)
    {
      fprintf(stderr,"can't alloc gsmooth_x\n");
      exit(0);
    }
  if ((gsmooth_y=(float *)calloc(picsize,sizeof(float)))==(float *)NULL)
    {
      fprintf(stderr,"can't alloc gsmooth_y\n");
      exit(0);
    }

  /* produce x- and y- convolutions with gaussian */

  ilimit=cols-(filter_width-1);
  jstart=cols*(filter_width-1);
  jlimit=cols*(rows-(filter_width-1));
  for (i=filter_width-1;i<ilimit;++i)
    {
      for(jfactor=jstart;
	  jfactor<jlimit;
	  jfactor+=cols)
        {
	  cindex=i+jfactor;
	  t1=data[cindex]*G[0];
	  t2=t1;
	  for(k=1,kfactor1=cindex-cols,
                kfactor2=cindex+cols;
	      k<filter_width;
	      k++,
                kfactor1-=cols,
                kfactor2+=cols)
            {
	      t1+=G[k]*(data[kfactor1]+
			data[kfactor2]);
	      t2+=G[k]*(data[cindex-k]+
			data[cindex+k]);
            }
	  gsmooth_x[cindex]=(float)t1;
	  gsmooth_y[cindex]=(float)t2;
        }
    }
    
  /* allocate space for gradient arrays */
  //fprintf(stderr,"canny_core: find grad\n");
  if ((derivative_x=(float *)calloc(picsize,sizeof(float)))==(float *)NULL)
    {
      fprintf(stderr,"can't alloc x\n");
      exit(0);
    }
  /* produce x and y convolutions with derivative of
     gaussian */

  for (i=filter_width-1;i<ilimit;++i)
    {
      for(jfactor=jstart;
	  jfactor<jlimit;
	  jfactor+=cols)
        {
	  t1=0;
	  cindex=i+jfactor;
	  for (k=1;k<filter_width;++k)
	    t1+=dG[k]*(gsmooth_x[cindex-k]-
                       gsmooth_x[cindex+k]);
	  derivative_x[cindex]=(float)t1;
        }
    }
  free(gsmooth_x);
  if ((derivative_y=(float *)calloc(picsize,sizeof(float)))==(float *)NULL)
    {
      fprintf(stderr,"can't alloc y\n");
      exit(0);
    }

  for (i=n;i<cols-n;++i)
    {
      for(jfactor=jstart;jfactor<jlimit;jfactor+=cols)
        {
	  t2=0;
	  cindex=i+jfactor;
	  for (k=1,kfactor=cols;
	       k<filter_width;
	       k++,kfactor+=cols)
	    t2+=dG[k]*(gsmooth_y[cindex-kfactor]-gsmooth_y[cindex+kfactor]);
	  derivative_y[cindex]=(float)t2;
        }
    }
  free(gsmooth_y);
    
  /* non-maximum suppression (4 cases for orientation of line
     of max slope) */

  //fprintf(stderr,"canny_core: non-maximum suppression\n");
  ilimit=cols-filter_width;
  jstart=cols*filter_width;
  jlimit=cols*(rows-filter_width);

  for (i=filter_width;i<ilimit;++i)
    {
      for (jfactor=jstart;
	   jfactor<jlimit;
	   jfactor+=cols)
        {
				/* calculate current indeces */
	  cindex=i+jfactor;
	  nindex=cindex-cols;
	  sindex=cindex+cols;
	  windex=cindex-1;
	  eindex=cindex+1;
	  nwindex=nindex-1;
	  neindex=nindex+1;
	  swindex=sindex-1;
	  seindex=sindex+1;
	  ux=derivative_x[cindex];
	  uy=derivative_y[cindex];
	  gc=hypotenuse(ux,uy);
	  /* scale gc to fit into an unsigned char array */
	  t=(int)(gc*20.0);
/*fprintf(stderr,"canny_core: i,j=(%d,%d), t=%lf\n",i,jfactor/cols,t);*/
	  derivative_mag[cindex]=(t<256 ? t : 255);
	  gn=hypotenuse(derivative_x[nindex],derivative_y[nindex]);
	  gs=hypotenuse(derivative_x[sindex],derivative_y[sindex]);
	  gw=hypotenuse(derivative_x[windex],derivative_y[windex]);
	  ge=hypotenuse(derivative_x[eindex],derivative_y[eindex]);
	  gne=hypotenuse(derivative_x[neindex],derivative_y[neindex]);
	  gse=hypotenuse(derivative_x[seindex],derivative_y[seindex]);
	  gsw=hypotenuse(derivative_x[swindex],derivative_y[swindex]);
	  gnw=hypotenuse(derivative_x[nwindex],derivative_y[nwindex]);
	  if (ux*uy>0)
            {
	      if(fabs(ux)<fabs(uy))
                {
		  if((g0=fabs(uy*gc))
                     < fabs(ux*gse+(uy-ux)*gs) ||
                     g0<=fabs(ux*gnw+(uy-ux)*gn))
                    continue;
                }
	      else
                {
		  if((g0=fabs(ux*gc))
                     < fabs(uy*gse+(ux-uy)*ge) ||
                     g0<=fabs(uy*gnw+(ux-uy)*gw))
                    continue;
                }
            }
	  else
            {
	      if(fabs(ux)<fabs(uy))
                {
		  if((g0=fabs(uy*gc))
                     < fabs(ux*gne-(uy+ux)*gn) ||
                     g0<=fabs(ux*gsw-(uy+ux)*gs))
		    continue;
                }
	      else
                {
		  if((g0=fabs(ux*gc))
                     < fabs(uy*gne-(ux+uy)*ge) ||
                     g0<=fabs(uy*gsw-(ux+uy)*gw))
		    continue;
                }
            }
	  /* seems to be a good scale factor */
	  magnitude[cindex]=derivative_mag[cindex];
	  /* pi*40 ~= 128 - direction is (thought
	     of as) a signed byte */
	  orientation[cindex]=(char)(atan2(uy, ux)*ORIENT_SCALE);
        }
    } 

  free(derivative_x);
  free(derivative_y);
}

/*            canny_subr.c                */

/* Subroutines used by *canny progs (but not by *canny_j progs) */
int follow(int i,int j,int low,int cols,int rows,byte *data,byte *magnitude,byte *orientation);

/* track the edges in the magnitude array, starting at points that exceed
   the "high" threshold, and continuing to track as long as point values
   don't duck below the "low" threshold (yes, it's hysteresis...I'm sure
   that hyster means "womb" (as in hysterical), but I don't know what
   it's doing in a common engineering term) */
void thresholding_tracker(int high,int low,int cols,int rows,
						  byte *data,byte *magnitude,byte *orientation)
//int high,low;        /* threshold values */
//int cols,rows;        /* picture size */
//unsigned char *data;    /* o/p pic array */
//unsigned char *magnitude;    /* gradient magnitude array */
//unsigned char *orientation;    /* gradient direction array */
{
    int i,j;    /* counters */
    int picsize;    /* picture area */

fprintf(stderr,"thresholding_tracker: tracking edges, high=%d, low=%d\n",high,low);
    /* clear data array before tracking */
    picsize=cols*rows;
    //for (i=0;i<picsize;++i) data[i]=0;
	memset(data, 0, picsize);

    /* threshold image with hysteresis: follow from
       each strong edge point */
    for (i=0;i<cols;++i)
    {
        for (j=0;j<rows;++j)
            if (magnitude[i+cols*j]>=high)
                follow(i,j,low,cols,rows,data,magnitude,orientation);
    }
}

/* follow a connected edge */
int follow(int i,int j,int low,int cols,int rows,byte *data,byte *magnitude,byte *orientation)
//int i,j;    /* start point */
//int low;    /* lower threshold value */
//int cols,rows;    /* picture dimensions */
//unsigned char *data;    /* o/p pic array */
//unsigned char *magnitude;    /* gradient magnitude array */
//unsigned char *orientation;    /* gradient direction array */
{
    int k,l;        /* counters */
    int i_plus_1,i_minus_1,j_plus_1,j_minus_1;
    long index,kindex;
    char break_flag;

    i_plus_1=i+1;
    i_minus_1=i-1;
    j_plus_1=j+1;
    j_minus_1=j-1;
    index=i+j*cols;
    if (j_plus_1>=rows) j_plus_1=rows-1;
    if (j_minus_1<0) j_minus_1=0;
    if (i_plus_1>=cols) i_plus_1=cols-1;
    if (i_minus_1<0) i_minus_1=0;
/*fprintf(stderr,"follow: i,j=%d %d, i_plus_1,i_minus_1=%d %d\n",i,j,i_plus_1,i_minus_1);*/
    if (!data[index])
    {
/*fprintf(stderr,"following %d %d\n",i,j);*/
        /* current point must be added to the list of tracked points */
        data[index]=magnitude[index];
        /* now we can check immediately adjacent points to see if
           they too could be added to the track */
        break_flag=0;
        for (k=i_minus_1;k<=i_plus_1;k++)
        {
            for(l=j_minus_1;l<=j_plus_1;++l)
            {
                kindex=k+l*cols;
                if (!(l==j && k==i) &&
                    magnitude[kindex]>=low/* &&
                    abs(abs(orientation[index]-orientation[kindex])-128)>120*/)
                {
                    if (follow(k,l,low,cols,rows,data,magnitude,orientation))
                    {
                        break_flag=1;
                        break;
                    }
                }
            }
            if (break_flag) break;
        }
        return(1);
    }
    else return(0);
}

