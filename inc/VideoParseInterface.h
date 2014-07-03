/******************************************************************************\

Microsoft Research Asia
Copyright (c) 2007 Microsoft Corporation

Module Name:
  Video Analysis Sdk

Abstract:
  This header file provide the external interfaces for the video parser 

Notes:

History:
  Created on 05/09/2007 by linjuny@microsoft.com
\******************************************************************************/
#pragma once
#include "VxComm.h"
#include "VideoSegments.h"
#include "VxDataSet.h"

namespace VideoAnalysis
{
    class IVideoParseReceiver;

    ///interface of video parser, act as base of IVideoSegmentDetector
    class __declspec(novtable) IVideoParseController
    {
    public:
        ///add a receiver
        virtual HRESULT AddReceiver(IVideoParseReceiver* const) = 0;
        ///remove a receiver.
        ///if the receiver exist in the receiver list, S_OK returned; else S_FALSE returned.
        virtual HRESULT RemoveReceiver(const IVideoParseReceiver* const) = 0;
		///the virtual destructor, it's important
		virtual ~IVideoParseController()=0{}
    };

    //// {4FDCCD5F-F4EE-48c1-B87A-C2D6DB612A25}
    //DEFINE_GUID(IID_IVideoParseReceiver, 
    //0x4fdccd5f, 0xf4ee, 0x48c1, 0xb8, 0x7a, 0xc2, 0xd6, 0xdb, 0x61, 0x2a, 0x25);
    ///interface for video parse receiver
    class __declspec(novtable) IVideoParseReceiver
    {
    public:
        ///notified when a new segment is detected (frame is also a kind of segment).
        ///if some unexpected errors happen, return E_FAIL. else return S_OK
        virtual HRESULT OnNewSegment(IVideoSegment& segment) = 0;
        /*/// Notify that the stream is starting
	    virtual HRESULT StartStreaming() = 0;
        /// Notify that the stream is stopped
	    virtual HRESULT StopStreaming() = 0;*/
	    /// Notify that the stream is ending
	    virtual HRESULT EndOfStream() = 0;
		/*///this member function is called when an controller tries to add the receiver
		virtual HRESULT CheckHost(IVideoParseController& host) = 0;*/
		///the virtual destructor, it's important
		virtual ~IVideoParseReceiver()=0{}
    };

	///interface for keyframe extractor.
	/**
	when using Keyframe extractor, you should ensure it can receive frame and the segment).
	*/
	class __declspec(novtable) IKeyframeExtractor
	{
	public:
        ///called when there is a new segment, and it will return a key frame list,
        /// *important* the returned key frame list will be invalidate when next call to OnNewSegment
		virtual CFrameList & OnNewSegment(IVideoSegment& segment) = 0;
        ///inform the extractor there is a new frame and whether it can be a key frame candidate
		virtual HRESULT OnNewFrame(CFrame& frame, bool isKeyframeCandidate) = 0;
		///the virtual destructor, it's important
		virtual ~IKeyframeExtractor()=0{}
	};

    //// {493C1D1A-5A87-4da9-AA9C-18A73BB9BD71}
    //DEFINE_GUID(IID_IVideoParseDetector, 
    //0x493c1d1a, 0x5a87, 0x4da9, 0xaa, 0x9c, 0x18, 0xa7, 0x3b, 0xb9, 0xbd, 0x71);
    ///interface for video segment detector, such as shot detector
    class __declspec(novtable) IVideoSegmentDetector: public IVideoParseReceiver, public IVideoParseController
    {
    };

	///interface for feature extractor
	class __declspec(novtable) IVideoFeatureExtractor: public IVideoParseReceiver
    {
    public:
		///get the extracted feature sequence.
		virtual const VxCore::IDataSet& GetData() const = 0;
    };

	///interface for shot detector.
	class __declspec(novtable) IFrameDetector: public IVideoSegmentDetector
	{};

	///interface for shot detector.
	class __declspec(novtable) IShotDetector: public IVideoSegmentDetector
	{
	public:
		///get the information of all the detected shots.
		///NOTE: only the id of the keyframe is validate, and others of the shot is not available.
		virtual const CVideoSegmentList& GetShots() const = 0;
	};

	///interface for sub shot detector
	class __declspec(novtable) ISubshotDetector: public IVideoSegmentDetector
	{
	public:
		///get the information of all the detected subshots.
		///NOTE: only the id of the keyframe is validate, and others of the shot is not available.
		virtual const CVideoSegmentList& GetSubshots() const = 0;
	};

	///interface for scene detector
	class __declspec(novtable) ISceneDetector: public IVideoSegmentDetector
	{
	public:
		///get the information of all the detected scenes.
		///NOTE: the keyframe in the scene is not available.
		virtual const CVideoSegmentList& GetScenes() const = 0;
	};

	///interface for offline video segment detector
	class __declspec(novtable) IOfflineSegmentDetector
	{
	public:
		virtual const CVideoSegmentList& DetectSegmentOffline() = 0;
		virtual ~IOfflineSegmentDetector()=0{}
	};

    ///interface for offline video scene detector
    class __declspec(novtable) IOfflineSceneDetector: public IOfflineSegmentDetector
	{
	public:
         virtual const CVideoSegmentList& GetScenes() const = 0;
	};        

    ///interface for offline video scene detector
    class __declspec(novtable) IOfflineSubshotDetector: public IOfflineSegmentDetector
	{
	public:
         virtual const CVideoSegmentList& GetSubshots() const = 0;
	};        
}
