#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Windows.h"
#include <opencv2\opencv.hpp>
#include <time.h>
#include <process.h>
#include <omp.h>

#include "videoCapturer.hpp"
#include "threadPool.hpp"

constexpr auto SKIP_SIZE = 0;
volatile int skipFrame = SKIP_SIZE;

namespace VideoPlayer {

	VideoCapturer::VideoCapturer(std::string host, int port, std::string username, std::string password) :
		host_(host), port_(port), username_(username), password_(password)
	{

	}

	VideoCapturer::~VideoCapturer()
	{
		std::cout << "[i] thread id :" << std::this_thread::get_id() << " deconstructor VideoCapturer..." << std::endl;
	}

	void VideoCapturer::start()
	{
		this->startCamera();
	}

	void VideoCapturer::stop()
	{
		this->stopCamera();			
	}

	void VideoCapturer::startCamera()
	{
		NET_DVR_Init();

		NET_DVR_SetConnectTime(2000, 1);
		NET_DVR_SetReconnect(10000, true);

		LONG lUserID;
		NET_DVR_DEVICEINFO_V30 struDeviceInfo;
		lUserID = NET_DVR_Login_V30((char*)(this->host_.c_str()), this->port_, (char*)(this->username_.c_str()), (char*)(this->password_.c_str()), &struDeviceInfo);
		if (lUserID < 0)
		{
			std::cout << "NET_DVR_Login_V30 error: " << NET_DVR_GetLastError() << std::endl;
			NET_DVR_Cleanup();
			return;
		}

		NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

		NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
		struPlayInfo.hPlayWnd = 0;
		struPlayInfo.lChannel = 1;
		struPlayInfo.dwStreamType = 0;
		struPlayInfo.dwLinkMode = 0;
		struPlayInfo.bBlocked = 0;

		lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, fRealDataCallBack, this);

		if (lRealPlayHandle < 0)
		{
			std::cout << "NET_DVR_RealPlay_V40 error: " << NET_DVR_GetLastError() << std::endl;
			NET_DVR_Logout(lUserID);
			NET_DVR_Cleanup();
			return;
		}

		std::cout << "[i] thread id :" << std::this_thread::get_id() << " Start..." << std::endl;
	}

	void VideoCapturer::stopCamera()
	{
		ThreadPool* thdPool = ThreadPool::getInstance();
		thdPool->DeleteHandle(this->host_);

		//停止预览
		NET_DVR_StopRealPlay(this->lRealPlayHandle);
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();

		//cv::destroyWindow(this->host_);
	}

	std::string VideoCapturer::getHost() const
	{
		return this->host_;
	}

	void VideoCapturer::show(cv::Mat frame)
	{
		cv::imshow(host_, frame);
		cvWaitKey(1);
	}

	void CALLBACK VideoCapturer::DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
	{
		if (skipFrame)
		{
			skipFrame--;
			return;
		}

		ThreadPool* thdPool = ThreadPool::getInstance();
		VideoHandle vHandle = thdPool->getHandleByPort(nPort);

		if (vHandle.vCapturer_ == nullptr)
			return;

		VideoCapturer* vc = vHandle.vCapturer_;

		long lFrameType = pFrameInfo->nType;
		if (lFrameType == T_YV12)
		{

			cv::Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
			cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
			cvtColor(src, pImg, CV_YUV2BGR_YV12);

			cv::Mat frametemp(pImg), frame;
			frametemp.copyTo(frame);
			
			struct tm ttime;
			time_t now;
			time(&now);
			localtime_s(&ttime, &now);
			std::string strTime = std::to_string(1900 + ttime.tm_year) + "-" + std::to_string(ttime.tm_mon + 1) + "-" + std::to_string(ttime.tm_mday) + " " + std::to_string(ttime.tm_hour) + ":" + std::to_string(ttime.tm_min) + ":" + std::to_string(ttime.tm_sec);

			ThreadPool* thdPool = ThreadPool::getInstance();
			thdPool->AddTask(Package(frame, vc->getHost(), ttime));
			std::cout << "[i] thread id :" << std::this_thread::get_id() << " produces frame from: " <<  vc->getHost() << std::endl;

		}

		skipFrame = SKIP_SIZE;
	}

	void CALLBACK VideoCapturer::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
	{
		ThreadPool* thdPool = ThreadPool::getInstance();
		VideoHandle vHandle = thdPool->getHandleBylRealHandle(lRealHandle);

		switch (dwDataType)
		{
		case NET_DVR_SYSHEAD: //系统头

			if (!PlayM4_GetPort(&vHandle.nPort_)) 
			{
				break;
			}
			else
			{
				vHandle.lRealHandle_ = lRealHandle;
				vHandle.vCapturer_ = (VideoCapturer*)pUser;
				thdPool->AddHandle(vHandle);
			}

			//m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
			if (dwBufSize > 0)
			{
				if (!PlayM4_SetStreamOpenMode(vHandle.nPort_, STREAME_REALTIME))  //设置实时流播放模式
				{
					break;
				}

				if (!PlayM4_OpenStream(vHandle.nPort_, pBuffer, dwBufSize, 10 * 1024 * 1024)) //打开流接口
				{
					break;
				}

				if (!PlayM4_Play(vHandle.nPort_, NULL)) //播放开始
				{
					break;
				}

				if (!PlayM4_SetDecCallBack(vHandle.nPort_, DecCBFun))
				{
					break;
				}
			}
			break;
		case NET_DVR_STREAMDATA:   //码流数据
			if (dwBufSize > 0 && vHandle.nPort_ != -1)
			{
				if (!PlayM4_InputData(vHandle.nPort_, pBuffer, dwBufSize))
				{
					std::cout << "error" << PlayM4_GetLastError(vHandle.nPort_) << std::endl;
					break;
				}
			}
			break;
		default: //其他数据
			if (dwBufSize > 0 && vHandle.nPort_ != -1)
			{
				if (!PlayM4_InputData(vHandle.nPort_, pBuffer, dwBufSize))
				{
					break;
				}
			}
			break;
		}
	}


	void CALLBACK VideoCapturer::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
	{
		char tempbuf[256] = { 0 };
		switch (dwType)
		{
		case EXCEPTION_RECONNECT:    //预览时重连
			printf("----------reconnect--------%d\n", time(NULL));
			break;
		default:
			break;
		}
	}
}





