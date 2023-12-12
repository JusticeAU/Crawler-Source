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
	// Handle arguments pre manager loading
	void LaunchArgumentPreLoad(const char* arg);
	// handle argument post manager loading
	void LaunchArgumentPostLoad(const char* arg);
	// Constructs the GLFW window - behaves differently depending on if a 'developer mode' argument launched the game.
	void ConstructWindow();
	void LoadResourceManagers();
	// This function will get the Texture Manager to preload a texture, then render a progress bar and loop until all textures are preloaded.
	void DoLoadingScreen();
	void PreloadAssetsAndRenderProgress();
	void RenderLoadProgress(float percentageLoaded);

	// Initialises all Briar Mansion assets not intialised by the engine itself.
	void InitialiseAdditionalGameAssets();
	void InitialiseMenu();

	// Initialises the Input system but also sets up the key Aliases, and injects the dance pat binding.
	void InitialiseInput();

	// Main game loop
	void Run();

	// Toggles in and out of the engine state - maintaining knowledge of the state it came from.
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
	int quality = 3; // 0-2 Low-High, 3 == Auto. - Used primarily by the texture manager, can be override with commandline arguments.

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