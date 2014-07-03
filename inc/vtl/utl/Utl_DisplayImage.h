/*************************************************************************\
Microsoft Research Asia
Copyright (c) 2003 Microsoft Corporation

Module Name:
    Utility Lib: Display a window for given image
  
Abstract:
    
Notes:

Usage:
        
History:
    Created  on 2004 Sep. 25 by oliver_liyin
          
\*************************************************************************/

#pragma once

#include "img/Img_Image.h"

namespace utl {

    /// Display a window to show the given image;
    HRESULT DisplayImage(const img::IImage& ref, const TCHAR szName[] = NULL);

}   // namespace utl
