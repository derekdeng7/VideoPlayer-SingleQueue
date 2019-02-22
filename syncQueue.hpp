#ifndef VIDEOPLAYER_QYNCQUEUE_HPP
#define VIDEOPLAYER_QYNCQUEUE_HPP

#include <iostream>
#include<list>
#include <map>
#include<mutex>
#include<thread>
#include<condition_variable>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "videoCapturer.hpp"

namespace VideoPlayer {

	struct VideoHandle
	{
		VideoHandle(long lRealHandle, long nPort, VideoCapturer* vCapturer) :
			lRealHandle_(lRealHandle), nPort_(nPort), vCapturer_(vCapturer)
		{}

		long lRealHandle_;
		long nPort_;
		VideoCapturer* vCapturer_;
	};

	struct Package
	{
		Package(cv::Mat frame, std::string ip, struct tm time) :
			frame_(frame), ip_(ip), time_(time)
		{}

		std::string PrintTime() const
		{
			std::string strTime = std::to_string(time_.tm_year + 1900) + "-" + std::to_string(time_.tm_mon + 1) + "-" + std::to_string(time_.tm_mday) + " " + std::to_string(time_.tm_hour) + ":" + std::to_string(time_.tm_min) + ":" + std::to_string(time_.tm_sec);
			return strTime;
		}

		cv::Mat frame_;
		std::string ip_;
		struct tm time_;
	};

class SyncQueue
{
public:
	SyncQueue(int MaxTaskCount);
	~SyncQueue();
	SyncQueue(const SyncQueue&) = delete;
	SyncQueue& operator=(const SyncQueue&) = delete;
	
	void Put(const Package& pkg);
	void Take(std::list<Package>& list);
	void Take(Package& pkg);
	void Stop();
	bool Empty();
	bool Full();
	size_t Size();
	int Count();

private:
	bool NotFull() const;
	bool NotEmpty() const;
	void Add(const Package& pkg);

    std::list<Package> queue_; 
    std::mutex mutex_; 
    std::condition_variable notEmpty_;
    std::condition_variable notFull_; 
    int maxSize_; 

    bool needStop_; 

};

}

#endif //VIDEOPLAYER_QYNCQUEUE_HPP