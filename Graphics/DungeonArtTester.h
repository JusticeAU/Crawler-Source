#pragma once
#include <iostream>
#include <vector>
#include <string>

class GLFWwindow;
class ComponentModel;
class ComponentRenderer;
class ComponentSkinnedRenderer;
class ComponentAnimator;

class Material;
class Texture;

using std::string;
using std::vector;

namespace Crawl
{
	enum class FileType
	{
		Model,
		Texture,
		Other
	};

	class ArtTester
	{
	public:
		ArtTester();

		void Activate();
		void Deactivate();

		void DrawGUI();
		void DrawGUIStaging();
		void ExportStaging(bool preview);
		void UpdateStagingFolders();

		void ModelDropCallBack(int count, const char** paths);

		static FileType GetFileType(string filename);

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

		ComponentModel* model = nullptr;
		ComponentRenderer* renderer = nullptr;
		ComponentSkinnedRenderer* rendererSkinned = nullptr;
		ComponentAnimator* animator = nullptr;
		Material* editingMaterial = nullptr;

		static ArtTester* s_instance;
		static void Refresh();

		// Staging Configuration
		int type = 0;
		string types[5] = { "Monster", "Tile", "Interactble", "Door", "Decoration"};
		string typesFolders[5] = { "monster_", "tile_", "interactable_", "door_", "decoration_"};
		string stagedType = "Monster";

		// Staging names and paths
		string stagedName = "name";	// Managed by ImGui and UpdateStagingFolders function
		string stagingFolder = "";	// staging/model/monster_name/
		string stagingPath = "";	// staging/model/monster_name/monster_
		string assetPath = "";		// crawler/model/monster_name/monster_

		vector<string> stagingModels;
		vector<string> stagingMaterials;
		vector<string> stagingTextures;
	};

	void ModelDropCallback(GLFWwindow* window, int count, const char** paths);

}

