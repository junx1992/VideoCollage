#include "stdafx.h"
#include "RgbImage.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <memory>
#include <gdiplus.h>
#include <list>
#include <algorithm>
#include <math.h>
using namespace std;
using namespace Gdiplus;
using namespace ImageAnalysis;

#define LHThreshold 10

inline BYTE dbl2byte(double v);

void EnhanceImage(CRgbImage &image);
