#include "StdAfx.h"
#include "ShotFeatureExtractor.h"
#include <queue>
namespace VideoAnalysis
{
	CShotFeatureExtractor::CShotFeatureExtractor(CFrameFeatureExtractor *pFrameFeatureExtractor) : m_isNeedImmediateFrame(false), m_PreId(-1), m_RecievedFrames(0)
	{
		SetFrameFeatureExtractor(pFrameFeatureExtractor);
	}
	CShotFeatureExtractor::CShotFeatureExtractor(ImageAnalysis::IImageFeatureExtractor *pImgFE) : m_isNeedImmediateFrame(false), m_PreId(-1), m_RecievedFrames(0)
	{
		SetFrameFeatureExtractor(NULL);
		SetImageFeatureExtractor(pImgFE);
		//apIFE.release(pImgFE); // done in FrameFeatureExtractor
	}
	CShotFeatureExtractor::~CShotFeatureExtractor(void)
	{
	}
	void CShotFeatureExtractor::SetFrameFeatureExtractor(CFrameFeatureExtractor* pFrmFE)
	{
		if (pFrmFE) {
			m_pFrameFeatureExtractor = pFrmFE;
		} else {
			m_pFrameFeatureExtractor = new CFrameFeatureExtractor();
			ap.reset(m_pFrameFeatureExtractor);
		}
	}
	void CShotFeatureExtractor::SetImageFeatureExtractor(ImageAnalysis::IImageFeatureExtractor* pImgFE)
	{
		m_pFrameFeatureExtractor->SetImageFeatureExtractor(pImgFE);
	}
	const CDataSet& CShotFeatureExtractor::GetData()
	{
		return m_Data;
	}
	
	// This callback function will be called by shot detector at each normal frame
	HRESULT CShotFeatureExtractor::OnNewFrame(CFrame& frame, bool isKeyframeCandidate)
	{
		if (frame.Id() == m_PreId)
			return E_FAIL;
		if (isKeyframeCandidate)
			++m_RecievedFrames;
		if (isKeyframeCandidate || m_isNeedImmediateFrame) {
			m_pFrameFeatureExtractor->OnNewSegment(frame); // extract feature from the frame.
			m_PreId = frame.Id();
		}
		m_isNeedImmediateFrame = isKeyframeCandidate;
		return S_OK;
	}
	bool CShotFeatureExtractor::NeedImmediateFrame()
	{
		return m_isNeedImmediateFrame;
	}

	// This callback function will be called once the shot detector finds a shot boundary
	// Pool features of all candidate frames as one feature
	CFrameList& CShotFeatureExtractor::OnNewSegment(IVideoSegment& segment)
	{
		const IDataSet &data = m_pFrameFeatureExtractor->GetData();
		CDoubleFeature pooled(-1, 1, -1);
		double M = 0, m = 10e15;
		switch (m_pm) {
		case Max:
			for (unsigned int i = 0; i < data.GetSampleNum(); ++i) {
				if (M < data.GetSample(i)[0]) {
					M = data.GetSample(i)[0];
					pooled = data.GetSample(i);
				}
			}
			break;
		case Min:
			for (unsigned int i = 0; i < data.GetSampleNum(); ++i) {
				if (m > data.GetSample(i)[0]) {
					m = data.GetSample(i)[0];
					pooled = data.GetSample(i);
				}
			}
			break;
		case Mean:
			pooled = AveragePositive(data);
			break;
		case Mid:
			pooled = data.GetSample(data.GetSampleNum() / 2);
			break;
		case Keyframe:
			pooled = *data.GetSampleById(segment.GetKeyframe(0)->Id());
			break;
		}

		if (pooled[0] >= 0) // score are obtained
			m_Data.AddSample(pooled);
		else if (m_RecievedFrames > 0) { // not enough data to get score
			pooled.PushBack(0);
			m_Data.AddSample(pooled);
		}
		m_pFrameFeatureExtractor->Clear();
		m_RecievedFrames = 0;
		return CFrameList();
	}

	void CShotFeatureExtractor::Clear()
	{
		m_Data.Clear();
		m_isNeedImmediateFrame = false;
	}
	void CShotFeatureExtractor::AbandonCurShot()
	{
		m_pFrameFeatureExtractor->Clear();
	}
	/*
	struct cmp { bool operator ()(const CDoubleFeature &a, const CDoubleFeature &b) { return a[0] < b[0]; } };
	CDoubleFeature CShotFeatureExtractor::MidPositive(const IDataSet &data)
	{
		int N = data.GetSampleNum();
		const int D = data.GetSample(0).Size();
		CDoubleFeature ave(data.GetSample(0).Id(), D, -1);
		std::priority_queue<CDoubleFeature, std::vector<CDoubleFeature>, cmp> pq;
		int n = 0;
		for (int i = 0; i < N; ++i) {
			if (data.GetSample(i)[0] > 0) {
				pq.push(data.GetSample(i));
				++n;
			}
		}
		n <<= 1;
		for (int i = 0; i < N; ++i)
			pq.pop();
		std::cout << pq.top().Id() << std::endl;
		if (pq.size() == 0)
			return ave;
		else
			return pq.top();
	}
	*/
	CDoubleFeature CShotFeatureExtractor::InCurrentShot(int i)
	{
		// wangyang edited 20130104
		const IDataSet &ids = m_pFrameFeatureExtractor->GetData();
		if (ids.GetSampleNum() > i)
			return ids.GetSample(i);
		else {
			return CDoubleFeature(0, 1);
		}
	}
	const CDoubleFeature CShotFeatureExtractor::InCurrentShotById(int id)
	{
		const IDataSet &ids = m_pFrameFeatureExtractor->GetData();
		const CDoubleFeature *f = ids.GetSampleById(id);
		if (f)
			return *f;
		return CDoubleFeature(0, 1);
	}
	CDoubleFeature CShotFeatureExtractor::AveragePositive(const IDataSet &data)
	{
		const int N = data.GetSampleNum();
		if (N == 0) {
			CDoubleFeature ave(-1, 1, -1);
			return ave;
		}
		const int D = data.GetSample(0).Size();
		CDoubleFeature ave(data.GetSample(0).Id(), D, -1);
		for (int d = 0; d < D; ++d) {
			double s = 0;
			int n = 0;
			for (int i = 0; i < N; ++i) {
				const double &v = (data.GetSample(i))[d];
				if (v >= 0) {
					s += v;
					++n;
				}
			}
			if (n > 0)
				ave[d] = s / n;
			else // for debug
				ave[d] = -1;
		}
		return ave;
	}
	void CShotFeatureExtractor::SetPoolMethod(PoolMethod pm)
	{
		m_pm = pm;
	}
}