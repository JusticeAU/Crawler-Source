//#define AIE_SHOWCASE

#pragma once
class Camera;
class Scene;
class Window;

namespace Crawl
{
	class Dungeon;
	class DungeonPlayer;
	class DungeonGameManager;
	class DungeonEditor;
	class DungeonMenu;
	class ArtTester;
}

class Application
{
public:
	enum class Mode
	{
		Menu,
		Game,
		Design,
		Art,
		Programming,
		Scene
	};

	Application();
	~Application();
	void LaunchArgumentPreLoad(char* arg);
	void LaunchArgumentPostLoad(char* arg);
	void ConstructWindow();
	void LoadResourceManagers();
	void DoLoadingScreen();
	void PreloadAssetsAndRenderProgress();
	void RenderLoadProgress(float percentageLoaded);
	void InitialiseAdditionalGameAssets();
	void InitialiseMenu();
	void Run();

	void RefreshImGui();
	void RenderImGui();

	void SwapBuffers();

	void Update(float delta);
protected:
	Window* window;
	Camera* camera;

public:
	bool developerMode = false;
	// Crawl Game and Editor Objects
	static Mode s_mode;
	static Mode s_modeOld;
	Crawl::Dungeon* dungeon = nullptr;
	Crawl::DungeonPlayer* dungeonPlayer = nullptr;
	Crawl::DungeonGameManager* gameManager = nullptr;
	
	// Dev stuff
	Crawl::DungeonEditor* dungeonEditor = nullptr;
	Crawl::ArtTester* artTester = nullptr;

	Crawl::DungeonMenu* menu = nullptr;
};