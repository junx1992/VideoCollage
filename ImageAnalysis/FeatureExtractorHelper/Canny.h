#ifndef _CANNY_H_
#define _CANNY_H_

void canny_core(double s,int cols,int rows,
				byte *data,byte *derivative_mag,byte *magnitude,byte *orientation);
void thresholding_tracker(int high,int low,int cols,int rows,
						  byte *data,byte *magnitude,byte *orientation);
double gaussian(double x,double s);
double hypotenuse(double x,double y);
int follow(int i,int j,int low,int cols,int rows,byte *data,byte *magnitude,byte *orientation);

#endif // _CANNY_H_