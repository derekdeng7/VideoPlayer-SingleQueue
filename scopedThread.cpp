#include "scopedThread.hpp"

namespace VideoPlayer {

	ScopedThread::ScopedThread(std::thread thd) : thd_(std::move(thd))
	{
		if (!this->thd_.joinable())
		{
			throw std::logic_error("No thread!");
		}
		std::cout << "[i] thread id :" << std::this_thread::get_id() << " construct ScopedThread..."  << std::endl;
	}

	ScopedThread::~ScopedThread()
	{
		this->thd_.join();
		std::cout << "[i] thread id :" << std::this_thread::get_id() << " deconstruct ScopedThread..." << std::endl;
	}

}