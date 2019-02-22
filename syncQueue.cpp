#include "syncQueue.hpp"

namespace VideoPlayer {

	SyncQueue::SyncQueue(int maxSize) : maxSize_(maxSize), needStop_(false)
	{
		
	}

	SyncQueue::~SyncQueue()
	{
		std::cout << "[i] thread id :" << std::this_thread::get_id() << " deconstructor SyncQueue..." << std::endl;
	}

	void SyncQueue::Put(const Package& x)
	{
		Add(x);
	}

	void SyncQueue::Take(std::list<Package>& list)
	{
		std::unique_lock<std::mutex> locker(mutex_);
		notEmpty_.wait(locker, [this] {return needStop_ || NotEmpty(); });

		if (needStop_)
			return;
		list = std::move(queue_);
		notFull_.notify_one();
	}

	void SyncQueue::Take(Package& pkg)
	{
		std::unique_lock<std::mutex> locker(mutex_);
		notEmpty_.wait(locker, [this] {return needStop_ || NotEmpty(); });

		if (needStop_)
			return;
		pkg = queue_.front();
		queue_.pop_front();
		notFull_.notify_one();
	}

	void SyncQueue::Stop()
	{
		{
			std::lock_guard<std::mutex> locker(mutex_);
			needStop_ = true;
		}
		notFull_.notify_all();
		notEmpty_.notify_all();
	}

	bool SyncQueue::Empty()
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.empty();
	}

	bool SyncQueue::Full()
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size() == maxSize_;
	}

	size_t SyncQueue::Size()
	{
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size();
	}

	int SyncQueue::Count()
	{
		return queue_.size();
	}

	bool SyncQueue::NotFull() const
	{
		bool full = queue_.size() >= maxSize_;
		if (full)
			std::cout << "full, waiting, thread id: " << std::this_thread::get_id() << std::endl;
		return !full;
	}

	bool SyncQueue::NotEmpty() const
	{
		bool empty = queue_.empty();
		if (empty)
			std::cout << "empty, waiting, thread id: " << std::this_thread::get_id() << std::endl;
		return !empty;
	}

	void SyncQueue::Add(const Package& pkg)
	{
		std::unique_lock< std::mutex> locker(mutex_);
		notFull_.wait(locker, [this] {return needStop_ || NotFull(); });
		if (needStop_)
			return;

		queue_.push_back(pkg);
		notEmpty_.notify_one();
	}

}