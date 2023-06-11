#pragma once

class Camera;
class Scene;
class Window;

class Application
{
public:
	Application();
	~Application();
	void LaunchArgument(char* arg);
	void Run();
	void Update(float delta);
protected:
	Window* window;

	Camera* camera;
	Scene* scene;
};