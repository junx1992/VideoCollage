#include "StdAfx.h"
#include "VideoSegments.h"
#include "VideoAnalysisException.h"
#include <vector>
using std::vector;

namespace VideoAnalysisImp
{
	using namespace VideoAnalysis;
    //a list that hold a copy for every added element
    template<class T>
    class CListImp
    {
        typedef vector<T*> ListImpT;
    public:
        CListImp()
        {

        }

        CListImp(const CListImp& list)
        {
           for(ListImpT::const_iterator p = list.m_list.begin(); p != list.m_list.end(); ++p)
           {
               this->Add(**p);
           }
        }

        CListImp& operator=(const CListImp& list)
        {
            if(this == &list)
                return *this;
            m_list.clear();
            for(ListImpT::const_iterator p = list.m_list.begin(); p != list.m_list.end(); ++p)
            {
                this->Add(**p);
            }
            return *this;
        }

        ~CListImp()
        {
            Clear();
        }

        bool Add(const T& element)
        {
            T* pNew = (T*) element.Clone();
            m_list.push_back(pNew);
            return true;
        }

        bool Remove(const T& element)
        {
            for(ListImpT::iterator p = m_list.begin(); p != m_list.end(); ++p)
            {
                if(element == *(*p) )
                {
                   (*p)->Release();
                    m_list.erase(p);
                    return true;
                }
            }
            return false;
        }

        bool Remove(const int elementId)
        {
            for(ListImpT::iterator p = m_list.begin(); p != m_list.end(); ++p)
            {
                if(elementId == (*p)->Id())
                {
                    (*p)->Release();
                    m_list.erase(p);
                    return true;
                }
            }
            return false;
        }
 
		void Clear()
		{
            for(ListImpT::iterator p = m_list.begin(); p != m_list.end(); ++p)
                (*p)->Release();
			m_list.clear();
		}

        int GetElementNum() const
        {
            return (int)m_list.size(); 
        }

        int Size() const
        {
            return GetElementNum();
        }

        const T* GetElement(const int num) const
        {
            _ASSERT(num >=0 && num < Size() );
            return m_list[num];
        }

        T* GetElement(const int num)
        {
            _ASSERT(num >=0 && num < Size() );
            return m_list[num];
        }

        const T* GetElementById(const int elementId) const
        {
            for(ListImpT::iterator p = m_list.begin(); p != m_list.end(); ++p)
            {
                if(elementId == (*p)->Id())
                {
                    return *p;
                }
            }
            return NULL;
        }
      
        T* GetElementById(const int elementId)
        {
            for(ListImpT::iterator p = m_list.begin(); p != m_list.end(); ++p)
            {
                if(elementId == (*p)->Id())
                {
                    return *p;
                }
            }
            return NULL;
        }
    private:
        ListImpT m_list;
    };

    class CVideoSegmentListImp: public CListImp<IVideoSegment>
    {
    };

    class CFrameListImp: public CListImp<CFrame>
    {
    };

    class CIdListImp
    {
    public:
        bool Add(const int id)
        {
            m_ids.push_back(id);
            return true;
        }

        bool Remove(const int id)
        {
            for(vector<int>::iterator p = m_ids.begin(); p != m_ids.end(); ++p)
                if(*p == id)
                {
                    m_ids.erase(p);
                    return true;
                }
            return false;
        }

        int Size() const
        {
            return (int)m_ids.size();
        }

        int GetId(const int num)
        {
            _ASSERT(num >=0 && num < Size() );
            return m_ids[num];
        }
    private:
        vector<int> m_ids;
    };
}

namespace VideoAnalysis
{
    using namespace VideoAnalysisImp;
#pragma region CFrameList

        CFrameList::CFrameList()
            :m_pImp(new CFrameListImp())
        {

        }

        CFrameList::CFrameList(const CFrameList& r)
        {
            m_pImp = new CFrameListImp();
            *m_pImp = *r.m_pImp;
        }

        CFrameList& CFrameList::operator=(const CFrameList& r)
        {
            if(this == &r)
                return *this;
            if(NULL == m_pImp)
                m_pImp = new CFrameListImp();
            *m_pImp = *r.m_pImp;
            return *this;
        }

        CFrameList::~CFrameList()
        {
            delete m_pImp;
        }

        bool CFrameList::Add(const CFrame& pFrame)
        {
            return m_pImp->Add(pFrame);
        }

        bool CFrameList::Remove(const CFrame& pFrame)
        {
            return m_pImp->Remove(pFrame);
        }

        bool CFrameList::Remove(const int frameId)
        {
            return m_pImp->Remove(frameId);
        }

		void CFrameList::Clear()
		{
		    return m_pImp->Clear();
		}

        int CFrameList::GetFrameNum() const
        {
            return m_pImp->GetElementNum();
        }

        int CFrameList::Size() const
        {
            return m_pImp->Size();
        }

        const CFrame* CFrameList::GetFrame(const int num) const
        {
            return m_pImp->GetElement(num);
        }

        CFrame* CFrameList::GetFrame(const int num)
        {
            return m_pImp->GetElement(num);
        }
       
        const CFrame* CFrameList::GetFrameById(const int frameId) const
        {
            return m_pImp->GetElementById(frameId);
        }
        
        CFrame* CFrameList::GetFrameById(const int frameId)
        {
            return m_pImp->GetElementById(frameId); 
        }
      
        CFrame& CFrameList::operator[](const int num)
        {
            return *GetFrame(num);
        }

        const CFrame& CFrameList::operator[](const int num) const
        {
            return *GetFrame(num);
        }    
#pragma endregion

#pragma region CVideoSegmentList
        CVideoSegmentList::CVideoSegmentList()
            :m_pImp(new CVideoSegmentListImp())
        {
        }

        CVideoSegmentList::CVideoSegmentList(const CVideoSegmentList& r)
        {
             m_pImp = new CVideoSegmentListImp();
            *m_pImp = *r.m_pImp;
        }

        CVideoSegmentList& CVideoSegmentList::operator=(const CVideoSegmentList& r)
        {
            if(this == &r)
                return *this;
            if(NULL == m_pImp)
                m_pImp = new CVideoSegmentListImp();
            *m_pImp = *r.m_pImp;
            return *this;
        }

        CVideoSegmentList::~CVideoSegmentList()
        {
            delete m_pImp;
        }

        bool CVideoSegmentList::Add(const IVideoSegment& pSegment)
        {
            return m_pImp->Add(pSegment);
        }

        bool CVideoSegmentList::Remove(const IVideoSegment& pSegment)
        {
            return m_pImp->Remove(pSegment);
        }

        bool CVideoSegmentList::Remove(const int segmentId)
        {
            return m_pImp->Remove(segmentId);
        }

		void CVideoSegmentList::Clear()
        {
                m_pImp->Clear();
        }

        int CVideoSegmentList::GetSegmentNum() const
        {
            return m_pImp->GetElementNum();
        }

        int CVideoSegmentList::Size() const
        {
            return m_pImp->Size();
        }

        const IVideoSegment* CVideoSegmentList::GetSegment(const int num) const 
        {
            return m_pImp->GetElement(num);
        }

        IVideoSegment* CVideoSegmentList::GetSegment(const int num)
        {
            return m_pImp->GetElement(num);
        }

        const IVideoSegment* CVideoSegmentList::GetSegmentById(const int segmentId) const 
        {
            return m_pImp->GetElementById(segmentId);
        }

        IVideoSegment* CVideoSegmentList::GetSegmentById(const int segmentId)
        {
            return m_pImp->GetElementById(segmentId);
        }

        IVideoSegment& CVideoSegmentList::operator[](const int num)
        {
            return *GetSegment(num);
        }

        const IVideoSegment& CVideoSegmentList::operator[](const int num) const
        {
            return *GetSegment(num);
        }

#pragma endregion

#pragma region CIdList
        CIdList::CIdList()
            :m_pImp(new CIdListImp)
        {

        }

        CIdList::CIdList(const CIdList& r)
        {
             m_pImp = new CIdListImp();
            *m_pImp = *r.m_pImp;
        }

        CIdList& CIdList::operator=(const CIdList& r)
        {
            if(this == &r)
                return *this;
            if(NULL == m_pImp)
                m_pImp = new CIdListImp();
            *m_pImp = *r.m_pImp;
            return *this;
        }

        CIdList::~CIdList()
        {
            delete m_pImp;
        }

        bool CIdList::Add(const int Id)
        {
            return m_pImp->Add(Id);
        }

        bool CIdList::Remove(const int Id)
        {   
            return m_pImp->Remove(Id);
        }

        int CIdList::Size() const
        {
            return m_pImp->Size();
        }

        int CIdList::GetId(const int num) const
        {
            return m_pImp->GetId(num);
        }

        int CIdList::operator[](const int num) const
        {
            return GetId(num);
        }

#pragma endregion

#pragma region CVideoSegmentBase
    CVideoSegmentBase::CVideoSegmentBase(const int id)
        :m_id(id), m_beginTime(0), m_endTime(0), m_beginFrameId(0), m_endFrameId(0)
    {}

   /* CVideoSegmentBase::CVideoSegmentBase(const CVideoSegmentBase& r)
    {

    }

    CVideoSegmentBase::~CVideoSegmentBase()
    {

    }*/

    CVideoSegmentBase& CVideoSegmentBase::operator=(const CVideoSegmentBase& r)
    {
        m_id = r.m_id;
        m_beginTime = r.m_beginTime;
        m_endTime = r.m_endTime;
        m_beginFrameId = r.m_beginFrameId;
        m_endFrameId = r.m_endFrameId;
        m_keyFrameList = r.m_keyFrameList;
        m_childrenList = r.m_childrenList;
        return *this;
    }

    int CVideoSegmentBase::Id() const
    {
        return m_id;
    }

    void CVideoSegmentBase::Id(const int id)
    {
        m_id = id;
    }

    __int64 CVideoSegmentBase::BeginTime() const
    {
        return m_beginTime;
    }

    void CVideoSegmentBase::BeginTime(const __int64 time)
    {
        m_beginTime = time;
    }

    __int64 CVideoSegmentBase::EndTime() const
    {
        return m_endTime;
    }

    void CVideoSegmentBase::EndTime(const __int64 time)
    {
        m_endTime = time;
    }

    int CVideoSegmentBase::BeginFrameId() const
    {
        return m_beginFrameId;
    }

    void CVideoSegmentBase::BeginFrameId(const int frameId)
    {
        m_beginFrameId = frameId;
    }

    int CVideoSegmentBase::EndFrameId() const
    {
        return m_endFrameId;
    }

    void CVideoSegmentBase::EndFrameId(const int frameId)
    {
        m_endFrameId = frameId;
    }

    int CVideoSegmentBase::GetKeyframeNum() const
    {
        return m_keyFrameList.Size();
    }

    const CFrame* CVideoSegmentBase::GetKeyframe(const int num) const
    {
        return m_keyFrameList.GetFrame(num);
    }

    CFrame* CVideoSegmentBase::GetKeyframe(const int num)
    {
        return m_keyFrameList.GetFrame(num);
    }

    bool CVideoSegmentBase::AddKeyframe(const CFrame& frame)
    {
        return m_keyFrameList.Add(frame);
    }

    bool CVideoSegmentBase::RemoveKeyframe(const CFrame& frame)
    {
        return m_keyFrameList.Remove(frame);
    }

    bool CVideoSegmentBase::RemoveKeyframe(const int frameId)
    {
        return m_keyFrameList.Remove(frameId);
    }
	
	void CVideoSegmentBase::RemoveAllKeyFrame()
	{
		return m_keyFrameList.Clear();
	}

    int CVideoSegmentBase::GetChildrenNum() const
    {
        return m_childrenList.GetSegmentNum();
    }
    
    const CVideoSegmentList &CVideoSegmentBase::GetChildren()const
    {
         return m_childrenList;
    }

    const IVideoSegment* CVideoSegmentBase::GetChild(const int num) const
    {
        return m_childrenList.GetSegment(num);
    }

    IVideoSegment* CVideoSegmentBase::GetChild(const int num)
    {
        return m_childrenList.GetSegment(num);
    }

    bool CVideoSegmentBase::AddChild(const IVideoSegment& segment)
    {
        return m_childrenList.Add(segment);
    }

    bool CVideoSegmentBase::RemoveChild(const IVideoSegment& segment)
    {
        return m_childrenList.Remove(segment);
    }

    bool CVideoSegmentBase::RemoveChild(const int segmentId)
    {
        return m_childrenList.Remove(segmentId);
    }

	void CVideoSegmentBase::RemoveAllChild()
	{
	      m_childrenList.Clear();
	}
	
	bool CVideoSegmentBase::operator==(const IVideoSegment& rgh) const
    {
        return Id() == rgh.Id();
    }

    bool CVideoSegmentBase::operator<(const IVideoSegment& rgh) const
    {
        if(EndTime() < rgh.BeginTime() )
            return true;
        else
        {
            //video segment of the same level (such as shot level, scene level) has no overlapping
            _ASSERT(rgh.EndTime() <= BeginTime() );
            return false;
        }
    }

    void CVideoSegmentBase::Release()
    {
        delete this;
    }

    void CVideoSegmentBase::Copy(const CVideoSegmentBase& segment)
    {
		if(this == &segment)
			return;
        m_id = segment.m_id;
        m_beginTime = segment.m_beginTime;
        m_endTime = segment.m_endTime;
        m_beginFrameId = segment.m_beginFrameId;
        m_endFrameId = segment.m_endFrameId;
        m_keyFrameList = segment.m_keyFrameList;
        m_childrenList = segment.m_childrenList;

        return;
    }

#pragma endregion

#pragma region CFrame
	CFrame::CFrame(const int id)
		:CVideoSegmentBase(id), m_pFrameImage(NULL)
	{

	}

    CFrame::~CFrame()
	{
		delete m_pFrameImage;
	}

    CFrame::CFrame(const CFrame& rhv)
        :CVideoSegmentBase(rhv)
	{
		if(rhv.m_pFrameImage == NULL)
		{
			m_pFrameImage = NULL;
			return;
		}
		else
		{
			m_pFrameImage = new CRgbImage();
			*m_pFrameImage = *rhv.m_pFrameImage;
		}
	}

    CFrame& CFrame::operator=(const CFrame& rhv)
	{
		if(this->m_pFrameImage == rhv.m_pFrameImage)
			return *this;
        CVideoSegmentBase::operator =(rhv);
		if(rhv.m_pFrameImage == NULL)
		{
			delete m_pFrameImage;
			m_pFrameImage = NULL;
			return *this;
		}
		else
		{
			delete m_pFrameImage;
			m_pFrameImage = new CRgbImage();
			*m_pFrameImage = *rhv.m_pFrameImage;
			return *this;
		}
	}

	void CFrame::Copy(const CFrame& rhv)
	{
		if(this == &rhv)
			return;
        CVideoSegmentBase::Copy(rhv);
		if(rhv.m_pFrameImage == NULL)
		{
			delete m_pFrameImage;
			m_pFrameImage = NULL;
			return;
		}
		else
		{
            delete m_pFrameImage;
			m_pFrameImage = new CRgbImage(); //we suppose the new operator is always ok
			if(FAILED(m_pFrameImage->Copy(*rhv.m_pFrameImage)))
				throw video_segment_exception("the frame image can't be copied");
		}
	}

    int CFrame::FrameId() const
    {
        _ASSERT(BeginFrameId() == EndFrameId());
        return BeginFrameId();
    }

    void CFrame::FrameId(const int frameId)
    {
           //for cframe, the id, bgnframeid, endframeid is the same
           CVideoSegmentBase::Id(frameId);
           CVideoSegmentBase::BeginFrameId(frameId);
		   CVideoSegmentBase::EndFrameId(frameId);
    }
        
    void CFrame::AttachImage(CRgbImage* pimg)
	{
		_ASSERT(pimg != NULL);
		if(m_pFrameImage == NULL)
			m_pFrameImage = new CRgbImage();
		*m_pFrameImage = *pimg;
	}
    
    void CFrame::CopyImage(const IRgbImage* pimg)
	{
		_ASSERT(pimg != NULL);
		if(m_pFrameImage == NULL)
			m_pFrameImage = new CRgbImage(); //we suppose new operator is always OK
        if(FAILED(m_pFrameImage->Copy(*pimg)))
			throw video_segment_exception("the frame image can't be copied");
	}
    
    const CRgbImage* CFrame::GetImage() const
	{
		return m_pFrameImage;
	}
    
    CRgbImage* CFrame::GetImage()
	{
		return m_pFrameImage;
	}

    void CFrame::ReleaseImage()
    {
          delete m_pFrameImage;
          m_pFrameImage = NULL;
    }

    void CFrame::Id(const int id)
    {
           //for cframe, the id, bgnframeid, endframeid is the same
           CVideoSegmentBase::Id(id);
           CVideoSegmentBase::BeginFrameId(id);
		   CVideoSegmentBase::EndFrameId(id);
    }

    int CFrame::Id()const
    {
            return CVideoSegmentBase::Id();
    }

	int CFrame::BeginFrameId() const 
	{
		    return CVideoSegmentBase::BeginFrameId();
	}

    void CFrame::BeginFrameId(const int frameId) 
	{
           //for cframe, the id, bgnframeid, endframeid is the same
           CVideoSegmentBase::Id(frameId);
           CVideoSegmentBase::BeginFrameId(frameId);
		   CVideoSegmentBase::EndFrameId(frameId);
	}

    int CFrame::EndFrameId() const
	{
		   return CVideoSegmentBase::EndFrameId();
	}

    void CFrame::EndFrameId(const int frameId) 
	{
           //for cframe, the id, bgnframeid, endframeid is the same
           CVideoSegmentBase::Id(frameId);
           CVideoSegmentBase::BeginFrameId(frameId);
		   CVideoSegmentBase::EndFrameId(frameId);
	}

	int CFrame::GetKeyframeNum() const 
	{
		return 1;
	}

    /*int CFrame::GetKeyframeId(const int num) const 
	{
		if( num >= 0 && num < GetKeyframeNum() )
            return this->Id();
		else
			return UNDEFINED_FRAME_ID;
	}*/

    const CFrame* CFrame::GetKeyframe(const int num) const
    {
        if( num >= 0 && num < GetKeyframeNum() )
            return this;
		else
			return NULL;
    }

    CFrame* CFrame::GetKeyframe(const int num)
    {
        if( num >= 0 && num < GetKeyframeNum() )
            return this;
		else
			return NULL;
    }

    bool CFrame::AddKeyframe(const CFrame& frame) 
	{
		return false;
	}

    bool CFrame::RemoveKeyframe(const CFrame& frame) 
	{
		return false;
	}

    bool CFrame::RemoveKeyframe(const int frameId) 
	{
		return false;
	}

	void CFrame::RemoveAllKeyFrame()
	{
	}

    int CFrame::GetChildrenNum() const 
	{
		return 0;
	}

    const CVideoSegmentList &CFrame::GetChildren()const
    {
        return CVideoSegmentBase::GetChildren();        
    }
    const IVideoSegment* CFrame::GetChild(const int num) const 
	{
		return NULL;
	}

    IVideoSegment* CFrame::GetChild(const int num) 
	{
		return NULL;
	}

    bool CFrame::AddChild(const IVideoSegment& segment)
	{
		return false;
	}

    bool CFrame::RemoveChild(const IVideoSegment& segment) 
	{
		return false;
	}

    bool CFrame::RemoveChild(const int segmentId)
	{
		 return false;
	}

	void CFrame::RemoveAllChild()
	{
	}

    CFrame* CFrame::Clone() const
    {
        return new CFrame(*this);
    }

    bool CFrame::operator==(const IVideoSegment& rgh) const
    {
        if(dynamic_cast<const CFrame*>(&rgh) == NULL)
            return false;
         return CVideoSegmentBase::operator==(rgh);
    }

    bool CFrame::operator<(const IVideoSegment& rgh) const
    {
        return CVideoSegmentBase::operator<(rgh);
    }
#pragma endregion

#pragma region CShot
    CShot::CShot(const int id)
		:CVideoSegmentBase(id), m_shotBoundaryType(SHOT_UNDEFINED)
    {}

    CShot::ShotBoundaryType CShot::GetShotBoundaryType() const
    {
        return m_shotBoundaryType;
    }

    void CShot::SetShotBoundaryType(const ShotBoundaryType t)
    {
        m_shotBoundaryType = t;
    }

    CShot* CShot::Clone() const
    {
        return new CShot(*this);
    }
#pragma endregion

#pragma region CScene
    CScene::CScene(const int id)
        :CVideoSegmentBase(id)
    {}

    CScene* CScene::Clone() const
    {
        return new CScene(*this);
    }
#pragma endregion

#pragma region CSubshot
    CSubshot::CSubshot(const int id)
        :CVideoSegmentBase(id),m_ShotId(-1)
    {}
    
    CSubshot* CSubshot::Clone() const
    {
        return new CSubshot(*this);
    }

    void CSubshot::ShotId(int Id)
    {
           m_ShotId = Id;
    }
    int CSubshot::ShotId()const
    {
          return m_ShotId;    
    }

#pragma endregion
}