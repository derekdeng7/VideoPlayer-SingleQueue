 #ifndef VIDEOPLAYER_VIDEOCAPTURER_HPP
#define VIDEOPLAYER_VIDEOCAPTURER_HPP

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "HCNetSDK.h"
#include "plaympeg4.h"

namespace VideoPlayer {

	class VideoCapturer
	{
	public:	
		VideoCapturer(std::string host, int port, std::string username, std::string password);
		~VideoCapturer();
		void start();
		void stop();
		std::string getHost() const;
		void show(cv::Mat frame);

		static void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
		static void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
		static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);

	private:
		void startCamera();
		void stopCamera();

		std::string host_;
		int port_;
		std::string username_;
		std::string password_;

		LONG lRealPlayHandle;
		LONG lUserID;
	};
}

#endif //VIDEOPLAYER_VIDEOCAPTURER_HPP