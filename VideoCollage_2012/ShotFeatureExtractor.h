#pragma once
#include "VxComm.h"
#include "VideoSegments.h"
#include "VxDataSet.h"
#include "memory"
#include "FrameFeatureExtractor.h"

using namespace VxCore;

namespace VideoAnalysis
{
    // A interface for the receiver of a shot detector. 
    // Shot detector will call OnNewFrame when there is a normal frame.
    // shot detector will call OnNewSegment when there is a shot boundary
    // This class call certain frame feature extractor at each received frame (in OnNewFrame)
    // and pool those features when a shot boundary is detected (in OnNewSegment).
    class CShotFeatureExtractor : public IKeyframeExtractor
    {
    public:
        enum PoolMethod	{ Max, Min, Mid, Mean, Keyframe };
    private:
        CDataSet m_Features;
        //CFeatureExtractorForRgbImageBase *m_pFrameFeature
        CFrameFeatureExtractor *m_pFrameFeatureExtractor;
        std::auto_ptr<CFrameFeatureExtractor> ap;
        std::auto_ptr<ImageAnalysis::IImageFeatureExtractor> apIFE;
        CDoubleFeature AveragePositive(const IDataSet &data);
        CDoubleFeature MidPositive(const IDataSet &data);
        CDataSet m_Data;
        // This is for frame difference because it need successive frame to extract frame difference.
        bool m_isNeedImmediateFrame;
        int m_PreId;
        mutable int m_RecievedFrames;
        PoolMethod m_pm;
    public:
        virtual CFrameList& OnNewSegment(IVideoSegment& segment);
        virtual HRESULT OnNewFrame(CFrame& frame, bool isKeyframeCandidate);
        void Clear();
        void AbandonCurShot();
        virtual const CDataSet& GetData();
        CShotFeatureExtractor(CFrameFeatureExtractor *pFrameFeatureExtractor = NULL);
        CShotFeatureExtractor(ImageAnalysis::IImageFeatureExtractor *pImgFE);
        ~CShotFeatureExtractor(void);
        void SetImageFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE);
        void SetFrameFeatureExtractor(CFrameFeatureExtractor* pFrmFE);
        bool NeedImmediateFrame();
        void SetPoolMethod(PoolMethod pm);
        CDoubleFeature InCurrentShot(int i);
        const CDoubleFeature InCurrentShotById(int id);
    };
}