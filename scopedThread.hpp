#ifndef VIDEOPLAYER_SCOPEDTHREAD_HPP
#define VIDEOPLAYER_SCOPEDTHREAD_HPP

#include <iostream>
#include <thread>

namespace VideoPlayer {

	class ScopedThread {
	public:
		ScopedThread(std::thread thd);
		~ScopedThread();
		ScopedThread(const ScopedThread&) = delete;
		ScopedThread& operator =(const ScopedThread&) = delete;

	private:
		std::thread thd_;
	};

}

#endif //VIDEOPLAYER_SCOPEDTHREAD_HPP