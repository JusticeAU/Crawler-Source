#pragma once
#include <iostream>

static class LogUtils
{
public:
	static void Log(const char* msg) { std::cout << msg << std::endl; }
};