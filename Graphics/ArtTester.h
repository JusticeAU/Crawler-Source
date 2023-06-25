#pragma once
#include <iostream>

class GLFWwindow;

namespace Crawl
{
	class ArtTester
	{
	public:
		ArtTester();

		void Activate();
		void Deactivate();

		bool isActive = false;
	};

	void ModelDropCallback(GLFWwindow* window, int count, const char** paths);
}

