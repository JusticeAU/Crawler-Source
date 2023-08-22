#pragma once
#include "Object.h"
#include "Light.h"
#include <vector>
#include <unordered_map>
#include <string>

using std::vector;
using std::unordered_map;
using std::to_string;

class Model;
class ShaderProgram;
class FrameBuffer;
class Mesh;
class Camera;
class ComponentCamera;

class PostProcess;

class SceneEditorCamera;
class SceneRenderer;

class Scene
{
public:
	~Scene();
	static void Init();

	static Scene* NewScene(string name);
	static void ChangeScene(string name) { s_instance = s_instances[name]; SetClearColour(); }
	static void CreateSceneEditorCamera();

	static Object* CreateObject(Object* parent = nullptr);
	static Object* CreateObject(string name, Object * parent = nullptr);
	static Object* DuplicateObject(Object* object, Object* newParent);
	
	static void SetClearColour();
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

	static int GetCameraIndex() { return s_instance->cameraIndex; }
	static ComponentCamera* GetCameraByIndex(int index);
	static ComponentCamera* GetCurrentCamera();
	static void SetCameraIndex(int index);
	static void SetCameraByName(string name = "Editor Camera");
	static unsigned int GetSelectedObject() { return s_instance->selectedObjectID; }
	static void SetSelectedObject(unsigned int selected);
	static void RequestObjectSelection() { s_instance->requestedObjectSelection = true; };

	static Object* FindObjectWithID(unsigned int id);
	static Object* FindObjectWithName(string objectName);

	bool drawn3DGizmo = false;;

	static Scene* s_instance;
	static unordered_map<string, Scene*> s_instances;
	string sceneName = "";
	
	vector<Object*> objects;
	vector<Object*> gizmos;
	
	bool drawGizmos = false;
	static SceneEditorCamera* s_editorCamera;
	vector<ComponentCamera*> componentCameras;

	string GetSceneName() { return s_instance->sceneName; };

	void Update(float deltaTime);
	void Render();
	void DrawGUI();
	static void DrawCameraGUI();
	void CleanUp();

	void SaveJSON();
	void LoadJSON(string sceneName);
	void LoadJSON();
protected:
	Scene(string name);

	string sceneFilename = "CrawlTest.scene";
protected:
	string sceneSubfolder = "scenes/";

	vec3 clearColour = { 0.25f, 0.25f, 0.25f };;
	int objectCount = 0;

	// Directional/SunLight
	vec3 m_sunDirection = {0, -0.707f, -0.707f };
	vec3 m_sunColour = { 1,1,1 };

	// Ambient Light
	vec3 m_ambientColour = { 0.25f, 0.25f, 0.25f };

	// Point Lights
	int MAX_LIGHTS = 4;
public:
	vector<Light> m_pointLights;
	static SceneRenderer* renderer;
	glm::mat4 lightSpaceMatrix; // Directional Light Shadow Mapping.

protected:
	glm::vec3* m_pointLightPositions;
	glm::vec3* m_pointLightColours;
	
	// selecting which camera we should render.
	int cameraIndex = 0;
	ComponentCamera* cameraCurrent = nullptr;

public:
	// Object Picking API
	bool requestedObjectSelection = false;
	glm::ivec2 requestedSelectionPosition = { 0,0 };
	unsigned int objectPickedID = 0;

	// Scene Editor Object Selection
	unsigned int selectedObjectID = 0;
	Object* selectedObject = nullptr;
public:
	static glm::mat4 GetLightSpaceMatrix();
};