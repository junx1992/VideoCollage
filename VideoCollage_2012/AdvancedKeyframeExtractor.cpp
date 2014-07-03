#include "StdAfx.h"
#include "AdvancedKeyframeExtractor.h"
#include "RgbImage.h"
#include "GrayImage.h"
#include "VideoSegments.h"
#include "VxFeature.h"
#include "ColorHistogramFeatureExtractor.h"
#include "CommonFrameFeatureFactory.h"
#include "ImageProcess.h"
#include "weight.h"

using namespace ImageAnalysis;
using namespace VideoAnalysis;


namespace VideoAnalysis
{
    //get the intersect sum of 2 features
    unsigned long AdvancedIntersect(const CIntFeature &H1, const CIntFeature &H2)
    {
        unsigned long sum = 0;

        //get the size of each Featrue
        int size_h1 = H1.Size();
        int size_h2 = H2.Size();

        //check the H1's size == H2's size
        if( size_h1 != size_h2 )
            return sum;

        for ( int i = 0; i < size_h1; ++i)
        {
            sum += H1[i] < H2[i] ? H1[i] : H2[i];
        }

        return sum;
    }
    
    //some thresholds of key frame extractor
    struct AdvancedKeyFrameExtractorThreshold
    {
        static const double dblKeyFrameThresh;            //0.22 Threshold 
        static const int  iSkippedFrames;                 //5  we should skip the first frames for denoising
    };

    const double AdvancedKeyFrameExtractorThreshold::dblKeyFrameThresh = 0.22;
    const int AdvancedKeyFrameExtractorThreshold::iSkippedFrames = 5;


    CAdvancedKeyframeExtractor::CAdvancedKeyframeExtractor(void) : 
        m_Shot(3),
        m_isBlackScreenPre(false),
        m_DifferenceExtractor(new ImageAnalysis::CDifferenceFeatureExtractor()),
        m_SkinExtractor(new ImageAnalysis::CSkinFeatureExtractor()),
        m_EntropyExtractor(new ImageAnalysis::CEntropyFeatureExtractor())
    {
        m_DifferenceExtractor.SetPoolMethod(CShotFeatureExtractor::PoolMethod::Mean);
        m_SkinExtractor.SetPoolMethod(CShotFeatureExtractor::PoolMethod::Keyframe);
        m_EntropyExtractor.SetPoolMethod(CShotFeatureExtractor::PoolMethod::Keyframe);
        //SetLogFile(L"shot.txt");
    }
    CAdvancedKeyframeExtractor::~CAdvancedKeyframeExtractor(void)
    {
        m_log.close();
    }
    CFrameList & CAdvancedKeyframeExtractor::OnNewSegment(IVideoSegment& segment)
    {
            m_Shot = 3;
            unsigned int shotBgnFid = segment.BeginFrameId();
            unsigned int shotEndFid = segment.EndFrameId();
            ChooseKeyframe(shotBgnFid, shotEndFid); // this is the core step
            if ( m_Shot > 0 ) --m_Shot; // m_Shot = 2
            if (m_KeyframeList.Size() > 0) {
                IVideoSegment *Keyframe = m_KeyframeList.GetFrame(0);
                m_DifferenceExtractor.OnNewSegment(*Keyframe);
                m_SkinExtractor.OnNewSegment(*Keyframe);
                //m_EntropyExtractor.OnNewFrame(m_KeyframeList[0], m_KeyframeList[0].Id());
                m_EntropyExtractor.OnNewSegment(*Keyframe);
            } else {
                m_DifferenceExtractor.AbandonCurShot();
                m_SkinExtractor.AbandonCurShot();
                m_EntropyExtractor.AbandonCurShot();
            }
            // std::cout << "new segment\n";
            return m_KeyframeList;
    }

    HRESULT CAdvancedKeyframeExtractor::OnNewFrame(CFrame& frame, bool isKeyframeCandidate)
    {
        bool isAddedKeyframeCandidate = false;
        //get the frame index
        m_FrameId = frame.Id();
     
        HRESULT _ret = S_OK;
        if ( AdvancedKeyFrameExtractorThreshold::iSkippedFrames > m_FrameId )
        { // early quit 
            SaveFrameFeature(frame); // set m_isBlackScreenPre		 
            return S_OK;
        } else if ( AdvancedKeyFrameExtractorThreshold::iSkippedFrames == m_FrameId ) {
                m_Shot = 3; // this is wierd
        } else if( isKeyframeCandidate ) {
            double k = 1.0 - double(AdvancedIntersect(m_HistRef, CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame))) / (frame.GetImage()->Width()*frame.GetImage()->Height());
            if ( 
                k > AdvancedKeyFrameExtractorThreshold::dblKeyFrameThresh // some difference
                && !m_isBlackScreenPre ) 
            {
                AddKeyFrameCandidate(frame);
                isAddedKeyframeCandidate = true;
                m_HistRef = CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
                m_EntropyExtractor.OnNewFrame(frame, true); // yang wang
                m_DifferenceExtractor.OnNewFrame(frame, true); // yang wang
                m_SkinExtractor.OnNewFrame(frame, true); // yang wang
            }
            _ret = S_OK;
        }

        if ( m_Shot > 0 ) --m_Shot;
        if ( m_Shot == 1 ) 
        { // force keeping one frame when m_Shot dropping to 1
            if( !CommonFrameFeatureFactory::GetInstance()->IsBlackScreen(frame))
            {
                if (!isAddedKeyframeCandidate) {
                    AddKeyFrameCandidate(frame);
                    isAddedKeyframeCandidate = true;
                    m_HistRef = CommonFrameFeatureFactory::GetInstance()->ExtractRGB4096Histo(frame);
                    m_EntropyExtractor.OnNewFrame(frame, true); // yang wang
                    m_DifferenceExtractor.OnNewFrame(frame, true); // yang wang
                    m_SkinExtractor.OnNewFrame(frame, true); // yang wang
                }
            }
            else
            {   // failed to force keeping -> restore m_Shot to wait next chance
                m_Shot++; // (shot = 2)
            }
        }

        //save the current frame's feature for future use
        SaveFrameFeature(frame);
        if (!isAddedKeyframeCandidate)
            m_DifferenceExtractor.OnNewFrame(frame, false); // make up, yang wang
        return _ret;
    }

     //add a key frame candidate in the frame buffer
     void CAdvancedKeyframeExtractor::AddKeyFrameCandidate(CFrame & condKey)
     {
             CFrame newCandidate;
             newCandidate.Copy(condKey);
             m_FrameArray.Add(newCandidate); // there will be duplicate frames
             // std::cout << m_FrameArray.Size() << ": " << newCandidate.Id() << std::endl;
     }

     //save pre-frame's feature
     void CAdvancedKeyframeExtractor::SaveFrameFeature(CFrame & frame)
     {
            m_isBlackScreenPre = CommonFrameFeatureFactory::GetInstance()->IsBlackScreen(frame);
     }

     // extract some key frames for the shot
     // this could be the most important step
     // it choose the best scored frame within a shot
     void CAdvancedKeyframeExtractor::ChooseKeyframe(unsigned int frameBgnId, unsigned int frameEndId)
     {

        //clear the pre key frame list 
        m_KeyframeList.Clear();
        //check keyframe candidate amount
        int size = m_FrameArray.Size();
        //if there is no key frame candidate in the frame buffer
        if( size == 0 ) return; 
        int num = 0;

        // below is the old method which is not remarked
        //we choose the middlest keyframe for output, old method
        unsigned int middle = (frameBgnId + frameEndId)>>1;
        for( num = 0; num < size; ++num )
        {
                if( unsigned long(m_FrameArray[num].Id()) >= middle )
                {
                    unsigned long diff1 = (num > 0 ) ? middle - m_FrameArray[num-1].Id() : 0xFFFF;
                    unsigned long diff2 = m_FrameArray[num].Id() - middle;
                    if( diff1 < diff2 ) --num;
                    break;
                }
        }
        //if this conditional happened, we use the last key frame
        if( num == size ) --num;
        middle = num;

        // below is the new method which choose the best scored frame of the shot as the keyframe
        // choose frame with max score, new method added by wangyang 2012
        int start = 0, end = size;
        if (size >= 3) { ++start; --end; } // remove the first and last frame because they may not stable an 
        double m = 0;
        for (num = start; num < end; ++num) {
            int id = m_FrameArray[num].Id();
            if (id > (int)frameEndId)
                break;
            double skin = m_SkinExtractor.InCurrentShot(num)[0];
            double face = m_SkinExtractor.InCurrentShot(num)[1];
            double entropy = m_EntropyExtractor.InCurrentShot(num)[0];
            // double motion = m_DifferenceExtractor.InCurrentShot(num)[0]; this is not correct because there may be extra frames (Frame before candidates)
            double motion = m_DifferenceExtractor.InCurrentShotById(id)[0];
            double duration = 0;
            double score = weight(face, skin, duration, -motion * 50, entropy * .2); // crucial
            if (m < score) {
                m = score;
                middle = num;
            }
            
        }
        num = middle;
        // output all the ids
        static int shot = 0;
        if (m_log.is_open()) {
            m_log << shot << endl;
            for (int i = 0; i < size; ++i) {
                int id = m_FrameArray[i].Id();
                if (id > (int)frameEndId)
                    break;
                double skin = m_SkinExtractor.InCurrentShot(i)[0];
                double face = m_SkinExtractor.InCurrentShot(i)[1];
                double entropy = m_EntropyExtractor.InCurrentShot(i)[0];
                // double motion = m_DifferenceExtractor.InCurrentShot(i)[0];
                double motion = m_DifferenceExtractor.InCurrentShotById(id)[0]; // safer for this
                double duration = 0;
                m_log << m_FrameArray[i].Id() <<'\t'<<PCT(skin)<<'\t'<<PCT(face)<<'\t'<<PCT(motion)<<'\t'<<PCT(entropy);
                if (i == num)
                    m_log <<'.'<<endl;
                else
                    m_log <<endl;
            }
        }
        //we only use key frame in the range [BgnNum, EndNum]
        if( unsigned long(m_FrameArray[num].Id()) > frameEndId ) {
            //reset keyframes for next shot
            m_FrameArray.Clear();
            return;
        }
        //save the key frame
        m_KeyframeList.Add(m_FrameArray[num]);
        if (m_log.is_open()) {
            m_log << endl;
            // int shot = m_KeyframeList.Size() - 1;
            wchar_t temp[10];
            wstring shotroot = logpath + _itow(shot++, temp, 10);
            CreateDirectory(shotroot.c_str(), NULL);
            shotroot = shotroot + L"\\";
            for (int i = 0; i < m_FrameArray.Size(); ++i) {
                m_FrameArray.GetFrame(i)->GetImage()->Save((shotroot + _itow(m_FrameArray.GetFrame(i)->Id(), temp, 10) + L".jpg").c_str());
            }
        }
        //reset keyframes for next shot
        m_FrameArray.Clear();
    }
    void CAdvancedKeyframeExtractor::SetLogPath(wchar_t *_logpath)
    {
        m_log.close();
        logpath = _logpath;
        m_log.open(logpath + L"shots.txt");
    }

}