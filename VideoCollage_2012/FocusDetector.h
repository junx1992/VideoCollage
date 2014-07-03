#include<Windows.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FOCUSDETECTOR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FOCUSDETECTOR_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef FOCUSDETECTOR_EXPORTS
#define FOCUSDETECTOR_API __declspec(dllexport)
#else
#define FOCUSDETECTOR_API __declspec(dllimport)
#endif

#define TOTALPIXELS 76800 // 320 * 240
#define SUCCESS_INIT 0
#define ERROR_RESOLUTION 1
#define ERROR_IMAGE_FORMAT 2

// This class is exported from the FocusDetector.dll
class FOCUSDETECTOR_API CFocusDetector
{
private:
	void* m_pAttentionExtractor;

public:
	CFocusDetector(void);
	~CFocusDetector(void);

public:


public:

	// Initialization before call any functions in this dll
	int Initialize(BYTE* pImage, int nWidth, int nHeight, int nStride, int nBits);
	// Release all memory allotted during all kinds of operations
	void DeInitialize();

	// import face detection result
	void SetFaceInformation(RECT* pFaceRect, int nFaceNum);

	// Extract attended view
	void ExtractAttendedView(RECT* AttView);
	// Extract attended Areas
	RECT* ExtractAttendedAreas(int* nNum);
	// Extract attended points
	POINT* ExtractAttendedPoints(int* nNum);

	// Extract saliency map
	float* ExtractSaliencyMap(int* nWidth, int* nHeight);
};


