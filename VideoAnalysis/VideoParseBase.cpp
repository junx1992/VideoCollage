#include "StdAfx.h"
#include "VideoParseBase.h"
#include <vector>
using std::vector;

namespace VideoAnalysisHelper
{
    using namespace VideoAnalysis;
    class CVideoParseReceiverListImp
    {
        typedef vector<IVideoParseReceiver*> ReceiverArray;
    public:
        HRESULT AddReceiver(IVideoParseReceiver* const pRec)
        {
            if(pRec == NULL)
                return E_FAIL;
            m_receivers.push_back(pRec);
            return S_OK;
        }
        HRESULT RemoveReceiver(const IVideoParseReceiver* const pRec)
        {
            HRESULT hr = S_FALSE;
            for(ReceiverArray::iterator p = m_receivers.begin(); p != m_receivers.end();)
            {
                if(pRec == *p)
                {
                    p = m_receivers.erase(p);
                    hr = S_OK;
                }
				else
					++p;
            }
            return hr;
        }

        template<class Func>
        HRESULT Foreach(Func f)
        {
            HRESULT hr = S_OK;
            for(ReceiverArray::iterator p = m_receivers.begin(); p != m_receivers.end(); ++p)
            {
                if(FAILED(((*p)->*f)()))
                    hr = E_FAIL;
            }
            return hr;
        }

        template<class T, class Func>
        HRESULT Foreach(Func f, T& t)
        {
            HRESULT hr = S_OK;
            for(ReceiverArray::iterator p = m_receivers.begin(); p != m_receivers.end(); ++p)
            {
                if(FAILED(((*p)->*f)(t)))
                    hr = E_FAIL;
            }
            return hr;
        }

    private:
        ReceiverArray m_receivers;
    };
}

namespace VideoAnalysis
{
    CVideoParseReceiverList::CVideoParseReceiverList()
        :m_pImp(new VideoAnalysisHelper::CVideoParseReceiverListImp())
    {}

    CVideoParseReceiverList::~CVideoParseReceiverList()
    {
        delete this->m_pImp;
    }

    HRESULT CVideoParseReceiverList::AddReceiver(IVideoParseReceiver* const pRec)
    {
        return m_pImp->AddReceiver(pRec);
    }

    HRESULT CVideoParseReceiverList::RemoveReceiver(const IVideoParseReceiver* const pRec)
    {
        return m_pImp->RemoveReceiver(pRec);
    }

    HRESULT CVideoParseReceiverList::OnNewSegment(IVideoSegment& segment)
    {
        return m_pImp->Foreach(&IVideoParseReceiver::OnNewSegment, segment);
    }
    /*HRESULT CVideoParseReceiverList::StartStreaming()
    {
        return m_pImp->Foreach(&IVideoParseReceiver::StartStreaming);
    }

	HRESULT CVideoParseReceiverList::StopStreaming()
    {
        return m_pImp->Foreach(&IVideoParseReceiver::StopStreaming);
    }*/

	HRESULT CVideoParseReceiverList::EndOfStream()
    {
        return m_pImp->Foreach(&IVideoParseReceiver::EndOfStream);
    }
}
