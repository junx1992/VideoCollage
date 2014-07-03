#include "Stdafx.h"
#include "SimpleThumbNailExtractor.h"
#include "VideoSegments.h"
#include "VideoEditService.h"
#include "VxUtils.h"
#include <vector>
#include <algorithm>
#include <cassert>

namespace VideoAnalysis
{
    using namespace std;
    using namespace VxCore;

	inline double GetDurationOfClip(double dblDurationOfVideo, unsigned int uiNumOfClips, double dblDurationOfTrans)
	{
		if ( uiNumOfClips == 0 ) 
			return 0.0;
		
        //double duration = dblDurationOfTrans + (dblDurationOfVideo - dblDurationOfTrans) / uiNumOfClips;
        //by miaohua do changes to calculation of duration
        double duration = dblDurationOfVideo / uiNumOfClips - dblDurationOfTrans/2;
		//	If the entire duration is less than transition duration, then we at least output one clip of c_dblDurationOfClipInSec length
		// return (dblDurationOfVideo < dblDurationOfTrans) ? VideoEditServiceConfig::m_DurationOfClipInSec : duration;
        // by miaohua duration must be more than zero
        return ((dblDurationOfVideo < dblDurationOfTrans) || duration <= 0)? VideoEditServiceConfig::m_DurationOfClipInSec : duration;
    }

    //get intervals according to scenelist
    void GetIntervals(const CVideoSegmentList & SceneList, VideoEditServiceConfig & config, vector<Interval> & intervals, double thumbnailDuaration)
    {

        if( thumbnailDuaration >= config.m_VideoInfo.GetDuration() )
		{
            double llStop = config.m_VideoInfo.GetDuration();
			intervals.push_back( Interval(0, llStop) );
            config.m_OutputVideoEffect = false;
		}  else {
            config.m_OutputVideoEffect = true;
            int size = SceneList.Size();
			vector<const CShot*> pLongestShotArr(size);
            
            //find the longest shot for each scene
			for( int i = 0; i < size; ++i )
			{
                  int shotNum = SceneList[i].GetChildrenNum();
                  if( shotNum == 0 ) continue;
                      
                  pLongestShotArr[i] = dynamic_cast<const CShot*>(SceneList[i].GetChild(0));
				  for( int j = 1; j < shotNum;  ++j )
				  {
					    __int64 shotLen = pLongestShotArr[i]->EndTime() - pLongestShotArr[i]->BeginTime();
                        __int64 curShotLen = SceneList[i].GetChild(j)->EndTime() - SceneList[i].GetChild(j)->BeginTime();
					    if( shotLen <  curShotLen )
					        pLongestShotArr[i] = dynamic_cast<const CShot*>(SceneList[i].GetChild(j));
				  }
			}

			//	At first we should calculate the transtion duration. The transition duration *2 must be less than
			//	the shortest shot, else neighbour shots in the same track will be overlaped to crash DES.
			//	So we get the shortest shot duration, then set transtion duration is one third of this shot duration.
			double dblDurationOfTrans = 0;
            dblDurationOfTrans = VideoEditServiceConfig::m_DurationOfTransInSec * 3;
            // get the shortest shot during the longest shots
			for ( size_t i = 0; i < pLongestShotArr.size(); ++i )
			{			
				double t = (pLongestShotArr[i]->EndTime() - pLongestShotArr[i]->BeginTime())*1.0/1e+7;
				if( t < dblDurationOfTrans )
					dblDurationOfTrans = t;
			}                     

            //set transtion duration is one third of this shot duration
			dblDurationOfTrans /= 3;
			config.m_ThumbTransDuarationInSec = dblDurationOfTrans/2;
			
            //get the duration for each clip
			double dblDurationOfClip = GetDurationOfClip(thumbnailDuaration, SceneList.Size(), dblDurationOfTrans);

			for( size_t i = 0; i < pLongestShotArr.size(); ++i )
			{
				//	We use the keyframe of the current scene's start time as clip's start time, and the duration is the sum 
				//	of clip duration and transtion duration. If the end of this clip exceeds the end of this shot, then we
				//	move this clip forward to the beginning of shot. After moving, if the start of this clip is less than
				//	the start of this shot, the we cut off the over part.
				
                double dblStart		= pLongestShotArr[i]->GetKeyframe(0)->BeginTime()*1.0/1e+7;
				double dblDuration	= dblDurationOfClip + dblDurationOfTrans;
				double dblStop		= dblStart + dblDuration;
				double dblShotStop	= pLongestShotArr[i]->EndTime()*1.0/1e+7;
				double dblShotStart	= pLongestShotArr[i]->BeginTime()*1.0/1e+7;
				if( dblStop > dblShotStop )
				{
					dblStart = dblShotStop - dblDuration;
					dblStop = dblShotStop;
				}
				
                if( dblStart < dblShotStart )
					dblStart = dblShotStart;

				Interval iv(dblStart, dblStop);
				intervals.push_back(iv);
			}

			sort( intervals.begin(), intervals.end() );
		}	
    }
    
    Interval GetIntervalFromShot(const CShot & shot, double thumbnailDuaration)
    {
		double dblShotStart	= shot.BeginTime()*1.0/1e+7;
		double dblShotStop	= shot.EndTime()*1.0/1e+7;
        double dblStart		= shot.GetKeyframe(0)->BeginTime()*1.0/1e+7;
		double dblDuration	= thumbnailDuaration ;
		double dblStop      = 0.0;	
		
		if( (dblShotStop - dblStart) <= 0.04 ) //if the keyframe is the last frame
			dblStart = dblShotStart;
		
		if( (dblShotStop - dblStart) < thumbnailDuaration )
			 return Interval(dblStart, dblShotStop);
		else {
			dblDuration	= thumbnailDuaration ;
			dblStop		= dblStart + dblDuration;
			
			if( dblStop > dblShotStop )
			{
				dblStart = dblShotStop - dblDuration;
				dblStop = dblShotStop;
			}

			if( dblStart < dblShotStart )
				dblStart = dblShotStart;    

			return Interval(dblStart, dblStop);
		}
    }

    HRESULT CSimpleVideoThumbnailExtractor::ExtractVideoMotionThumbnailToFile(const CVideoInfo & videoInfo,
                                                                  const FileInfo & fileInfo,
												                  const CVideoSegmentList & sceneList,
												                  WCHAR* thumbnailFileName,
												                  double thumbnailDuaration,
  												                  WCHAR* profileFileName,
												                  bool bOutputAudio,
                                                                  ProgressHandler handler
												                  )
	{

		VideoEditServiceConfig config(videoInfo, fileInfo, bOutputAudio, true);
		
		//_config.SetVideoMotionThumbnailDuration(thumbnailDuaration);
		if( profileFileName != NULL )
			config.SetProfileByFile(profileFileName);
		config.SetMotionFileName(thumbnailFileName);

        //get intervals
		vector<Interval> intervals;
        GetIntervals(sceneList, config, intervals, thumbnailDuaration);
		if( intervals.size() == 0 )
            return E_FAIL;

        //do the video edit
		CVideoEditService composer;

        //build filter graph
        HRESULT hr = composer.BuildGraph(config, intervals);
		if( hr != S_OK )
			return hr;

        return composer.Save(INFINITE, handler);
	}

    HRESULT  CSimpleVideoThumbnailExtractor::ExtractVideoMotionThumbnailToPreview(const CVideoInfo & videoInfo,
                                                const FileInfo & fileInfo,
												const CVideoSegmentList & sceneList,
												double thumbnailDuaration,
												WCHAR* profileFileName,
												bool bOutputAudio,
                                                ProgressHandler handler 
												)
    {
        VideoEditServiceConfig config(videoInfo, fileInfo, bOutputAudio, true);
		
		//_config.SetVideoMotionThumbnailDuration(thumbnailDuaration);
		if( profileFileName != NULL )
			config.SetProfileByFile(profileFileName);

        //get intervals
		vector<Interval> intervals;
        GetIntervals(sceneList, config, intervals, thumbnailDuaration);
		if( intervals.size() == 0 )
            return E_FAIL;
        
        //do the video edit
		CVideoEditService composer;

        //build filter graph
        HRESULT hr = composer.BuildGraph(config, intervals);
		if( hr != S_OK )
			return hr;

        return composer.Preview(INFINITE, handler);
    }

	HRESULT CSimpleShotThumbnailExtractor::ExtractShotMotionThumbnailToFile(const CVideoInfo & videoInfo,
                                                const FileInfo & fileInfo,
                                                const CShot & shot,
                                                WCHAR* thumbnailFileName,
												double thumbnailDuaration,
												WCHAR* profileFileName,
												bool bOutputAudio,
                                                ProgressHandler handler
												)	
	{
        assert( shot.GetKeyframeNum() > 0 );
		assert( (shot.GetKeyframe(0)->BeginTime() >= shot.BeginTime()) && (shot.GetKeyframe(0)->EndTime() <= shot.EndTime()));

		VideoEditServiceConfig config(videoInfo, fileInfo, bOutputAudio, false);
		
        if( profileFileName != NULL )
			config.SetProfileByFile(profileFileName);
		config.SetMotionFileName(thumbnailFileName);

        //get the interval from the shot
        vector<Interval> intervals;
        intervals.push_back(GetIntervalFromShot(shot, thumbnailDuaration));

        //save the motion thumb nail into a file
		CVideoEditService composer;

        //build filter graph
        HRESULT hr = composer.BuildGraph(config, intervals);
		if( hr != S_OK )
			return hr;

        return composer.Save(INFINITE, handler);
	}

    HRESULT  CSimpleShotThumbnailExtractor::ExtractShotMotionThumbnailToPreview(const CVideoInfo & videoInfo,
                                                const FileInfo & fileInfo,
												const CShot & shot,
												double thumbnailDuaration,
												WCHAR* profileFileName,
												bool bOutputAudio,
                                                ProgressHandler handler
										)
    {
        assert( shot.GetKeyframeNum() > 0 );
		assert( (shot.GetKeyframe(0)->BeginTime() >= shot.BeginTime()) && (shot.GetKeyframe(0)->EndTime() <= shot.EndTime()) );

		VideoEditServiceConfig config(videoInfo, fileInfo, bOutputAudio, false);
		
        if( profileFileName != NULL )
			config.SetProfileByFile(profileFileName);

        //get the interval from the shot
        vector<Interval> intervals;
        intervals.push_back(GetIntervalFromShot(shot, thumbnailDuaration));

        //save the motion thumb nail into a file
		CVideoEditService composer;

        //build filter graph
        HRESULT hr = composer.BuildGraph(config, intervals);
		if( hr != S_OK )
			return hr;

        return composer.Preview(INFINITE, handler);    
    }
  
}