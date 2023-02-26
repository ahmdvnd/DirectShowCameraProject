// DirectShowCameraProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include "DirectShowCamera.h"
#include <Mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")
const std::string currentTime_()
{
	time_t now = time(0);
	struct tm tstruct;
	char buf[1000] = { 0 };
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
	return buf;
}
void	threadTest()
{
	DirectShowCamera cam1;
	auto camNamelist = cam1.getCameraNameList();
	vector<DirectShowCamera*> camObjects;
	vector<thread*> threadObjects;
	for (auto value : camNamelist)
	{
		DirectShowCamera* newCam = new DirectShowCamera();
		camObjects.push_back(newCam);
		cout << to_string(camObjects.size()) << ":" << value.first << " " << value.second << endl;

		thread* newThread = new thread(&DirectShowCamera::Live, newCam, value);
		threadObjects.push_back(newThread);

	}

	for (auto threadItem : threadObjects)
		threadItem->join();

	for (auto threadItem : threadObjects)
		delete threadItem;

	for (auto camItem : camObjects)
		delete camItem;
}
void CameraTester()
{
	DirectShowCamera cam1;
	int camNum, FuncType;
	while (true)
	{
		auto camNamelist = cam1.getCameraNameList();
		vector<DirectShowCamera*> camObjects;
		for (auto value : camNamelist)
		{
			DirectShowCamera* newCam = new DirectShowCamera();
			camObjects.push_back(newCam);
			cout << to_string(camObjects.size()) << ":" << value.first << " " << value.second << endl;
		}
		cout << "Select Camera:";
		cin >> camNum;
		camNum--;
		if (camNum < 0 && camNum >= camObjects.size())
		{
			camNum = 0;
		}
		cout << "Live:0 Take Image: 1 Take Video:2" << endl;
		cin >> FuncType;

		string BasePath = "D:/Temp/";
		if (FuncType == 0)
		{
			auto it = camNamelist.begin();
			std::advance(it, camNum);
			camObjects.at(camNum)->Live(make_pair(it->first, " "));
		}
		else if (FuncType == 1)
		{
			string fileName = BasePath + "CAM" + to_string(camNum + 1) + "_Image.jpg";
			wstring path_wstrVideo(fileName.begin(), fileName.end());
			auto it = camNamelist.begin();
			std::advance(it, camNum);
			camObjects.at(camNum)->TakeImage(make_pair(it->first, it->second), 600, 800, path_wstrVideo.c_str(), 0);
		}
		else if (FuncType == 2)
		{
			string fileName = BasePath + "CAM" + to_string(camNum + 1) + "_Video.wmv";
			wstring path_wstrVideo(fileName.begin(), fileName.end());
			auto it = camNamelist.begin();
			std::advance(it, camNum);
			camObjects.at(camNum)->Stop();
			camObjects.at(camNum)->RecordVideo(make_pair(it->first, it->second), 10, path_wstrVideo.c_str(), Synch);
		}
		camObjects.clear();
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	CameraTester(); return 0;
	//threadTest(); return 0;
	DirectShowCamera cam2;
	DirectShowCamera cam1;
	int tt = DirectShowCamera::GetAvailableCameraCount();
	auto list = cam1.getCameraNameList();

	//DirectShowCamera cam4;
	//cam4.TakeImage(2, 600, 800, L"C:/Temp/JustTest2.jpg", 0);
	//cam4.RecordVideo(2, 10, L"C:/Temp/JustTest2.wmv", Synch);

	//mciSendString(L"open \"Music.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
	time_t start, ending;
	time(&start);
	int i = 0;
	//auto resd = mciSendString(L"open \"C:\\Temp\\sound\\warning_Beep.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
	//auto resd=mciSendString(L"open \"C:\\Temp\\sound\\Beep-2sec.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
	//resd=mciSendString(L"play mp3", NULL, 0, NULL);		 

	//mciSendString(L"stop mp3", NULL, 0, NULL);
	//resd = PlaySound(L"D:/Personal/Music/Enigma - Je T aime till My Dying Day - musicgeek.ir.mp3", NULL, SND_FILENAME); //SND_FILENAME or SND_LOOP
	while (i++ < 20)
	{
		auto iter = list.begin();

		//using namespace date;

		string fName = "D:/Temp/Images/" + to_string(i) + ".jpg";
		wstring path_wstr(fName.begin(), fName.end());
		cam1.TakeImage(move(make_pair(iter->first, iter->second)), 600, 800, path_wstr.c_str(), 0);
		cout << path_wstr.c_str() << "\n";


		//cam1.TakeImage(1, 480, 640, path_wstr.c_str(), 0);


		string fNameVideo = "D:/Temp/" + to_string(i) + ".wmv";
		wstring path_wstrVideo(fNameVideo.begin(), fNameVideo.end());
		cam1.RecordVideo(move(make_pair(iter->first, iter->second)), 5, path_wstrVideo.c_str(), Synch, L"CARD NUMBER:6037-9911-2563-4998");

		//std::make_pair("vid_05a3pid_9320", "NH_USB CAMERA1")
		//"vid_1871pid_0306" 
		//"vid_2232pid_9023"
		// "vid_05a3pid_9320"
		//cout << fNameVideo << "\n";

		Sleep(1);
	}
	time(&ending);
	cout << "Total time required = "
		<< difftime(ending, start)
		<< " seconds " << endl;
	cin.get();
	return 0;
}

