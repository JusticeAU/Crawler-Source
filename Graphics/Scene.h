#pragma once
#include "Object.h"
#include <vector>
#include <string>

using std::vector;
using std::to_string;

class Scene
{
public:
	Scene();
	~Scene();
	static Object* CreateObject(Object* parent = nullptr);
	static Object* CreateObject(string name, Object * parent = nullptr);
	static void SetClearColour(vec3 clearColour);

	static Scene* s_instance;
	
	vector<Object*> objects;
	void Update(float deltaTime);
	void DrawObjects();
	void DrawGUI();
	void CleanUp();
protected:
	vec3 clearColour;
	int objectCount = 0;
};