#include "Stdafx.h"
#include <list>
#include <cmath>
#include <cassert>
#include <cfloat>
#include <numeric>
#include <algorithm>

#include "EntropyOfflineSceneDetector.h"
#include "VxDataSet.h"

namespace VideoAnalysisHelper
{
    using namespace std;
    using namespace VxCore;
    using namespace VideoAnalysis;

	template<typename Node>
	class CPtrList
	{
	public:
		typedef std::list<Node*>				_Container;
		typedef typename _Container::iterator	_Iterator;

	public:
		~CPtrList()
		{
			clear();
		}

		inline void resize(size_t size)
		{
			container.resize(size);
		}
		inline size_t size() const
		{
			return container.size();
		}
		inline _Iterator begin()
		{
			return container.begin();
		}
		inline _Iterator end()
		{
			return container.end();
		}
		inline void clear()
		{
			for ( _Iterator ite = container.begin(); ite != container.end(); ++ite )
            {
				delete (*ite);
            }
			container.clear();
		}
		inline void push_back(Node *node)
		{
			container.push_back(node);
		}
		inline void erase(_Iterator ite)
		{
			delete (*ite);
			container.erase(ite);
		}

	private:
		_Container	container;
	}; 

	static const int MaxNumOfFeature    = 256;         //the Max number of data feature array
    static const double s_dblMaxCost      = DBL_MAX;   //the default cost of node

	//********************************************************************************************************
	//	CSceneNode class for cluster
	//********************************************************************************************************
	class CSceneNode
	{
	public:
		CSceneNode(const CDoubleFeature & feature, int index);

		bool Merge(const CSceneNode *node);
		void SetCost(const CSceneNode *node);
		void SetCost(double cost);

	public:
		struct SceneNodeLess
		{
			bool operator()(const CSceneNode *op1, const CSceneNode *op2) const
			{
				_ASSERT(op1 && op2);
				return op1->m_dblCostOfFeature < op2->m_dblCostOfFeature;
			}
		};

	private:
		double Entropy(const double feature[], __in double sum);

	public: 
		static int m_s_uiNumOfFeature;

	public:
		int  m_rgIdxOfShots[2];                     //The shots index array
											        //Begin shot Id and end shot Id
		
	public:
		double	m_pFeature[MaxNumOfFeature];         //the feature data allocated outside, destroyed outside
		double  m_dblCostOfFeature;                  //the cost of feature data
		double  m_dblSumOfFeature;                   //the sum of feature data
	};

	int CSceneNode::m_s_uiNumOfFeature = 256;

	//********************************************************************************************************
	//	CSceneNode implementation
	//********************************************************************************************************
	CSceneNode::CSceneNode(const CDoubleFeature & feature,  int index) : 
		m_dblCostOfFeature(s_dblMaxCost)
	{
		   for( int i = 0; i < MaxNumOfFeature; ++i )
			      m_pFeature[i] = feature[i];
		
            //m_pFeature = feature;
		    m_dblSumOfFeature = accumulate(m_pFeature, m_pFeature + CSceneNode::m_s_uiNumOfFeature, 0.0);
		    m_rgIdxOfShots[0] = index;
		  	m_rgIdxOfShots[1] = index;
	}

	bool CSceneNode::Merge(const CSceneNode *node)
	{
		assert(node);

		//  Merge idx array
		if ( m_rgIdxOfShots[0] > node->m_rgIdxOfShots[0] )
			m_rgIdxOfShots[0] = node->m_rgIdxOfShots[0];
		else
			m_rgIdxOfShots[1] = node->m_rgIdxOfShots[1];
		
		for( int i = 0; i < CSceneNode::m_s_uiNumOfFeature; ++i )
			   m_pFeature[i] += (node->m_pFeature)[i];

        m_dblSumOfFeature += node->m_dblSumOfFeature;

		return true;
	}

	void CSceneNode::SetCost(__in double cost)
	{
		m_dblCostOfFeature = cost;
	}

	void CSceneNode::SetCost(__deref_in const CSceneNode *node)
	{
		assert(node);

		static double s_rgTempFeature[MaxNumOfFeature];

		//double *data1 = m_pFeature;
		//double *data2 = node->m_pFeature;
		for ( int i = 0; i < CSceneNode::m_s_uiNumOfFeature; ++i )
		{
			s_rgTempFeature[i] = m_pFeature[i] + (node->m_pFeature)[i];
		}
		double sum1 = m_dblSumOfFeature;
		double sum2 = node->m_dblSumOfFeature;
		double cost = Entropy(s_rgTempFeature, sum1 + sum2) 
			- Entropy(m_pFeature, sum1) 
			- Entropy((node->m_pFeature), sum2);
		m_dblCostOfFeature = cost < s_dblMaxCost ? cost : s_dblMaxCost;
	}

	double CSceneNode::Entropy(const double feature[], __in double sum)
	{
		double ret = sum * log(sum);
		for ( int i = 0; i < CSceneNode::m_s_uiNumOfFeature; ++i )
		{
			if( feature[i] != 0.0 )
				ret -= feature[i] * log(feature[i]);	
		}
		return ret;
	}
	//********************************************************************************************************
	//	CSceneNodeList class
	//********************************************************************************************************
	typedef CPtrList<CSceneNode> CSceneNodeList;

	/***************************************************************************************************\
	Function Description: 
		This function cluster shots into some Scene Nodes. 

	Arguments:
		[OUT]   CSceneNodeList&     SceneNodeList - when input, each shot in each scene node; after 
									cluster, some shots in each scene node.
		[IN]    size_t              uiMaxNumOfScenes - the number of scene nodes we expect

	Algoritm:
		1. calculate the cost of every adjacent nodes;
		2. choose and merge the nodes with the minimal cost;
		3. if the size of list after merge is still larger than uiMaxNumOfScenes, then seek the next pair 
		with the minimal cost, repeat 2,3 step.

	History:
		Modified on 05/25/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	static void ClusterShots(CSceneNodeList &list, size_t uiMaxNumOfScenes)
	{
		assert( uiMaxNumOfScenes > 0 );
		if ( uiMaxNumOfScenes == 0 )
			return;

		//	If there is the only one scene node in list, we needn't cluster anything, return immediately.
		assert( list.size() >= 1 );
		if ( list.size() == 1 ) 
			return;
		
		//	Calcuate the cost between each pair 
		CSceneNodeList::_Iterator ite_node = list.begin();
		CSceneNodeList::_Iterator ite_next = list.begin();
		for( ++ite_next; ite_next != list.end(); ++ite_node, ++ite_next )
			  (*ite_node)->SetCost( *ite_next );

		CSceneNodeList::_Iterator ite_best = min_element(list.begin(), list.end(), CSceneNode::SceneNodeLess());
		while ( list.size() >= uiMaxNumOfScenes )
		{
			//	Because the last node has the largest cost MAXCOST, so it mustn't be choosed as best node.
			//	If this conditional successed, then there must be something wrong with our algorithm!!!
			ite_next = ++ite_best;
			_ASSERT( ite_next != list.end() );
			ite_best--;
			CSceneNodeList::_Iterator ite_next_next = ++ite_next;
			ite_next--;
			//	Merge the best node with his next node, because of them with smallest cost
			(*ite_best)->Merge( *ite_next );
			//	Calculate the best node's new cost with his new next neighbour node. If it has no next node, then let its cost 
			//  a largest cost in order not to be selected to merge. And update the cost between prev node and best node
			if( ite_next_next != list.end() )
				(*ite_best)->SetCost( *ite_next_next );
			else
				(*ite_best)->SetCost( s_dblMaxCost );

			if( ite_best != list.begin() )
				(*(--ite_best))->SetCost( *ite_best );

            //	Then erase the best node's last pair
			list.erase( ite_next );
			//	Seek next minimal cost pair
			ite_best = min_element(list.begin(), list.end(), CSceneNode::SceneNodeLess());
		}
	}

	//********************************************************************************************************
	//	Maximal shots in one scene. In this application, we only use the first shot in the scene to estimate 
	//  the start time and stop time, so it is enough that the longest shot is selected to be saved into the 
	//  scene.
	//********************************************************************************************************
	/*#define	SHOT_MAXNUM_IN_SCENE 1*/
	#define	SHOT_MAXNUM_IN_SCENE 65535  //modified by linjuny@microsoft.com. Different from the motion thumbnail application, 
										//we want all shots in the scene. 65535 is a number large enough so that any scene 
										//has no so many shots.

	/***************************************************************************************************\
	Function Description: 
		This function chooses the top X longest shots (X is determined by uiMaxNumOfTopLongestShots)
		from the shots array (rgDurationOfShots). This function is internal function, only used by 
		CopyTopLongestShotsToScene function.

	Arguments:
		[OUT]   size_t  rgIdxOfTopLongestShots - an array to store the index of top longest shots in 
						rgShots array 
		[IN]    size_t  uiMaxNumOfTopLongestShots - the max number of output shots we expect
		[IN]    size_t  rgDurationOfShots - an array to store the duration of every shot
		[IN]    size_t  uiNumOfShots - the size of rgShots array

	Return Value:
		size_t - This is the size of rgTopLongestShots array.

	History:
		Modified on 05/21/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	static int ChooseTopLongestShots(
		int *IdxOfTopLongestShots, 
		int MaxNumOfTopLongestShots, 
		int *DurationOfShots, 
		int NumOfShots
		)
	{
		if ( NumOfShots <= MaxNumOfTopLongestShots )
		{
			//  If the amount of the input shots is less than the number of output we expect, 
			//  then we output all of shot indexes.
			for( int i = 0; i < NumOfShots; ++i )
				  IdxOfTopLongestShots[i] = i;

			return NumOfShots;
		}   else  	{
			//  If the amount of the input shots is larger than the number of output we expect,
			//  then we choose the top longest shots at first, and put their index into output array.
			//  Because uiNumOfShots and uiMaxNumOfTopLongestShots are not very large, we don't sort 
			//  rgShots array firstly.
			for( int i = 0; i < MaxNumOfTopLongestShots; ++i )
			{
				  int maxVal = 0;
				  int maxIdx = 0;
				  for( int j = 0; j < NumOfShots; ++j )
				  { 
					    if( DurationOfShots[j] > maxVal )
					    {
						    maxVal = DurationOfShots[j];
						    maxIdx = j;
					    }
			   	  }
				  //Since the elements of rgIdxOfTopLongestShots will be used as array index, 
				  //we check it valid here
				 _ASSERT( maxIdx < NumOfShots );
				 IdxOfTopLongestShots[i] = maxIdx;
				 DurationOfShots[maxIdx] = 0;
			}
			//after the above choosing method, the index array is in disorder, so we have to reorder them 
			//in ascend order. This order determines the show order of shots have been selected in the final trailer.
			sort(IdxOfTopLongestShots, IdxOfTopLongestShots + MaxNumOfTopLongestShots, less<int>());
			return MaxNumOfTopLongestShots;
		}
	}

	/***************************************************************************************************\
	Function Description: 
		This function is helper function, to help put the feature data of shots in array into scene 
		node list for cluster algorithm. 

		Here we don't make a copy of feature data, but use this buffer directly. Because this feature 
		buffer would be nouseful after cluster algorithm, we can write and read at buffer in cluster 
		algorithm as our will. But don't release this buffer in cluster algorithm, since feature class
		will manage this buffer by itself.

		This function is internal function, only be used by GroupShotsIntoSceneArray.

	Arguments:
		[OUT]   CSceneNodeList&     SceneNodeList - one list node stores only one shot after this function
		[IN]    vss::CShotArray&    ShotArray - an array to store shots, we will group them into scenes

	Return Value:
		HRESULT - If allocate node failed, then return E_OUTOFMEMORY

	History:
		Modified on 05/21/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	static HRESULT ShotArrayToSceneNodeList(
		    CSceneNodeList &SceneNodeList, 
		    const CVideoSegmentList &ShotArray,
		    const IDataSet &rfs
		)
	{
        if( rfs.GetSampleNum() <= 0 )
            return E_FAIL;

        int NumOfFeature = rfs.GetSample(0).Size();
		//  Since we use a double array as temp array to store feature data, we have to add a check
		//  whether uiNumOfFeature is larger than temp array length. If _ASSERT failed, we return
		//  E_OUTOFMEMORY.
		assert( NumOfFeature <= MaxNumOfFeature );
		if ( NumOfFeature > MaxNumOfFeature )
			 return E_OUTOFMEMORY;

        CSceneNode::m_s_uiNumOfFeature = NumOfFeature;

        SceneNodeList.clear();
		try
		{
            int NumOfShotArray = ShotArray.Size();
			for( int i = 0; i < NumOfShotArray; ++i )
			{
				  const CDoubleFeature * ptfe  = rfs.GetSampleById( ShotArray[i].GetKeyframe(0)->Id() );
				  if( ptfe == NULL )
				      return E_POINTER;
				

				  CSceneNode *node = NULL;
				  int index = ShotArray[i].Id();
				  SAFE_NEW_PTR(node, CSceneNode(*ptfe, index));
				  //LOG_ERROR_MEM_RET("Failed to allocate memory for node.", node);
				  SceneNodeList.push_back(node);    
			}
		}  catch(bad_alloc&) {
			return E_OUTOFMEMORY;
		}
		return S_OK;
	}

	/***************************************************************************************************\
	Function Description: 
		This function is helper function, to help make up Shots from scene node list to scene array.
		This function is internal function, only be used by GroupShotsIntoSceneArray.

	Arguments:
		[OUT]   vss::CSceneArray&   SceneArray - store scenes clusterd
		[IN]    vss::CShotArray&    ShotArray - an array to store shots, we will group them into scenes
		[IN]    CSceneNodeList&     SceneNodeList - ids array of scene node stores ShotArray's indexes 
									which belong to one scene

	History:
		Modified on 05/21/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	static HRESULT SceneNodeListToSceneArray(
		         CVideoSegmentList &SceneArray, 
		         const CVideoSegmentList &ShotArray, 
		         CSceneNodeList &SceneNodeList
		)
	{
		SceneArray.Clear();
		try
		{
			int id = -1;
			for ( CSceneNodeList::_Iterator  ite = SceneNodeList.begin(); ite != SceneNodeList.end(); ++ite )
			{
				CSceneNode *node = *ite;
				CScene Scene(++id);

				int index = node->m_rgIdxOfShots[0];
				while ( index <= node->m_rgIdxOfShots[1] )
				{	
					//Scene.shotArray.Add( ShotArray[index-1] );
					const IVideoSegment * pShot = ShotArray.GetSegmentById(index);
					if(pShot)
						Scene.AddChild( *pShot ); //modified by linjuny@microsoft.com
													   //subscript of shot array is not neccessary equal to id-1
					++index;
				}
				SceneArray.Add(Scene);
			}
		}
		catch(bad_alloc&)
		{
			return E_OUTOFMEMORY;
		}
		return S_OK;
	}

	/***************************************************************************************************\
	Function Description: 
		This function is the wrapper of function ChooseTopLongestShots. It use ChooseTopLongestShots to
		get the top longest shots, then put them into one scene.

	Arguments:
		[OUT]   CScene&        Scene - We will save the top longest shots into this scene
		[IN]      CVideoSegmentList&    ShotArray - an array to store shots, we will choose what we want from this array
		[IN]      int                   uiMaxNumOfShots - the amount of shots be selected we expect

	Return Value:
		HRESULT - If the size of input shots array (ShotArray) is zero, then result E_INVALIDARG. If
		any initialize operation failed, then return E_OUTOFMEMORY.

	History:
		Modified on 05/21/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	HRESULT CopyTopLongestShotsToScene(
		   CScene &Scene, 
		   const CVideoSegmentList & ShotList, 
		   int MaxNumOfShots
		)
	{
        //validate the size of array, if the size is zero, there is something wrong with the code outsize.
        int NumOfShots = ShotList.Size();
		assert( NumOfShots > 0 );
		if ( NumOfShots == 0 )
			return E_INVALIDARG;

		int  NumOfTopLongestShots  = 0;
		int * rgDurationOfShots       = NULL;
		int* rgIdxOfTopLongestShots  = NULL;
		
        //initialize the buffer for recording the duration of every shots
		SAFE_NEW_ARRAY(rgDurationOfShots, int, NumOfShots);
		//initialize the buffer for recording the index of top longest shots
		SAFE_NEW_ARRAY(rgIdxOfTopLongestShots, int, NumOfShots);
		//calculate every shot's duration by begin and end frame id
		for ( int i = 0; i < NumOfShots; ++i )
		{
               assert( ShotList[i].EndFrameId() >= ShotList[i].BeginFrameId() );
			   rgDurationOfShots[i] = ShotList[i].EndFrameId() - ShotList[i].BeginFrameId();
		}
		
        //choose the top longest shots according to duration value.
		NumOfTopLongestShots = ChooseTopLongestShots(rgIdxOfTopLongestShots, MaxNumOfShots, rgDurationOfShots, NumOfShots);
			
        //we validate rgIdxOfTopLongestShots size, for quickly find the fault of algorithm in debug mode
		_ASSERT( NumOfTopLongestShots <= MaxNumOfShots && NumOfTopLongestShots <= NumOfShots );
			
        //Create new scene consists with the shots we select.
		//CR3 : Add a initialize function to Scene.update( ... )
		//Resize before saving anything for efficient reason.
			
        //CShotArray& rsa = Scene.shotArray;
		for ( int i = 0; i < NumOfTopLongestShots; ++i )
               Scene.AddChild(ShotList[rgIdxOfTopLongestShots[i]]);


		SAFE_DELETE_ARRAY(rgDurationOfShots);
		SAFE_DELETE_ARRAY(rgIdxOfTopLongestShots);

		return S_OK;
	}

	/***************************************************************************************************\
	Function Description: 
		This function groups shots into some scenes, and ensures the number of scenes is less than
		uiMaxNumOfScenes.

	Arguments:
		[OUT]   vss::CSceneArray&   SceneArray - store the scenes which stores the longest shot in it
		[IN]    vss::CShotArray&    ShotArray - an array to store shots, we will group them into scenes
		[IN]    size_t              uiMaxNumOfScenes - the amount of scenes we expect after shots clustered

	Return Value:
		HRESULT - If the size of output scenes number we expected is zero, then return E_INVALIDARG.

	History:
		Modified on 05/21/2006 by hwei@microsoft.com
	          
	\***************************************************************************************************/
	HRESULT GroupShotsIntoSceneArray(
		CVideoSegmentList &SceneArray, 
		const CVideoSegmentList &ShotArray,
		const IDataSet &rfs,
		int MaxNumOfScenes
		)
	{
		HRESULT hr = S_OK;

		MaxNumOfScenes = min(ShotArray.Size(), MaxNumOfScenes);
		if( MaxNumOfScenes == 0 )	
			return E_INVALIDARG;
		else if ( MaxNumOfScenes == 1 )
		{
			//the outside only need one scene, the we choose the top SHOT_MAXNUM_IN_SCENE longest shots, then put them into scene
			CScene Scene(0);
			hr = CopyTopLongestShotsToScene(Scene, ShotArray, SHOT_MAXNUM_IN_SCENE);
			if( FAILED(hr) )
                return hr;
			
            SceneArray.Clear();
			SceneArray.Add(Scene);

		}  else  	{
			//	The outside need more than one scenes, so we cluster shots into uiMaxNumOfScenes scenes
			CSceneNodeList SceneNodeList;
			//  At first, we put shots into scene node list. Every scene node hold only one shot.
			hr = ShotArrayToSceneNodeList(SceneNodeList, ShotArray, rfs);
			if( FAILED(hr) )
                return hr;
			//  Cluster scene nodes. After clustering, scene node maybe hold some shots
			ClusterShots(SceneNodeList, MaxNumOfScenes);
			//	This must be always right!!! Because cluster algorithm never stops but the number of scene node list is 
			//  less than the number of scenes we expect.
			_ASSERT( SceneNodeList.size() < (size_t)MaxNumOfScenes ); 
			//	We use tempral scene array to store scene nodes from scene node list
			CVideoSegmentList OriginalSceneArray;
			hr = SceneNodeListToSceneArray(OriginalSceneArray, ShotArray, SceneNodeList);
			if( FAILED(hr) )
                return hr;
			int NumOfSceneArray = OriginalSceneArray.Size();
			//In our algorithm, when we create a motion thumbnail, we use the first shot's key frame in each scene as 
			//its motion clip's beginning frame, and shot's end as clip end. For keeping long enough clip in the first 
			//shot, we choose the longest shot as the first shot.
			try
			{
				SceneArray.Clear();
				for( int i = 0; i < NumOfSceneArray; ++i )
				{
					   CScene Scene(i);
					   hr = CopyTopLongestShotsToScene(Scene, OriginalSceneArray[i].GetChildren(), SHOT_MAXNUM_IN_SCENE);
			           if( FAILED(hr) )
                           return hr;
					   SceneArray.Add(Scene);
				}
			} catch(bad_alloc&) 	{
				      return E_OUTOFMEMORY;
			}
		}
		return S_OK;
	}
}

namespace VideoAnalysis
{
    using namespace VideoAnalysisHelper;

    CEntropyOfflineSceneDetector::CEntropyOfflineSceneDetector(const CVideoSegmentList & ShotList, const IDataSet & Rgb256Histo, int numOfScene)
        :m_ShotList(ShotList), m_RGB256Histogram(Rgb256Histo), m_NumOfScene(numOfScene)
    {}
   		
    const CVideoSegmentList& CEntropyOfflineSceneDetector::DetectSegmentOffline()
    {
             GroupShotsIntoSceneArray(m_SceneList, m_ShotList, m_RGB256Histogram, m_NumOfScene);
             return m_SceneList;
    }
    const CVideoSegmentList& CEntropyOfflineSceneDetector::GetScenes()const
    {
             return m_SceneList;
    }
}