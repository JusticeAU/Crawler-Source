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

		void DrawGUI();

		bool isActive = false;

		static const int QTY_SCALES = 3;
		float scales[QTY_SCALES] = { 1, 0.1f, 0.01f };
		int scaleIndex = 0;

		static const int QTY_UP_AXIS = 2;
		float upAxis[QTY_UP_AXIS] = { 0, 90.0f };
		int upAxisIndex = 0;

		static const int QTY_FORWARD_AXIS = 4;
		float forwardAxis[QTY_FORWARD_AXIS] = { 0, 90.0f, 180.0f, -90.0f };
		int forwardAxisIndex = 0;

		static const int MAX_VIEW_DISTANCE = 10;
		int playerViewDistance = 1;
	};

	void ModelDropCallback(GLFWwindow* window, int count, const char** paths);
}

