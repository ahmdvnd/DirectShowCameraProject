#include "stdafx.h"

#include "DirectShowCamera.h"
typedef map<string, string> CameraNameList;


DirectShowCamera::DirectShowCamera()
{
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}
HRESULT DirectShowCamera::SelectCamera(pair<string, string>& DeviceUniquePath)
{
	pair<string, string>item;
	bool cameraFound = false;
	if (CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum)) == S_OK)
	{
		if (pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0) == S_OK)
		{
			if (pEnum->Reset() == S_OK)
			{
				while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
				{
					std::string cameraFriendlyName;
					IPropertyBag* pPropBag;
					hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
					if (FAILED(hr))
					{
						continue;
					}

					VARIANT var;
					VariantInit(&var);
					hr = pPropBag->Read(L"FriendlyName", &var, 0);
					if (SUCCEEDED(hr))
					{
						std::wstring cameraFriendlyNameWs(var.bstrVal, SysStringLen(var.bstrVal));
						cameraFriendlyName = string(cameraFriendlyNameWs.begin(), cameraFriendlyNameWs.end());
					}
					VariantClear(&var);
					hr = pPropBag->Read(L"DevicePath", &var, 0);
					if (SUCCEEDED(hr))
					{
						cout << "DevicePath: " << "\n" << endl;
						std::wstring cameraPathRaw(var.bstrVal, SysStringLen(var.bstrVal));
						const std::string cameraPath(cameraPathRaw.begin(), cameraPathRaw.end());
						cout << "Camera :" << cameraPath << "\n" << endl;
						auto strlist = split((string)cameraPath, "#");
						cout << "strlist :" << strlist.size() << "\n" << endl;
						if (strlist.size() > 1)
						{

							auto strlist2 = split(strlist[1], "&");
							if (strlist2.size() > 1)
							{
								cout << "strlist2 :" << strlist2.size() << "\n" << endl;
								item = make_pair(cameraPath, strlist2[0] + strlist2[1]);
								cout << "Final Name :" << item.first << "\n" << endl;
							}
							else
							{
								cout << "Error in CameraName 2" << endl;
							}

						}
						else
						{
							cout << "Error in CameraName 1" << endl;
						}
					}
					VariantClear(&var);
					pPropBag->Release();
					if (item.first == DeviceUniquePath.first)
					{
						cameraFound = true;
						selectedCameraName = move(DeviceUniquePath);
						break;
					}
				}
			}
			else
				return -1;
		}
		else
			return -3;
	}
	else
		return -4;

	if (!cameraFound)
		return -2;
	//	LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "SelectCamera 2");
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
	return hr;
}

HRESULT DirectShowCamera::SelectCamera(int CameraNumber = 1)
{
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	if (pEnum == NULL)
	{
		return -1;
	}
	hr = pEnum->Reset();
	for (int i = 0; i < CameraNumber; i++)
	{
		hr = pEnum->Next(1, &pMoniker, NULL);
	}

	VARIANT var;
	VariantInit(&var);
	hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
	if (FAILED(hr))
		return hr;
	hr = pPropBag->Read(L"DevicePath", &var, 0);
	if (FAILED(hr))
		return hr;
	//LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "SelectCamera1 2");

	return pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
}

bool DirectShowCamera::IsAvailable(int CameraNumber)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ICreateDevEnum* pDevEnumTemp = NULL;
	IEnumMoniker* pEnumTemp = NULL;
	IMoniker* pMonikerTemp = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnumTemp));
	hr = pDevEnumTemp->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumTemp, 0);
	hr = pEnumTemp->Reset();
	for (int i = 0; i < CameraNumber; i++)
	{
		hr = pEnumTemp->Next(1, &pMonikerTemp, NULL);
	}
	if (pDevEnumTemp != NULL)
	{
		pDevEnumTemp->Release();
	}
	if (pEnumTemp != NULL)
	{
		pEnumTemp->Release();
	}
	if (pMonikerTemp != NULL)
	{
		pMonikerTemp->Release();
	}

	CoUninitialize();
	return (hr == S_OK);
}

int	DirectShowCamera::GetAvailableCameraCount()
{

	int cameraCount = 0;
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ICreateDevEnum* pDevEnumTemp = NULL;
	IEnumMoniker* pEnumTemp = NULL;
	IMoniker* pMonikerTemp = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnumTemp));
	hr = pDevEnumTemp->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumTemp, 0);
	hr = pEnumTemp->Reset();
	while (pEnumTemp->Next(1, &pMonikerTemp, NULL) == S_OK)
	{

		if (hr == S_OK)
		{
			string cameraFriendlyName;
			IPropertyBag* pPropBag;
			hr = pMonikerTemp->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
			if (FAILED(hr))
			{
				continue;
			}
			VARIANT var;
			VariantInit(&var);
			hr = pPropBag->Read(L"DevicePath", &var, 0);
			if (SUCCEEDED(hr))
			{
				std::wstring cameraPathRaw(var.bstrVal, SysStringLen(var.bstrVal));
				const std::string cameraPath(cameraPathRaw.begin(), cameraPathRaw.end());
				auto strlist = split((string)cameraPath, "#");
				if (strlist.size() > 1)
				{
					auto strlist2 = split(strlist[1], "&");
					if (strlist2.size() > 1)
					{
						cameraCount++;
					}

				}
			}
			VariantClear(&var);
			pPropBag->Release();

		}
		Sleep(10);
	}

	if (pDevEnumTemp != NULL)
	{
		pDevEnumTemp->Release();
	}
	if (pEnumTemp != NULL)
	{
		pEnumTemp->Release();
	}
	if (pMonikerTemp != NULL)
	{
		pMonikerTemp->Release();
	}
	CoUninitialize();
	return cameraCount;
}

CameraNameList DirectShowCamera::getCameraNameList()
{
	//LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "getCameraNameList start");
	//Logger::getInstance(DEVELOPER_APPENDER_ID).log(TRACE_LOG_LEVEL, L">> DirectShowCamera::getCameraNameList");
	CameraNameList cameraList;
	cameraList.clear();
	exit_message("Initialize for Video", 0);
	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) == S_OK)
	{
		if (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild) == S_OK)
		{
			if (CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum)) == S_OK)
			{
				if (pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0) == S_OK && pEnum != NULL)
				{
					auto hr = pEnum->Reset();
					while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
					{
						string cameraFriendlyName;
						IPropertyBag* pPropBag;
						hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
						if (FAILED(hr))
						{
							continue;
						}
						VARIANT var;
						VariantInit(&var);
						hr = pPropBag->Read(L"FriendlyName", &var, 0);
						if (SUCCEEDED(hr))
						{
							std::wstring cameraFriendlyNameWs(var.bstrVal, SysStringLen(var.bstrVal));
							cameraFriendlyName = string(cameraFriendlyNameWs.begin(), cameraFriendlyNameWs.end());
						}
						VariantClear(&var);
						hr = pPropBag->Read(L"DevicePath", &var, 0);
						if (SUCCEEDED(hr))
						{
							std::wstring cameraPathRaw(var.bstrVal, SysStringLen(var.bstrVal));
							const std::string cameraPath(cameraPathRaw.begin(), cameraPathRaw.end());
							cout << "---------------------------------" << endl;
							cout << cameraPath << endl;
							cout << "---------------------------------" << endl;
							auto strlist = split((string)cameraPath, "#");
							if (strlist.size() > 1)
							{
								auto strlist2 = split(strlist[1], "&");
								if (strlist2.size() > 1)
								{
									cameraList.insert(move(make_pair(move(cameraPath), move(strlist2[0] + strlist2[1]))));
								}

							}
						}
						VariantClear(&var);
						pPropBag->Release();
					}
					exit_message("Finish getCameraNameList", 0);
				}
				else
					exit_message("Create Class Enumerator Failed", 0);
			}
			else
				exit_message("Create Device Enumerator Failed", 0);
		}
		else
			exit_message("Create Device Enumerator Failed", 0);
	}
	else
		exit_message("Initialize Multi Thread Failed", 0);

	CoUninitialize();
	//	LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "getCameraNameList end");

	return move(cameraList);
}

bool DirectShowCamera::setDeviceID(int ID)
{
	DeviceID = ID;
	return true;
}

int DirectShowCamera::getDeviceID()
{
	return DeviceID;
}

bool DirectShowCamera::RecordVideo(pair<string, string>& pIDvID, long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode = Asynch, LPCTSTR EngraveText)
{
	HRESULT hr = 0;
	exit_message("Initialize for RecordVideo with pIDvID", 0);
	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) == S_OK)
	{
		if (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild) == S_OK)
		{
			if (SelectCamera(pIDvID) == S_OK)
			{
				if (EngraveText != NULL)
				{
					SampleCallback.setVideoText(EngraveText);
				}
				return StartRecordVideo(Duration, OutputFileName, SynchMode);
			}
			else
			{
				exit_message("Error in select camera", 1);
				return false;
			}
		}
		else
		{
			exit_message("Error in CoCreateInstance", 1);
			return false;
		}
	}
	else
	{
		exit_message("Error in CoCreateInstance", 1);
		return false;
	}
}
bool DirectShowCamera::RecordVideo(int CameraNumber, long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode = Asynch, LPCTSTR EngraveText)
{
	if (inUse)
	{
		return -10;
	}
	inUse = true;
	HRESULT hr = 0;
	exit_message("Initialize for RecordVideo with pIDvID", 0);
	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) == S_OK)
	{
		if (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild) == S_OK)
		{
			if (SelectCamera(CameraNumber) == S_OK)
			{
				if (EngraveText != NULL)
				{
					SampleCallback.setVideoText(EngraveText);
				}
				return StartRecordVideo(Duration, OutputFileName, SynchMode);
			}
			else
			{
				exit_message("Error in select camera", 1);
				return false;
			}
		}
		else
		{
			exit_message("Error in CoCreateInstance", 1);
			return false;
		}
	}
	else
	{
		exit_message("Error in CoCreateInstance", 1);
		return false;
	}

}
HRESULT DirectShowCamera::GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
	IEnumPins* pEnum;
	IPin* pPin;
	pFilter->EnumPins(&pEnum);
	while (pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_DIRECTION PinDirThis;
		pPin->QueryDirection(&PinDirThis);
		if (PinDir == PinDirThis)
		{
			pEnum->Release();
			*ppPin = pPin;
			return S_OK;
		}
		pPin->Release();
	}
	pEnum->Release();
	return E_FAIL;
}
HRESULT DirectShowCamera::ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pFirst, IBaseFilter* pSecond)
{
	IPin* pOut = NULL, * pIn = NULL;
	HRESULT hr = GetPin(pSecond, PINDIR_INPUT, &pIn);
	if (FAILED(hr)) return hr;
	// The previous filter may have multiple outputs, so try each one!
	IEnumPins* pEnum;
	pFirst->EnumPins(&pEnum);
	while (pEnum->Next(1, &pOut, 0) == S_OK)
	{
		PIN_DIRECTION PinDirThis;
		pOut->QueryDirection(&PinDirThis);
		if (PINDIR_OUTPUT == PinDirThis)
		{
			hr = pGraph->Connect(pOut, pIn);
			if (!FAILED(hr))
			{
				break;
			}
		}
		pOut->Release();
	}
	pEnum->Release();
	pIn->Release();
	pOut->Release();
	return hr;
}
bool DirectShowCamera::StartRecordVideo(long Duration, LPCTSTR OutputFileName, SynchronizMode SynchMode)
{
	hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**)&pGraph);
	hr = CoCreateInstance(CLSID_WMAsfWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pASFWriter);
	hr = pGraph->AddFilter(pCap, L"Capture Filter");
	hr = pBuild->SetFiltergraph(pGraph);
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
	hr = pBuild->SetOutputFileName(&MEDIASUBTYPE_Asf, OutputFileName, &pASFWriter, NULL); // Remove this line if you only want a preview.
	hr = pASFWriter->QueryInterface(IID_IConfigAsfWriter, (void**)&pConfigWM);
	hr = ConfigFormat();
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSampleGrabberFilter));
	hr = pGraph->AddFilter(pSampleGrabberFilter, L"Sample Grabber");
	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
	hr = pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, pSampleGrabberFilter, pASFWriter);             // Pointer to the sink filter (ASF Writer).

	/*AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	mt.formattype = GUID_NULL;
	hr = pSampleGrabber->SetMediaType(&mt);*/
	hr = pSampleGrabber->SetOneShot(FALSE);

	if (SUCCEEDED(hr))
	{
		hr = pSampleGrabber->SetBufferSamples(FALSE);
		if (SUCCEEDED(hr))
		{
			if (0)
			{
				if (sgcb != NULL)
				{
					delete sgcb;
					sgcb = NULL;
				}
				sgcb = new SampleGrabberCallback();
				hr = pSampleGrabber->SetCallback((SampleGrabberCallback*)sgcb, 1);
			}
			else
			{
				long pWidth = 0;
				long pHeight = 0;
				AM_MEDIA_TYPE mt = { 0 };
				HRESULT hr = pSampleGrabber->GetConnectedMediaType(&mt);
				if (SUCCEEDED(hr))
				{
					if (mt.formattype == FORMAT_VideoInfo)
					{
						VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
						pWidth = pVih->bmiHeader.biWidth;
						pHeight = pVih->bmiHeader.biHeight;
					}

				}
				SampleCallback.setFrameSize(pHeight, pWidth);
				hr = pSampleGrabber->SetCallback(&SampleCallback, 0);
			}
		}
		if (hr)
		{
			exit_message("Stoped ", 0);
			inUse = false;
			return false;
		}
	}

	if (0)
	{
		AM_MEDIA_TYPE mt = { 0 };
		HRESULT hr = pSampleGrabber->GetConnectedMediaType(&mt);
		if (SUCCEEDED(hr))
		{
			if (mt.formattype == FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
				long pWidth = pVih->bmiHeader.biWidth;
				long pHeight = pVih->bmiHeader.biHeight;
			}

		}

		if (FAILED(hr)) {
			return hr;
		}
		VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
		auto gChannels = pVih->bmiHeader.biBitCount / 8;
		auto gWidth = pVih->bmiHeader.biWidth;
		auto gHeight = pVih->bmiHeader.biHeight;
	}

	/*Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);*/
	videoDuration = (Duration * 1000);
	hr = pControl->Run();

	if (hr != S_OK)
	{
		exit_message("Stoped ", 0);
		inUse = false;
		return false;
	}

	if (SynchMode == Synch)
	{
		long evCode = 0;
		auto result = pEvent->WaitForCompletion(videoDuration, &evCode);
		hr = pControl->Stop();
		exit_message("Stoped Video By Time Limit", 2);
		if (hr != S_OK)
		{
			return false;
		}
	}
	return true;
}

bool DirectShowCamera::TakeImage(pair<string, string>& pIDvID, int Height, int Width, LPCTSTR OutputFileName, LPCTSTR ImageLabel)
{
	HRESULT hr = 0;
	threadName = OutputFileName;
	if (!isOpen)
	{
		exit_message("Initialize for TakeImage", 3);
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
		hr = pBuild->SetFiltergraph(pGraph);
		if (SelectCamera(pIDvID) != S_OK)
		{
			exit_message("Error in select camera", 1);
			return hr;
		}
		hr = pGraph->AddFilter(pCap, L"Capture Filter");
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSampleGrabberFilter);
		hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
		hr = pSampleGrabber->SetBufferSamples(TRUE);
		//openerThread = std::thread(&DirectShowCamera::openCamera, this, Height, Width);
		openCamera(Height, Width);
		isOpen = true;
		Sleep(1000);
	}

	if (captureImage(OutputFileName, ImageLabel) == S_OK)
	{
		//LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "TakeImage end");
		//LOG4CPLUS_INFO(Logger::getInstance(DEVELOPER_APPENDER_ID), "Take Image - Result : Successful - Camera Name : " << pIDvID.first.c_str());
		return true;
	}

	//LOG4CPLUS_DEBUG(Logger::getInstance(DEVELOPER_APPENDER_ID), "TakeImage end");
	//LOG4CPLUS_WARN(Logger::getInstance(DEVELOPER_APPENDER_ID), "Take Image - Result : Failed - Reason : result of captureImage function - Camera Name: " << pIDvID.first.c_str());
	return false;
}
bool DirectShowCamera::TakeImage(int CameraNumber, int Height, int Width, LPCTSTR OutputFileName, LPCTSTR ImageLabel)
{

	HRESULT hr = 0;
	threadName = OutputFileName;
	if (!isOpen)
	{
		exit_message("Initialize for TakeImage", 3);
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
		hr = pBuild->SetFiltergraph(pGraph);
		hr = SelectCamera(CameraNumber);
		if (FAILED(hr))
		{
			exit_message("Error in select camera", 1);
			return hr;
		}
		hr = pGraph->AddFilter(pCap, L"Capture Filter");
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSampleGrabberFilter);
		hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
		hr = pSampleGrabber->SetBufferSamples(TRUE);
		//openerThread = std::thread(&DirectShowCamera::openCamera,this,Height,Width);
		openCamera(Height, Width);
		isOpen = true;
		//Sleep(1000);
	}
	if (captureImage(OutputFileName, ImageLabel) == S_OK)
	{
		//LOG4CPLUS_INFO(Logger::getInstance(DEVELOPER_APPENDER_ID), "Take Image - Result : Successful - Camera Number : " << CameraNumber);
		return true;
	}

	//LOG4CPLUS_WARN(Logger::getInstance(DEVELOPER_APPENDER_ID), "Take Image - Result : Faile - Reason : result of captureImage function - Camera Number : " << CameraNumber);
	return false;

}

void DirectShowCamera::openCamera(int Height, int Width)
{
	int snapshot_delay = 2000;
	int show_preview_window = 0;
	int list_devices = 0;
	int device_number = 1;
	char char_buffer[100];
	int loopController = 10;
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = pSampleGrabber->SetMediaType((_AMMediaType*)&mt);
	if (FAILED(hr)) { return; }

	// Add sample grabber filter to filter graph
	hr = pGraph->AddFilter(pSampleGrabberFilter, L"SampleGrab");
	if (FAILED(hr)) { return; }
	// Create Null Renderer filter
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pNullRenderer);
	if (FAILED(hr)) { return; }
	// Add Null Renderer filter to filter graph
	hr = pGraph->AddFilter(pNullRenderer, L"NullRender");

	if (FAILED(hr)) { return; }
#pragma region selectCompresssion_Resulation	
	//if (Width > 0 && Height > 0)
	{
		IAMStreamConfig* pConfig = NULL;
		hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pCap, IID_IAMStreamConfig, (void**)&pConfig);
		if (FAILED(hr)) { return; }
		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		if (FAILED(hr)) { return; }
		// Check the size to make sure we pass in the correct structure.
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			LONG lsize = 0x00;
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE* pmtConfig;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					/* Examine the format, and possibly use it. */
					if (
						(pmtConfig->majortype == MEDIATYPE_Video) &&
						//(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&
						(pmtConfig->formattype == FORMAT_VideoInfo) &&
						(pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
						(pmtConfig->pbFormat != NULL)
						)
					{
						VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						// pVih contains the detailed format information.
						LONG lWidth = pVih->bmiHeader.biWidth;
						LONG lHeight = pVih->bmiHeader.biHeight;
						//cout << "\n" << iFormat << "\t" << lWidth << "\t" << lHeight << "\t" << pVih->bmiHeader.biCompression << "\t" << pVih->bmiHeader.biSize << "\t" << pVih->bmiHeader.biSizeImage << "\t" << pVih->bmiHeader.biBitCount << "\n";
						if (lWidth == Width && lHeight == Height /*&& pVih->bmiHeader.biCompression == 1196444237*/)
						{
							if (pVih->bmiHeader.biSizeImage > lsize)
							{
								lsize = pVih->bmiHeader.biSizeImage;
								//pVih->bmiHeader.biBitCount = 16;
								hr = pConfig->SetFormat(pmtConfig);
								if (FAILED(hr)) { return; }
							}
						}
					}
					// Delete the media type when you are done.
				}
			}
		}
	}
#pragma endregion selectCompresssion_Resulation

	// Connect up the filter graph's capture stream
	hr = pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, pSampleGrabberFilter, pNullRenderer);

	if (FAILED(hr)) { return; }
	// Connect up the filter graph's preview stream
	if (show_preview_window > 0)
	{
		hr = pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap, NULL, NULL);
	}
	// Get media control interfaces to graph builder object
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);

	if (FAILED(hr)) { return; }
	// Run graph
	while (loopController--)
	{
		hr = pControl->Run();
		if (hr == S_OK) break; // graph is now running
		if (hr == S_FALSE) continue; // graph still preparing to run		
	}
	if (0 && hr == S_OK)
	{
		long evCode = 0;
		pEvent->WaitForCompletion(INFINITE, &evCode);
	}

	//exit_message("Could not run filter graph", 6);
	return;
}

bool DirectShowCamera::WriteOnImage(LPCTSTR ImageFileName, LPCTSTR EngraveText)
{
	bool bRet = false;
	//Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	//ULONG_PTR gdiplusToken;
	//GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	if (EngraveText != NULL && lstrlen(EngraveText) > 0)
	{

		CLSID   encoderClsid;
		Gdiplus::Status  stat;
		CComPtr<IStream> stream;

		std::ifstream file(ImageFileName, std::ios::binary);
		file.seekg(0, std::ios::end);
		auto size = file.tellg();
		file.seekg(0, std::ios::beg);
		std::streambuf* raw_buffer = file.rdbuf();
		unique_ptr<char[]> buffer(new char[size]);
		raw_buffer->sgetn((char*)buffer.get(), size);
		stream.Attach(SHCreateMemStream((BYTE*)buffer.get(), size));
		file.close();

		Gdiplus::Image* image = Gdiplus::Image::FromStream(stream);
		Gdiplus::Graphics     Gx(image);
		FontFamily  fontFamily(L"Arial");
		Gdiplus::Font font(&fontFamily, 16, FontStyleRegular, UnitPixel);
		PointF      pointF(10.0f, image->GetHeight() - 40);
		SolidBrush  solidBrush(Color::Red);
		Gx.DrawString(EngraveText, -1, &font, pointF, &solidBrush);
		Gx.Save();
		// Get the CLSID of the JPEG encoder.						
		GetEncoderClsid(L"image/jpeg", &encoderClsid);
		remove(CStringA(ImageFileName));
		stat = image->Save(ImageFileName, &encoderClsid, NULL);
		if (stat == Gdiplus::Ok)
		{
			//printf("Bird.png was saved successfully\n");
			bRet = true;
		}
		else
		{
			bRet = false;
			//printf("Failure: stat = %d\n", stat);
		}
		delete image;
	}
	//Gdiplus::GdiplusShutdown(gdiplusToken);
	return bRet;
}

DirectShowCamera::DirectShowCamera(int CameraNumber, int CaptureType, LPCTSTR OutputFileName)
{
	HRESULT hr = 0;
	captureType = CaptureType;
	threadName = OutputFileName;
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);

	if (captureType == CaptureType::Video)
	{
		SelectCamera(CameraNumber);
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**)&pGraph);
		hr = CoCreateInstance(CLSID_WMAsfWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pASFWriter);
		hr = pGraph->AddFilter(pCap, L"Capture Filter");
		hr = pBuild->SetFiltergraph(pGraph);
		hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
		hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
		hr = pBuild->SetOutputFileName(&MEDIASUBTYPE_Asf, OutputFileName, &pASFWriter, NULL); // Remove this line if you only want a preview.
		hr = pASFWriter->QueryInterface(IID_IConfigAsfWriter, (void**)&pConfigWM);
		hr = ConfigFormat();
		hr = pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, 0, pASFWriter);             // Pointer to the sink filter (ASF Writer).	 
	}
	else
	{
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
		hr = pBuild->SetFiltergraph(pGraph);
		SelectCamera(CameraNumber);
		hr = pGraph->AddFilter(pCap, L"Capture Filter");
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSampleGrabberFilter);
		hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
		hr = pSampleGrabber->SetBufferSamples(TRUE);
	}

}

bool DirectShowCamera::Stop()
{
	FILTER_STATE fs;
	if (pControl != NULL)
	{
		hr = pControl->GetState(10, (OAFilterState*)&fs);
	}

	if (pControl != NULL && fs != FILTER_STATE::State_Stopped)
	{
		pControl->Stop();
	}

	exit_message("Stoped Video By Stop", 4);
	return true;
}

DirectShowCamera::~DirectShowCamera()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
	openerThread.join();
	exit_message("deconstructor", 0);
}

void DirectShowCamera::exit_message(const char* error_message, int error)
{
	//cout << error_message << "\t" << error << endl;
	if (isOpen)
	{
		while (0)
		{
			DWORD  dr = TerminateThread(openerThread.native_handle(), 1);
			if (dr != 0)
			{
				break;
			}
			Sleep(1);
		}
		//openerThread.detach();
		isOpen = false;
	}

	if (pControl != NULL)
	{
		FILTER_STATE fs;
		if (pControl != NULL)
		{
			hr = pControl->GetState(10, (OAFilterState*)&fs);
		}

		if (pControl != NULL && fs != FILTER_STATE::State_Stopped)
		{
			hr = pControl->Stop();
		}
		if (pControl != NULL)
			pControl->Release();
		pControl = NULL;
	}

	if (pBuffer != NULL)
	{
		delete[] pBuffer;
		pBuffer = NULL;
	}
	if (pWindow != NULL)
	{
		pWindow->Release();
		pWindow = NULL;
	}
	if (pEvent != NULL)
	{
		pEvent->Release();
		pEvent = NULL;
	}
	if (pNullRenderer != NULL)
	{
		pNullRenderer->Release();
		pNullRenderer = NULL;
	}
	if (pSampleGrabber != NULL)
	{
		pSampleGrabber->Release();
		pSampleGrabber = NULL;
	}
	if (pSampleGrabberFilter != NULL)
	{
		pSampleGrabberFilter->Release();
		pSampleGrabberFilter = NULL;
	}
	if (pCap != NULL)
	{
		pCap->Release();
		pCap = NULL;
	}
	if (pBuild != NULL)
	{
		pBuild->Release();
		pBuild = NULL;
	}
	if (pGraph != NULL)
	{
		pGraph->Release();
		pGraph = NULL;
	}
	if (pPropBag != NULL)
	{
		pPropBag->Release();
		pPropBag = NULL;
	}
	if (pMoniker != NULL)
	{
		pMoniker->Release();
		pMoniker = NULL;
	}
	if (pEnum != NULL)
	{
		pEnum->Release();
		pEnum = NULL;
	}
	if (pDevEnum != NULL)
	{
		pDevEnum->Release();
		pDevEnum = NULL;
	}
	if (pASFWriter != NULL)
	{
		pASFWriter->Release();
		pASFWriter = NULL;
	}

	if (pConfigWM != NULL)
	{
		pConfigWM->Release();
		pConfigWM = NULL;
	}
	SampleCallback.setVideoText(NULL);
	//cout << "CoUninitializeStart" << "\n" << endl;
	CoUninitialize();
	//cout << "CoUninitializeFinish" << "\n" << endl;
}

int DirectShowCamera::captureImage(LPCTSTR FileName, LPCTSTR ImageLabel)
{
	long buffer_size = 0;
	auto loopController = 10000;
	while (loopController--)
	{
		// Passing in a NULL pointer signals that we're just checking
		// the required buffer size; not looking for actual data yet.
		hr = pSampleGrabber->GetCurrentBuffer(&buffer_size, NULL);
		// Keep trying until buffer_size is set to non-zero value.
		if (hr == S_OK && buffer_size != 0) break;
		// If the return value isn't S_OK or VFW_E_WRONG_STATE
		// then something has gone wrong. VFW_E_WRONG_STATE just
		// means that the filter graph is still starting up and
		// no data has arrived yet in the sample grabber filter.
		Sleep(1);
		if (hr != S_OK && hr != VFW_E_WRONG_STATE)
		{
			exit_message("Could not get buffer size", 7);
			return -1;
		}
	}

	if (buffer_size <= 0)
	{
		exit_message("Could not get buffer size", 8);
		return -1;
	}

	pBuffer = new char[buffer_size];
	if (!pBuffer)
		exit_message("Could not allocate data buffer for image", 1);
	// Retrieve image data from sample grabber buffer
	hr = pSampleGrabber->GetCurrentBuffer(&buffer_size, (long*)pBuffer);
	if (FAILED(hr)) { return -1; }
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = pSampleGrabber->SetMediaType((_AMMediaType*)&mt);
	// Get the media type from the sample grabber filter
	hr = pSampleGrabber->GetConnectedMediaType((_AMMediaType*)&mt);
	if (hr != S_OK)
	{
		exit_message("Could not get media type", 9);
		return -1;
	}
	// Retrieve format information
	VIDEOINFOHEADER* pVih = NULL;
	if ((mt.formattype == FORMAT_VideoInfo) && (mt.cbFormat >= sizeof(VIDEOINFOHEADER)) && (mt.pbFormat != NULL))
	{
		// Get video info header structure from media type
		pVih = (VIDEOINFOHEADER*)mt.pbFormat;
		// Create bitmap structure
		long cbBitmapInfoSize = mt.cbFormat - SIZE_PREHEADER;
		BITMAPFILEHEADER bfh;
		ZeroMemory(&bfh, sizeof(bfh));
		bfh.bfType = 'MB'; // Little-endian for "BM".
		bfh.bfSize = sizeof(bfh) + buffer_size + cbBitmapInfoSize;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + cbBitmapInfoSize;
		DWORD dwAllWritten = 0;
		dwAllWritten = buffer_size + cbBitmapInfoSize + sizeof(bfh);
		BYTE* sp1 = new BYTE[dwAllWritten];
		char* pointer = (char*)sp1;
		CopyMemory(pointer, &bfh, sizeof(bfh));
		pointer += sizeof(bfh);
		CopyMemory(pointer, HEADER(pVih), cbBitmapInfoSize);
		pointer += cbBitmapInfoSize;
		CopyMemory(pointer, pBuffer, buffer_size);
		CComPtr<IStream> stream;
		stream.Attach(SHCreateMemStream(sp1, dwAllWritten));
		delete[] sp1;
		// Initialize GDI+.
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		{
			CLSID   encoderClsid;
			Gdiplus::Status  stat;
			Gdiplus::Image* image = Gdiplus::Image::FromStream(stream);
			if (lstrlen(ImageLabel))
			{
				Gdiplus::Graphics     Gx(image);
				Gdiplus::FontFamily  fontFamily(L"Arial");
				Gdiplus::Font font(&fontFamily, 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
				Gdiplus::PointF      pointF(10.0f, image->GetHeight() - 40);
				Gdiplus::SolidBrush  solidBrush(Gdiplus::Color(255, 255, 0, 255));
				Gx.DrawString(ImageLabel, -1, &font, pointF, &solidBrush);
				// Get the CLSID of the JPEG encoder.						
			}
			GetEncoderClsid(L"image/jpeg", &encoderClsid);
			stat = image->Save(threadName, &encoderClsid, NULL);
			if (stat == Gdiplus::Ok)
			{

			}
			else
			{
			}
			delete image;
		}
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
	else
	{
		exit_message("Wrong media type", 1);
		return 0xff;
	}
	// Free the format block
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
	if (pBuffer != NULL)
	{
		delete[] pBuffer;
		pBuffer = NULL;
	}
	return Gdiplus::Ok;
}

HRESULT DirectShowCamera::ConfigFormat()
{
	IWMProfileManager* profileManager = NULL;
	IWMProfile* currentProfile = NULL;
	HRESULT hr = WMCreateProfileManager(&profileManager);
	auto prof = L"<profile version=\"589824\" storageformat=\"1\" name=\"Quality\" description=\"Quality type for output.\">"
		L"<streamconfig majortype=\"{73646976-0000-0010-8000-00AA00389B71}\" streamnumber=\"1\" "
		L"streamname=\"Video Stream\" inputname=\"Video409\" bitrate=\"894960\" "
		L"bufferwindow=\"0\" reliabletransport=\"1\" decodercomplexity=\"AU\" rfc1766langid=\"en-us\">"
		L"<videomediaprops maxkeyframespacing=\"50000000\" quality=\"90\"/>"
		L"<wmmediatype subtype=\"{33564D57-0000-0010-8000-00AA00389B71}\" bfixedsizesamples=\"0\" "
		L"btemporalcompression=\"1\" lsamplesize=\"0\">"
		L"<videoinfoheader dwbitrate=\"894960\" dwbiterrorrate=\"0\" avgtimeperframe=\"10000\">"
		L"<rcsource left=\"0\" top=\"0\" right=\"640\" bottom=\"480\"/>"
		L"<rctarget left=\"0\" top=\"0\" right=\"640\" bottom=\"480\"/>"
		L"<bitmapinfoheader biwidth=\"640\" biheight=\"480\" biplanes=\"1\" bibitcount=\"24\" "
		L"bicompression=\"WMV3\" bisizeimage=\"0\" bixpelspermeter=\"0\" biypelspermeter=\"0\" "
		L"biclrused=\"0\" biclrimportant=\"0\"/>"
		L"</videoinfoheader>"
		L"</wmmediatype>"
		L"</streamconfig>"
		L"</profile>";
	hr = profileManager->LoadProfileByData(prof, &currentProfile);
	hr = pConfigWM->ConfigureFilterUsingProfile(currentProfile);
	if (currentProfile != NULL)
	{
		currentProfile->Release();
	}
	if (profileManager != NULL)
	{
		profileManager->Release();
	}
	return hr;
}

int DirectShowCamera::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

vector<string> DirectShowCamera::split(string data, string delimiter)
{
	vector<string> splits;
	splits.clear();

	size_t pos = 0;
	string token;
	while ((pos = data.find(delimiter)) != string::npos)
	{
		token = data.substr(0, pos);
		splits.push_back(token);
		data.erase(0, pos + delimiter.length());
	}

	splits.push_back(data);

	return splits;
}

void DirectShowCamera::Live(pair<string, string>DeviceUniquePath)
{
	exit_message("Initialize for TakeImage", 0);
	HRESULT hr = 0;
	//sprintf_s((char*)threadName, "Live from camera %d", CameraNumber);
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
	hr = pBuild->SetFiltergraph(pGraph);
	SelectCamera(DeviceUniquePath);
	hr = pGraph->AddFilter(pCap, L"Capture Filter");
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
	hr = pGraph->QueryInterface(IID_IVideoWindow, (void**)&pWindow);
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSampleGrabberFilter);
	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabber);
	hr = pSampleGrabber->SetBufferSamples(TRUE);
	GetLiveFromCamera(INFINITE);
}

int DirectShowCamera::GetLiveFromCamera(long duration)
{
	// Capture settings
	int Width = 640, Height = 480;
	int snapshot_delay = 20000;
	long show_preview_window = 1;
	int list_devices = 0;
	int device_number = 1;
	char char_buffer[100];
	int n = 1;
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = pSampleGrabber->SetMediaType((_AMMediaType*)&mt);
	// Add sample grabber filter to filter graph
	hr = pGraph->AddFilter(pSampleGrabberFilter, L"SampleGrab");
	// Create Null Renderer filter
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pNullRenderer);
	// Add Null Renderer filter to filter graph
	hr = pGraph->AddFilter(pNullRenderer, L"NullRender");

#pragma region selectCompresssion_Resulation
	if (0 && Width > 0 && Height > 0)
	{
		IAMStreamConfig* pConfig = NULL;
		hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pCap, IID_IAMStreamConfig, (void**)&pConfig);
		int iCount = 0, iSize = 0;
		hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
		// Check the size to make sure we pass in the correct structure.
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
		{
			// Use the video capabilities structure.
			LONG lsize = 0x00;
			for (int iFormat = 0; iFormat < iCount; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE* pmtConfig;
				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					/* Examine the format, and possibly use it. */
					if (
						(pmtConfig->majortype == MEDIATYPE_Video) &&
						//(pmtConfig->subtype == MEDIASUBTYPE_RGB24) &&
						(pmtConfig->formattype == FORMAT_VideoInfo) &&
						(pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
						(pmtConfig->pbFormat != NULL)
						)
					{
						VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						// pVih contains the detailed format information.
						LONG lWidth = pVih->bmiHeader.biWidth;
						LONG lHeight = pVih->bmiHeader.biHeight;
						//cout << "\n" << iFormat << "\t" << lWidth << "\t" << lHeight << "\t" << pVih->bmiHeader.biCompression << "\t" << pVih->bmiHeader.biSize << "\t" << pVih->bmiHeader.biSizeImage << "\t" << pVih->bmiHeader.biBitCount << "\n";
						if (lWidth == Width && lHeight == Height /*&& pVih->bmiHeader.biCompression == 1196444237*/)
						{
							if (pVih->bmiHeader.biSizeImage > lsize)
							{
								lsize = pVih->bmiHeader.biSizeImage;
								//pVih->bmiHeader.biBitCount = 16;
								hr = pConfig->SetFormat(pmtConfig);
							}
						}
					}
					// Delete the media type when you are done.
				}
			}
		}
	}
#pragma endregion selectCompresssion_Resulation

	// Connect up the filter graph's capture stream
	hr = pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, pSampleGrabberFilter, pNullRenderer);

	// Connect up the filter graph's preview stream
	if (show_preview_window > 0)
	{
		hr = pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap, NULL, NULL);
	}
	// Get media control interfaces to graph builder object
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
	//OAHWND hv;
	//HWND dummyHWND = ::CreateWindowA("STATIC", "dummy", WS_VISIBLE | WS_SYSMENU , 0, 0, 800, 600, NULL, NULL, NULL, NULL);
	//::SendMessage(dummyHWND, WM_NCHITTEST, VK_RETURN, 0x20000000);

	//::SetWindowTextA(dummyHWND, "Dummy Window!");
	//DefWindowProc(dummyHWND, NULL, NULL, NULL);
	//pWindow->get_Owner(&hv);	
	//pWindow->put_Owner((OAHWND)GetActiveWindow());//GetActiveWindow();//GetConsoleWindow();//GetCurrentProcess()
	//pWindow->put_WindowStyle(WINSTA_READSCREEN);
	//pWindow->put_Width(320);
	//pWindow->put_Height(240);

	//pWindow->put_WindowStyle(DCX_WINDOW);
	//pWindow->SetWindowForeground(0);
	//pWindow->put_AutoShow(1);
	//pWindow->SetWindowPosition(0, 0, 640, 480);
	//pWindow->put_Visible(OAFALSE);
	//pWindow->put_Caption((BSTR)L"Device Capture");
	//pWindow->put_MessageDrain((OAHWND)GetActiveWindow());
	// Run graph
	while (1)
	{
		hr = pControl->Run();

		// Hopefully, the return value was S_OK or S_FALSE
		if (hr == S_OK) break; // graph is now running
		if (hr == S_FALSE) continue; // graph still preparing to run
		// If the Run function returned something else,
		// there must be a problem
		fprintf(stderr, "Error: %u\n", hr);
		exit_message("Could not run filter graph", 1);
	}
	// Wait for specified time delay (if any)
	pEvent->WaitForCompletion(duration/*INFINITE*/, &show_preview_window);

	pControl->Stop();
	// Allocate buffer for image

	// Free the format block
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}

	// Clean up and exit	
	exit_message("Image Captured", 0);
	return 0;
}

ULONG SampleGrabberCallback::AddRef()
{
	return 2;
}

ULONG SampleGrabberCallback::Release()
{
	return 1;
}

STDMETHODIMP_(HRESULT __stdcall) SampleGrabberCallback::QueryInterface(REFIID iid, void** ppv)
{
	if (NULL == ppv)
		return E_POINTER;

	*ppv = NULL;
	if (IID_IUnknown == iid)
	{
		*ppv = (IUnknown*)this;
		AddRef();
		return S_OK;
	}
	else if (IID_ISampleGrabberCB == iid)
	{
		*ppv = (ISampleGrabberCB*)this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

SampleGrabberCallback::SampleGrabberCallback()
{

	framenum = 1;
}

SampleGrabberCallback::~SampleGrabberCallback()
{
}

void SampleGrabberCallback::setVideoText(LPCTSTR Text)
{
	EngraveText = Text;
}
TCHAR* SampleGrabberCallback::stringToTChar(string data)
{
	auto dataBuffer = data.c_str();
	int length = 1 + strlen(dataBuffer);
	auto wideString = new wchar_t[length];
	size_t outSize;
	mbstowcs_s(&outSize, wideString, length, dataBuffer, length - 1);
	//mbstowcs(wideString, dataBuffer, length);
	return static_cast<TCHAR*>(wideString);
}

void SampleGrabberCallback::setFrameSize(int frameHeight, int frameWidth)
{
	height = frameHeight;
	width = frameWidth;
}

inline const std::string SampleGrabberCallback::currentTime()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[100] = { 0 };
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%D %H:%M:%S", &tstruct);
	return buf;
}
HRESULT __stdcall SampleGrabberCallback::SampleCB(double n, IMediaSample* pSample)
{
	//framenum++;	
	AM_MEDIA_TYPE* pMediaType;
	pSample->GetMediaType(&pMediaType);
	BYTE* pBuf = NULL;
	pSample->GetPointer(&pBuf);
	Bitmap image(width, height, 2 * width, PixelFormat16bppRGB565, pBuf);
	Graphics* Gx = Graphics::FromImage(&image);
	FontFamily  fontFamily(L"Arial");
	Gdiplus::Font font(&fontFamily, 32, FontStyleBold, UnitPixel);
	PointF      pointF(10.0f, height - 80);
	SolidBrush  solidBrush(Color::Red);
	LPCTSTR dateTime = (LPCTSTR)stringToTChar(currentTime());
	TCHAR pszDest[260] = _T("");
	StringCchCat(pszDest, sizeof(pszDest), dateTime);
	StringCchCat(pszDest, sizeof(pszDest), L"\n");
	StringCchCat(pszDest, sizeof(pszDest), EngraveText);
	delete[] dateTime;
	Gx->DrawString(pszDest, -1, &font, pointF, &solidBrush);
	Gx->Save();
	delete Gx;
	return S_OK;
}

HRESULT __stdcall SampleGrabberCallback::BufferCB(double SampleTime, BYTE* pBuffer, long BufferSize)
{
	//framenum++;
	BYTE* pBuf = pBuffer;
	Bitmap image(640, 480, 2 * 640, PixelFormat16bppRGB565, pBuf);
	Graphics* Gx = Graphics::FromImage(&image);
	FontFamily  fontFamily(L"Arial");
	Gdiplus::Font font(&fontFamily, 32, FontStyleBold, UnitPixel);
	PointF      pointF(10.0f, 400.0f);
	SolidBrush  solidBrush(Color::Red);
	Gx->DrawString(EngraveText, -1, &font, pointF, &solidBrush);
	Gx->Save();
	delete Gx;
	return S_OK;
}
