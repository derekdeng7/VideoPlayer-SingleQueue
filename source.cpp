#include <iostream>
#include<windows.h>
#include <chrono>

#include "videoCapturer.hpp"
#include "scopedThread.hpp"
#include "threadPool.hpp"

using namespace VideoPlayer;

int main()
{
	VideoCapturer vc1("192.168.1.64", 8000, "admin", "admin123");
	VideoCapturer vc2("192.168.1.65", 8000, "admin", "admin123");
	VideoCapturer vc3("192.168.1.66", 8000, "admin", "admin123");

	ThreadPool* thdPool = ThreadPool::getInstance();
	thdPool->Start(1);

	vc1.start();
	vc2.start();
	vc3.start();

	std::cout << "press any key to exit..." << std::endl;
	std::cin.get();

	vc1.stop();
	vc2.stop();
	vc3.stop();

	std::cout << "Finish..." << std::endl;

	return 0;
}