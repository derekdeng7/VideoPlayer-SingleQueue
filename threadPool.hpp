#ifndef VIDEOPLAYER_THREADPOOL_HPP
#define VIDEOPLAYER_THREADPOOL_HPP

#include <vector>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include"syncQueue.hpp"

namespace VideoPlayer {

	class ThreadPool
	{
	public:
		static ThreadPool* getInstance()
		{
			static ThreadPool instance;
			return &instance;
		}

		void Start(int numThreads);
		void Stop();
		void AddTask(const Package& pkg);
		void AddHandle(VideoHandle vHandle);
		void DeleteHandle(std::string ip);
		VideoHandle getHandleBylRealHandle(long lRealHandle);
		VideoHandle getHandleByPort(long nPort);

	private:
		struct  Object_Creator
		{
			Object_Creator()
			{
				ThreadPool::getInstance();
			};
		};
		static Object_Creator object_creator_;

		ThreadPool();
		~ThreadPool();
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator =(const ThreadPool&) = delete;
		void RunInThread();
		void StopThreadGroup();

		std::list<std::shared_ptr<std::thread>> threadgroup_; 
		SyncQueue syQueue_; 
		std::atomic_bool running_;
		std::once_flag flag_;
		std::mutex handleMutex_; 
		std::vector<VideoHandle> videoHandleVec_;
	};
}

#endif //VIDEOPLAYER_THREADPOOL_HPP
