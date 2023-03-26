#pragma once
#include "Object.h"
#include <vector>
#include <string>

using std::vector;
using std::to_string;

class Scene
{
public:
	~Scene();
	static void Init();

	static Object* CreateObject(Object* parent = nullptr);
	static Object* CreateObject(string name, Object * parent = nullptr);
	
	static void SetClearColour(vec3 clearColour);
	
	static vec3 GetSunColour();
	static void SetSunColour(vec3 sunColour);
	
	static vec3 GetSunDirection();
	static void SetSunDirection(vec3 sunDirection);


	static Scene* s_instance;
	
	vector<Object*> objects;
	void Update(float deltaTime);
	void DrawObjects();
	void DrawGUI();
	void CleanUp();
protected:
	Scene();
	vec3 clearColour;
	int objectCount = 0;

	// Directional/SunLight
	vec3 m_sunDirection = {0, -1, 0};
	vec3 m_sunColour = { 1,1,1 };
};