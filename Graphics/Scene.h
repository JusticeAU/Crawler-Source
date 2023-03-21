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
	static Scene* s_instance;
	vec3 clearColour;
	vector<Object*> objects;
	void Update(float deltaTime);
	void DrawObjects();
	void DrawGUI();
	void CleanUp();
	int objectCount = 0;
};