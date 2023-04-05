#pragma once

struct GLFWwindow;
class Camera;
class Scene;

class Application
{
public:
	Application();
	~Application();
	void Run();
	void Update(float delta);
protected:
	GLFWwindow* window;

	Camera* camera;
	Scene* scene;
};