#pragma once
#include <iostream>

class LogUtils
{
public:
	static void Log(const char* msg)
	{
#ifndef RELEASE
		std::cout << msg << std::endl;
#endif // !RELEASE
	}
	static void Log(const std::string msg)
	{
#ifndef RELEASE
		std::cout << msg << std::endl;
#endif // !RELEASE
	}
};