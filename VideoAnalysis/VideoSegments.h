/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the video segment definition 

Notes:
  

History:
  Created on 04/28/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "RgbImage.h"
using ImageAnalysis::CRgbImage;
using ImageAnalysis::IImage;
using ImageAnalysis::IRgbImage;

//these are the implementation classes
namespace VideoAnalysisImp
{
    class CVideoSegmentListImp;
    class CFrameListImp;
    class CIdListImp;
}

namespace VideoAnalysis
{
    #define UNDEFINED_SEGMENT_ID -1
    
    class CFrame;
    class CVideoSegmentList;
    ///base class for all video segment, including frame, shot, subshot, scene.
    class DLL_IN_EXPORT IVideoSegment
    {
    public:
        ///get the segment id, id is all zero-based
        virtual int Id() const = 0 ;
        ///set the segment id
        virtual void Id(const int id) = 0 ;
        ///get the begine time of the segment. time is based on 100 ns
        virtual __int64 BeginTime() const = 0;
        ///set the begine time of the segment. time is based on 100 ns
        virtual void BeginTime(const __int64 time) = 0;
        ///get the end time of the segment. time is based on 100 ns
        virtual __int64 EndTime() const = 0;
        ///set the end time of the segment. time is based on 100 ns
        virtual void EndTime(const __int64 time) = 0;
        ///get the begine frame id of the segment
        virtual int BeginFrameId() const = 0;
        ///set the begine frame id of the segment
        virtual void BeginFrameId(const int frameId) = 0;
        ///get the end frame id of the segment
        virtual int EndFrameId() const = 0;
        ///set the end frame id of the segment
        virtual void EndFrameId(const int frameId) = 0;
        ///get keyframe number
        virtual int GetKeyframeNum() const = 0;
        ///get keyframe based on the number ( 0 to GetKeyframeNum()-1)
        virtual const CFrame* GetKeyframe(const int num) const = 0;
        ///get keyframe based on the number ( 0 to GetKeyframeNum()-1)
        virtual CFrame* GetKeyframe(const int num) = 0;
        ///add keyframe
        ///return value: true-success, false-the operation is forbidden
        virtual bool AddKeyframe(const CFrame& frame) = 0;
        ///remove keyframe 
        ///return value: true-success, false-the frame doesn't exist in the segment
        virtual bool RemoveKeyframe(const CFrame& frame) = 0;
        ///remove the keyframe by frame id
        ///return value: true-success, false-the frame doesn't exist in the segment
        virtual bool RemoveKeyframe(const int frameId) = 0;
        ///get children number
        virtual int GetChildrenNum() const = 0;
        ///get children list
        virtual const CVideoSegmentList &GetChildren()const = 0;
        ///get child segment based on the number ( 0 to GetChildrenNum()-1)
        virtual const IVideoSegment* GetChild(const int num) const = 0;
        virtual IVideoSegment* GetChild(const int num) = 0;
        ///add child
        ///return value: true-success, false-the operation is forbidden
        virtual bool AddChild(const IVideoSegment& segment) = 0;
        ///remove child 
        ///return value: true-success, false-the segment doesn't exist in the segment
        virtual bool RemoveChild(const IVideoSegment& segment) = 0;
        ///remove the child by segment id
        ///return value: true-success, false-the segment doesn't exist in the segment
        virtual bool RemoveChild(const int segmentId) = 0;
        ///clone self
        virtual IVideoSegment* Clone() const = 0;
        ///release self if the object is created by Clone();
        virtual void Release() = 0;
        ///equality
        virtual bool operator==(const IVideoSegment& rhv) const = 0;
        ///less than
        virtual bool operator<(const IVideoSegment& rhv) const = 0;
    public:
        ///pure virtual destructor
        virtual ~IVideoSegment() = 0 
        { }
    };

    ///segment list
    ///the segment list hold a copy for every segment
    class DLL_IN_EXPORT CVideoSegmentList
    {
    public:
        ///constructor
        CVideoSegmentList();
        CVideoSegmentList(const CVideoSegmentList& r);
        CVideoSegmentList& operator=(const CVideoSegmentList& r);
        ~CVideoSegmentList();
    public:
        ///add a segment to the list
        ///return value: true-success, false-the segment to be added exists in the list
        ///BE SURE THAT the pointer muse be valid as long as the lifetime of the segment list
        bool Add(const IVideoSegment& pSegment);
        ///remove a segment from the list
        ///true-success, false-the frame doesn't exist in the list
        bool Remove(const IVideoSegment& pSegment);
        //remove a segment from the list based on the segment id
        ///true-success, false-the frame doesn't exist in the list
        bool Remove(const int segmentId);
		///remove all the segment from the list
		void Clear();
        ///get segment number
        int GetSegmentNum() const;
        ///get segment number
        int Size() const;
        ///get segment based on the number ( 0 to GetSegmentNum()-1)
        const IVideoSegment* GetSegment(const int num) const; 
        ///get segment based on the number ( 0 to GetSegmentNum()-1)
        IVideoSegment* GetSegment(const int num);
        ///get segment based on segment id
        const IVideoSegment* GetSegmentById(const int segmentId) const; 
        ///get segment based on segment id
        IVideoSegment* GetSegmentById(const int segmentId);
        ///get segment based on the number ( 0 to GetSegmentNum()-1)
        IVideoSegment& operator[](const int num);
        ///get segment based on the number ( 0 to GetSegmentNum()-1)
        const IVideoSegment& operator[](const int num) const;
    private:
        VideoAnalysisImp::CVideoSegmentListImp* m_pImp;
    };

    ///frame list
    ///the frame list hold a copy for every added frames
    class DLL_IN_EXPORT CFrameList
    {
    public:
        ///constructor
        CFrameList();
        CFrameList(const CFrameList& r);
        CFrameList& operator=(const CFrameList& r);
        ~CFrameList();
        ///add a frame to the list
        ///return value: true-success, false-the frame to be added exists in the list
        bool Add(const CFrame& pFrame);
        ///remove a frame from the list
        ///true-success, false-the frame doesn't exist in the list
        bool Remove(const CFrame& pFrame);
        ///remove a frame from the list based on the frame id
        ///true-success, false-the frame doesn't exist in the list
        bool Remove(const int frameId);
        ///remove all frame from the list
		void Clear();
        ///get frame number
        int GetFrameNum() const;
        ///get frame number
        int Size() const;
        ///get frame based on the number( 0 to GetFrameNum()-1)
        const CFrame* GetFrame(const int num) const; 
        ///get frame based on the number( 0 to GetFrameNum()-1)
        CFrame* GetFrame(const int num);
        ///get frame based on the frame id
        const CFrame* GetFrameById(const int frameId) const; 
        ///get frame based on the frame id
        CFrame* GetFrameById(const int frameId);
        ///get frame based on the number( 0 to GetFrameNum()-1)
        CFrame& operator[](const int num);
        ///get frame based on the number( 0 to GetFrameNum()-1)
        const CFrame& operator[](const int num) const;
    private:
        VideoAnalysisImp::CFrameListImp* m_pImp;
    };

    class DLL_IN_EXPORT CIdList
    {
    public:
        ///constructor
        CIdList();
        CIdList(const CIdList& r);
        CIdList& operator=(const CIdList& r);
        ~CIdList();
        ///add a frame to the list
        ///return value: true-success, false-the id to be added exists in the list
        bool Add(const int Id);
        ///remove a frame from the list
        ///true-success, false-the id doesn't exist in the list
        bool Remove(const int Id);
        ///get the size
        int Size() const;
        ///get id based on the number( 0 to Size()-1)
        int GetId(const int num) const;
        ///get id based on the number( 0 to GetFrameNum()-1)
        int operator[](const int num) const;
    private:
        VideoAnalysisImp::CIdListImp* m_pImp;
    };
    ///base class for all VideoSegment.
    class DLL_IN_EXPORT CVideoSegmentBase: public IVideoSegment
    {
    public:
        CVideoSegmentBase(const int id = UNDEFINED_SEGMENT_ID);
        //CVideoSegmentBase(const CVideoSegmentBase& r);
        CVideoSegmentBase& operator=(const CVideoSegmentBase& r);
        //~CVideoSegmentBase();
    public:
        virtual int Id() const;
        virtual void Id(const int id);
        virtual __int64 BeginTime() const;
        virtual void BeginTime(const __int64 time);
        virtual __int64 EndTime() const;
        virtual void EndTime(const __int64 time) ;
        virtual int BeginFrameId() const ;
        virtual void BeginFrameId(const int frameId);
        virtual int EndFrameId() const;
        virtual void EndFrameId(const int frameId);
        virtual int GetKeyframeNum() const;
        virtual const CFrame* GetKeyframe(const int num) const;
        virtual CFrame* GetKeyframe(const int num);
        virtual bool AddKeyframe(const CFrame& frame);
        virtual bool RemoveKeyframe(const CFrame& frame);
        virtual bool RemoveKeyframe(const int frameId);
		virtual void RemoveAllKeyFrame();
        virtual int GetChildrenNum() const;
        virtual const CVideoSegmentList &GetChildren()const;
        virtual const IVideoSegment* GetChild(const int num) const;
        virtual IVideoSegment* GetChild(const int num);
        virtual bool AddChild(const IVideoSegment& segment);
        virtual bool RemoveChild(const IVideoSegment& segment);
        virtual bool RemoveChild(const int segmentId);
		virtual void RemoveAllChild();
        /*///set an external FrameList so that the segment of one video can share one frame list for the memory
        ///efficiency.
        ///BE SURE THAT the lifetime of the frame list is longer than or equal to the segment.
        ///BE SURE TAHT the function call must be made before the first use of frame list (such as add a frame),
        ///or else an exception will happen.
        virtual void SetFrameList(CFrameList* pFrameList);*/
        ///equality
        bool operator==(const IVideoSegment& rgh) const;
        bool operator<(const IVideoSegment& rgh) const;
        virtual IVideoSegment* Clone() const = 0;
        virtual void Release();
        void Copy(const CVideoSegmentBase& segment);
    private:
        int m_id;
        __int64 m_beginTime;
        __int64 m_endTime;
        int m_beginFrameId;
        int m_endFrameId;
        CFrameList m_keyFrameList;
        CVideoSegmentList m_childrenList;
    };

    ///Frame Class.
	///The copy constructor and assignment DO NOT copy the image.
	///If you want the image also copied please call CFrame:;Copy() function.
    class DLL_IN_EXPORT CFrame: public CVideoSegmentBase
    {
    public:
		///default constructor
        CFrame(const int id = UNDEFINED_SEGMENT_ID);
        ~CFrame();
        ///copy constructor.
        ///the image is NOT copied. 
		///exception: video_segment_exception
        CFrame(const CFrame&);
        ///copy assignment
        ///the image is NOT copied
        CFrame& operator=(const CFrame&);
    public:
        ///attach to a image, no copy
        virtual void AttachImage(CRgbImage* pimg);
		///copy the frame
        ///if the frame can NOT be copied, a video_segment_exception exception may be thrown
		virtual void Copy(const CFrame& rhv);
        ///keep a copy of the image.
        ///if the image can't be copied, a video_segment_exception exception may be thrown
        virtual void CopyImage(const IRgbImage* pimg);
        ///get the image of this frame
        virtual const CRgbImage* GetImage() const;
        ///release the image of this frame
        void ReleaseImage();
        ///get the image of this frame
        virtual CRgbImage* GetImage();
        ///copy the corresponding information, no copy the image
        ///similar to "return new CFrame(*this);"
        virtual CFrame* Clone() const;
        bool operator==(const IVideoSegment& rgh) const;
        bool operator<(const IVideoSegment& rgh) const;
        ///get and set the Id
        virtual void Id(const int id);
        virtual int Id()const;
	private:
        ///get the current frame id
        int FrameId() const;
        ///set the current frame id
        void FrameId(int frameId);
        ///the override virtual function
		virtual int BeginFrameId() const ;
        virtual void BeginFrameId(const int frameId) ;
        virtual int EndFrameId() const ;
        virtual void EndFrameId(const int frameId) ;
		virtual int GetKeyframeNum() const ;
        virtual const CFrame* GetKeyframe(const int num) const;
        virtual CFrame* GetKeyframe(const int num);
        virtual bool AddKeyframe(const CFrame& frame) ;
        virtual bool RemoveKeyframe(const CFrame& frame) ;
        virtual bool RemoveKeyframe(const int frameId) ;
		virtual void RemoveAllKeyFrame();
        virtual int GetChildrenNum() const ;
        virtual const CVideoSegmentList &GetChildren()const;
        virtual const IVideoSegment* GetChild(const int num) const ;
        virtual IVideoSegment* GetChild(const int num) ;
        virtual bool AddChild(const IVideoSegment& segment) ;
        virtual bool RemoveChild(const IVideoSegment& segment) ;
        virtual bool RemoveChild(const int segmentId);
		virtual void RemoveAllChild();
    private:
        CRgbImage* m_pFrameImage;
    };

    ///shot class
    class DLL_IN_EXPORT CShot: public CVideoSegmentBase
    {
    public:
        ///shot boundary type enumeration.
        ///To determine whether a shot is of the specified type:<br/>
        /// -ShotType t; <br/>
        /// -if(t&SHOT_GRADUAL)
        enum ShotBoundaryType
        {
            SHOT_UNDEFINED  = 0x0000,
            SHOT_CUT        = 0x1000,
            SHOT_WIPE       = 0x0001,
            SHOT_DISSOLVE   = 0x0002,
            SHOT_FADEIN     = 0x0004,
            SHOT_FADEOUT    = 0x0008,
            //SHOT_FADE       = 0x000c,
            SHOT_FADE       = SHOT_FADEIN | SHOT_FADEOUT,
            //SHOT_GRADUAL    = 0x000F
            SHOT_GRADUAL    = SHOT_WIPE | SHOT_DISSOLVE | SHOT_FADE
        };
    public:
        CShot(const int id = UNDEFINED_SEGMENT_ID);
    public:
        ///get the type of the shot boundary between this shot and the succedent shot
        ShotBoundaryType GetShotBoundaryType() const;
        void SetShotBoundaryType(const ShotBoundaryType t);
        virtual CShot* Clone() const;
    private:
        ShotBoundaryType m_shotBoundaryType;
    };

    ///scene class
    class DLL_IN_EXPORT CScene: public CVideoSegmentBase
    {
    public:
        CScene(const int id = UNDEFINED_SEGMENT_ID);
         virtual CScene* Clone() const;
    };

    ///sub shot class
    class DLL_IN_EXPORT CSubshot: public CVideoSegmentBase
    {
    public:
        CSubshot(const int id = UNDEFINED_SEGMENT_ID);
        virtual CSubshot* Clone() const;

        void ShotId(int Id);
        int ShotId()const;
   private:
        int m_ShotId; 
   };

}