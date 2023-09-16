#pragma once
class Camera;
class Scene;
class Window;

namespace Crawl
{
	class Dungeon;
	class DungeonPlayer;
	class DungeonEditor;
	class DungeonMenu;
	class ArtTester;
}

class Application
{
public:
	enum class Mode
	{
		Game,
		Design,
		Art,
		Programming,
		Scene
	};

	Application();
	~Application();
	void LaunchArgument(char* arg);
	void InitGame();
	void Run();
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
	
	// Dev stuff
	Crawl::DungeonEditor* dungeonEditor = nullptr;
	Crawl::ArtTester* artTester = nullptr;

	Crawl::DungeonMenu* menu = nullptr;
};