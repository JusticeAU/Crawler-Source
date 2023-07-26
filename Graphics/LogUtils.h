#pragma once
#include <iostream>

class LogUtils
{
public:
	static void Log(const char* msg) { std::cout << msg << std::endl; }
	static void Log(const std::string msg) { std::cout << msg << std::endl; }
};