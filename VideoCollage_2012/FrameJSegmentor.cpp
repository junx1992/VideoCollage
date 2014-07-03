#include "Stdafx.h"
#include "JSeg.h"
#include "GrayImage.h"
#include "VideoSegments.h"
#include "FrameJSegmentor.h"

using namespace ImageAnalysis;
using namespace VideoAnalysis;

std::auto_ptr<CFrameJSegmentor> CFrameJSegmentor::m_Segmentor;

void CFrameJSegmentor::Initialize(int numberOfScale, float quanThresh, float mergeThresh)
{
        if( m_Segmentor.get() == NULL )
            m_Segmentor = std::auto_ptr<CFrameJSegmentor>(new CFrameJSegmentor(numberOfScale, quanThresh, mergeThresh));
}
     
CFrameJSegmentor * CFrameJSegmentor::GetInstance()
{
         return m_Segmentor.get();
}

IGrayImage* CFrameJSegmentor::GetSegment(const CFrame & frame)
{
         if( frame.Id() != m_FrameId )
            DetectSemgment(frame);

         return m_RegionMap;
}
  
unsigned int CFrameJSegmentor::GetSegmentNum(const CFrame & frame)
{
         if( frame.Id() != m_FrameId )
            DetectSemgment(frame);

         return m_SegmentNum;
}

CFrameJSegmentor::CFrameJSegmentor(int numberOfScale, float quanThresh, float mergeThresh)
{
        m_SegmentDetector = new CJSeg(numberOfScale, quanThresh, mergeThresh);
        m_RegionMap = NULL;
        m_SegmentNum = 0;
        m_FrameId = -1;
}

CFrameJSegmentor::~CFrameJSegmentor()
{
        if( m_RegionMap )
            m_RegionMap->Release();
        delete  m_SegmentDetector;
}

void CFrameJSegmentor::DetectSemgment(const CFrame & frame)
{
        if( m_RegionMap )
            m_RegionMap->Release();
        m_SegmentNum = m_SegmentDetector->Segment(*frame.GetImage(), &m_RegionMap);
        m_FrameId = frame.Id();
}

