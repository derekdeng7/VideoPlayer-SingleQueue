#include "threadPool.hpp"

const int MaxTaskCount = 100;

namespace VideoPlayer {

	ThreadPool::Object_Creator ThreadPool::object_creator_;

	ThreadPool::ThreadPool() : syQueue_(MaxTaskCount)
	{
		//int numThreads = std::thread::hardware_concurrency();
	}

	ThreadPool::~ThreadPool(void)
	{
		Stop();
	}

	void ThreadPool::Start(int numThreads)
	{
		running_ = true;
		for (int i = 0; i < numThreads; ++i)
		{
			threadgroup_.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
		}
	}

	void ThreadPool::Stop()
	{
		std::call_once(flag_, [this] {StopThreadGroup(); }); 
	}

	void ThreadPool::AddTask(const Package& pkg)
	{
		syQueue_.Put(pkg);
	}

	void ThreadPool::AddHandle(VideoHandle vHandle)
	{
		std::unique_lock< std::mutex> locker(handleMutex_);
		videoHandleVec_.push_back(vHandle);
	}

	void ThreadPool::DeleteHandle(std::string ip)
	{
		std::unique_lock< std::mutex> locker(handleMutex_);
		for (auto iter = videoHandleVec_.begin(); iter != videoHandleVec_.end(); iter++)
		{
			if ((*iter).vCapturer_->getHost() == ip)
			{
				videoHandleVec_.erase(iter);
				return;
			}
		}
	}

	VideoHandle ThreadPool::getHandleBylRealHandle(long lRealHandle)
	{
		for (auto& iter : videoHandleVec_)
		{
			if (iter.lRealHandle_ == lRealHandle)
				return iter;
		}
		return VideoHandle(-1, -1, nullptr);
	}

	VideoHandle ThreadPool::getHandleByPort(long nPort)
	{
		for (auto& iter : videoHandleVec_)
		{
			if (iter.nPort_ == nPort)
				return iter;
		}
		return VideoHandle(-1, -1, nullptr);
	}

	void ThreadPool::RunInThread()
	{
		while (running_)
		{
			std::list<Package> list;
			syQueue_.Take(list);

			for (auto& pkg : list)
			{
				if (!running_)
					return;
				std::cout << "[i] thread id :" << std::this_thread::get_id() << " receives frame..." << pkg.PrintTime() << std::endl;

				cv::imshow(pkg.ip_, pkg.frame_);
				cvWaitKey(1);
			}
		}
	}

	void ThreadPool::StopThreadGroup()
	{
		syQueue_.Stop(); 
		running_ = false;

		for (auto thread : threadgroup_)
		{
			if (thread)
				thread->join();
		}
		threadgroup_.clear();
	}

}
