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

		void Update(float delta);

		void Activate();
		void Deactivate();

		void DrawGUI();
		void DrawGUIStaging();
		void ExportStaging(bool preview);
		void UpdateStagingFolders();

		void ModelDropCallBack(int count, const char** paths);

		static FileType GetFileType(string filename);

		static const int MAX_VIEW_DISTANCE = 10;
		int playerViewDistance = 1;

		bool hasAnimations = true;

		ComponentModel* model = nullptr;
		ComponentRenderer* renderer = nullptr;
		ComponentAnimator* animator = nullptr;
		Material* editingMaterial = nullptr;

		bool stagingWindowOpen = false;

		static ArtTester* s_instance;
		static void Refresh();

		// Staging Configuration
		float timeLastRefreshed = 0.0f;
		int type = 0;
		string types[6] = { "Monster", "Tile", "Interactble", "Door", "Decoration", "Other"};
		string typesFolders[6] = { "monster_", "tile_", "interactable_", "door_", "decoration_", "" };
		string stagedType = "Monster";

		// Staging names and paths
		string stagedName = "name";	// Managed by ImGui and UpdateStagingFolders function
		//string stagingFolder = "";	// staging/model/monster_name/
		//string stagingPath = "";	// staging/model/monster_name/monster_
		
		string modelPath = "";
		string materialPath = "";
		string texturePath = "";		// crawler/texture/monster_

		vector<string> stagingModels;
		vector<string> stagingMaterials;
		vector<string> stagingTextures;
	};

	void ModelDropCallback(GLFWwindow* window, int count, const char** paths);

}

