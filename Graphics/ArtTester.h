#pragma once
#include <iostream>
#include <string>

class GLFWwindow;
class ComponentRenderer;
class ComponentSkinnedRenderer;
class ComponentAnimator;
class Material;

using std::string;

namespace Crawl
{
	class ArtTester
	{
	public:
		ArtTester();

		void Activate();
		void Deactivate();

		void DrawGUI();
		void DrawGUIStaging();

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

		bool hasAnimations = true;
		ComponentRenderer* renderer = nullptr;
		ComponentSkinnedRenderer* rendererSkinned = nullptr;
		ComponentAnimator* animator = nullptr;
		Material* editingMaterial = nullptr;

		static ArtTester* s_instance;
		static void Refresh();

		// Test Staging config
		string types [3] = { "Monster", "Tile", "Interactble" };
		string stagedType = "Monster";
	};

	void ModelDropCallback(GLFWwindow* window, int count, const char** paths);
}

