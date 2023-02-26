#pragma once
#include <tchar.h>
#include "windows.h"
#include <iostream>
#include <dshowasf.h>
#include <dshow.h>
#include <thread>
#include <ctime>
#include "vfw.h"
#include <cstdio>
#include <gdiplus.h> 
#include <atlbase.h>
#include <atlstr.h>
#include<fstream>
#include<memory>
#include <map>
#include <vector>
#include "C:/Temp\Directshow_camera\directshow_camera\include\directshow_camera\qedit.h"

#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "Wmvcore.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib,"gdiplus.lib")
#import "qedit.dll" raw_interfaces_only named_guids
EXTERN_C const CLSID CLSID_NullRenderer;
EXTERN_C const CLSID CLSID_SampleGrabber;

enum CaptureType
{
	Image = 1,
	Video = 2
};
enum SynchronizMode
{
	Synch = 1,
	Asynch = 2
};

class ImageInformationsPackage
{
public:
	std::pair<std::string, std::string> pIDvID;
	int Height;
	int Width;
	LPCTSTR OutputFileName;
	LPCTSTR ImageLabel;
};


using namespace std;
using namespace Gdiplus;

class SampleGrabberCallback :public ISampleGrabberCB
{
public:
	// Fake out any COM ref counting
	//
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	SampleGrabberCallback();
	~SampleGrabberCallback();
	void setVideoText(LPCTSTR Text);
	inline const std::string currentTime();
	TCHAR* stringToTChar(string data);
	void setFrameSize(int frameHeight,int frameWidth);
public:
	// These will get set by the main thread below. We need to
	// know this in order to write out the bmp	
	STDMETHODIMP SampleCB(double n, IMediaSample* pSample);
	// The sample grabber is calling us back on its deliver thread.
	// This is NOT the main app thread!
	//
	STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferSize);
private:
	long width;
	long height;
	long framenum;
	CString strPath;
	//CString EngraveText;
	LPCTSTR EngraveText;
};

class DirectShowCamera
{
public:
	DirectShowCamera();
	bool RecordVideo(int CameraNumber, long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode, LPCTSTR EngraveText=NULL);
	bool RecordVideo(pair<string, string>& pIDvID, long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode, LPCTSTR EngraveText=NULL);
	bool TakeImage(int CameraNumber,int Height,int Width, LPCTSTR OutputFileName, LPCTSTR ImageLabel);
	bool TakeImage(pair<string, string>& pIDvID, int Height, int Width, LPCTSTR OutputFileName, LPCTSTR ImageLabel);	
	void openCamera(int Height, int Width);
	bool WriteOnImage(LPCTSTR ImageFileName, LPCTSTR EngraveText);
	bool IsAvailable(int CameraNumber);
	static int	GetAvailableCameraCount();
	std::map<string, string> getCameraNameList();
	bool setDeviceID(int ID);
	int getDeviceID();
	void Live(pair<string, string>DeviceUniquePath);
	DirectShowCamera(int CameraNumber, int CaptureType, LPCTSTR OutputFileName);
	bool Stop();
	void	setOutputFile(LPCTSTR fileName);
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	~DirectShowCamera();
private:
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	void* sgcb=NULL;
	SampleGrabberCallback SampleCallback;
	std::thread openerThread;
	bool isOpen=false;
	int DeviceID;
	pair<string, string> selectedCameraName;
	bool	inUse;
	HRESULT SelectCamera(pair<string, string>&DeviceUniquePath);
	HRESULT SelectCamera(int DeviceNumber);
	HRESULT ConfigFormat();
	void exit_message(const char* error_message, int error);
	int captureImage(LPCTSTR FileName, LPCTSTR EngraveText);
	int GetLiveFromCamera(long duration);
	bool StartRecordVideo(long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode = Asynch);
	static vector<string> split(string data, string delimiter);
	HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pFirst, IBaseFilter* pSecond);
	HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
	ICaptureGraphBuilder2 *pBuild = NULL;
	IGraphBuilder *pGraph = NULL;
	IBaseFilter *pCap = NULL;
	IVideoWindow *pWindow = NULL;
	IMoniker *pMoniker = NULL;
	IEnumMoniker *pEnum = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IMediaControl *pControl = NULL;
	IMediaEvent   *pEvent = NULL;   // methods for getting events from the Filter Graph Manager
	IBaseFilter*   pASFWriter = NULL;
	IBaseFilter*   pNullRenderer = NULL;
	IConfigAsfWriter *pConfigWM = NULL;
	ISampleGrabber *pSampleGrabber = NULL;
	IBaseFilter *pSampleGrabberFilter = NULL;
	IPropertyBag *pPropBag = NULL;
	long videoDuration;//Second
	LPCTSTR threadName;
	short captureType;
	char* pBuffer = NULL;
	HRESULT hr = 0;
};


