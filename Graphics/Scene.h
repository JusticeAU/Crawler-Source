#pragma once
#include "Object.h"
#include "Light.h"
#include <vector>
#include <string>

using std::vector;
using std::to_string;

class Model;
class ShaderProgram;
class FrameBuffer;

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

	static vec3 GetAmbientLightColour();
	static void SetAmbientLightColour(vec3 ambientColour);

	static int GetNumPointLights();
	static glm::vec3* GetPointLightPositions() { return &s_instance->m_pointLightPositions[0]; }
	static glm::vec3* GetPointLightColours() { return &s_instance->m_pointLightColours[0]; }


	static Scene* s_instance;
	
	vector<Object*> objects;
	void Update(float deltaTime);
	void DrawObjects();
	void DrawGizmos();
	void DrawGUI();
	void CleanUp();

	void Save();
	void Load();
protected:
	Scene();
	vec3 clearColour;
	int objectCount = 0;

	// Directional/SunLight
	vec3 m_sunDirection = {0, -0.707f, -0.707f };
	vec3 m_sunColour = { 1,1,1 };

	// Ambient Light
	vec3 m_ambientColour = { 0.25f, 0.25f, 0.25f };

	// Point Lights
	int MAX_LIGHTS = 4;
	vector<Light> m_pointLights;
	glm::vec3* m_pointLightPositions;
	glm::vec3* m_pointLightColours;

	Object* lightGizmo = nullptr;
	//Model* lightGizmoModel = nullptr;
	ShaderProgram* lightGizmoShader = nullptr;

	// Frame Buffer Test
	FrameBuffer* fb;

};