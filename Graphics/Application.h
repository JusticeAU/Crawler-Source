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
		Developer,
		Scene
	};

	Application();
	~Application();
	void LaunchArgumentPreLoad(const char* arg);
	void LaunchArgumentPostLoad(const char* arg);
	void ConstructWindow();
	void LoadResourceManagers();
	void DoLoadingScreen();
	void PreloadAssetsAndRenderProgress();
	void RenderLoadProgress(float percentageLoaded);
	void InitialiseAdditionalGameAssets();
	void InitialiseMenu();
	void InitialiseInput();
	void Run();

	void HandleDevelopmentModeToggle();

	void RefreshImGui();
	void RenderImGui();

	void SwapBuffers();

	void Update(float delta);
protected:
	Window* window;
	Camera* camera;

public:
	bool developerModeLaunch = false;
	bool developerModeDrawGUI = true;
	int quality = 3; // 0-2 Low-High, 3 == Auto.

	// Crawl Game and Editor Objects
	static Mode s_mode;
	static Mode s_modeOld;
	Crawl::Dungeon* dungeon = nullptr;
	Crawl::DungeonPlayer* dungeonPlayer = nullptr;
	Crawl::DungeonGameManager* gameManager = nullptr;
	Crawl::DungeonEditor* dungeonEditor = nullptr;
	
	Crawl::ArtTester* artTester = nullptr;
	Crawl::DungeonMenu* menu = nullptr;

};