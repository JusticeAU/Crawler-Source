#pragma once
class Camera;
class Scene;
class Window;

namespace Crawl
{
	class Dungeon;
	class DungeonPlayer;
	class DungeonEditor;
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
		Programming
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
	
	Scene* scene;

public:
	bool developerMode = false;
	// Crawl Game and Editor Objects
	static Mode s_mode;
	Crawl::Dungeon* dungeon = nullptr;
	Crawl::DungeonPlayer* dungeonPlayer = nullptr;
	bool isDungeonGaming = false;
	
	// Dev stuff
	Crawl::DungeonEditor* dungeonEditor = nullptr;
	bool dungeonEditingEnabled = false;
	
	Crawl::ArtTester* artTester = nullptr;
	bool isArtTesting = false;
};