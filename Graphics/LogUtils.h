#pragma once
#include <iostream>

class LogUtils
{
public:
	static void Log(const char* msg) { std::cout << msg << std::endl; }
};