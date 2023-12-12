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
class ComponentLightPoint;
class ComponentRenderer;

class PostProcess;

class SceneEditorCamera;
class SceneRenderer;

class Scene
{
public:
	~Scene();
	static void Init();

	// Creates a new Scene and adds it to the s_instances map.
	static Scene* NewScene(string name);
	// Sets a particular scene as the active scene. Has no protections for null scenes. Use carfully.
	static void ChangeScene(string name) { s_instance = s_instances[name]; SetClearColour(); }

	// Initialises the editor camera. It's a camera that doesnt exist in the gameworld. Uses camera index -1 and has input controls available when in the scene graph viewer.
	static void CreateSceneEditorCamera();

	static Object* CreateObject(Object* parent = nullptr);
	static Object* CreateObject(string name, Object * parent = nullptr);
	// Clones the object, its children and all components - Returns the new object.
	static Object* DuplicateObject(const Object* object, Object* newParent);
	
	// Applies the scene specific clear colour  to the OpenGL context.
	static void SetClearColour();
	// Sets a new clear colour and applies it to the OpenGL context.
	static void SetClearColour(vec3 clearColour);
	
	static vec3 GetSunColour();
	static void SetSunColour(vec3 sunColour);
	
	static vec3 GetSunDirection();
	static void SetSunDirection(vec3 sunDirection);

	static vec3 GetAmbientLightColour();
	static void SetAmbientLightColour(vec3 ambientColour);

	static int GetNumPointLights();
	static glm::vec4* GetPointLightPositions() { return &s_instance->m_pointLightPositions[0]; }
	static glm::vec4* GetPointLightColours() { return &s_instance->m_pointLightColours[0]; }
	static void AddPointLight(ComponentLightPoint* light);
	static void RemovePointLight(ComponentLightPoint* light);

	static void AddRendererComponent(ComponentRenderer* rendererComponent);
	static void RemoveRendererComponent(ComponentRenderer* rendererComponent);

	static int GetCameraIndex() { return s_instance->cameraIndex; }
	static ComponentCamera* GetCameraByIndex(int index);
	static ComponentCamera* GetCurrentCamera();
	static void SetCameraIndex(int index);
	static void SetCameraByName(string name = "Editor Camera");
	static unsigned int GetSelectedObject() { return s_instance->selectedObjectID; }
	static void SetSelectedObject(unsigned int selected);
	static void RequestObjectSelection() { s_instance->requestedObjectSelection = true; };

	static Object* FindObjectWithID(unsigned int id);
	// Searches the Scene hierarchy for an object with the name objectName. Try to set an ID to an object and search for it by that instead.
	static Object* FindObjectWithName(string objectName);

	// Tells the renderer that static objects may have moved and it should refresh any cached shadow maps.
	void SetStaticObjectsDirty() { rendererShouldRefreshStaticMaps = true; };
	void SetAllObjectsStatic();

	// Deprecated funtionality to try the 3D gizmo in the scene editor. Do not use.
	bool drawn3DGizmo = false;

	static Scene* s_instance;
	static unordered_map<string, Scene*> s_instances;
	string sceneName = "";
	
	vector<Object*> objects;
	vector<Object*> gizmos;
	
	bool drawGizmos = false;
	static SceneEditorCamera* s_editorCamera;
	vector<ComponentCamera*> componentCameras;

	bool rendererShouldRefreshStaticMaps = true;
	int staticObjects = 0;
	int dynamicObjects = 0;

	string GetSceneName() { return s_instance->sceneName; };

	void Update(float deltaTime);
	void UpdatePointLightData();

	void Render();
	void RenderBakedShadowMaps();
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
	int MAX_POINTLIGHTS = 20;
public:
	vector<ComponentLightPoint*> m_pointLightComponents;
	vector<ComponentRenderer*> m_rendererComponents;
	static SceneRenderer* renderer;
	glm::mat4 lightSpaceMatrix; // Directional Light Shadow Mapping.

protected:
	glm::vec4* m_pointLightPositions;
	glm::vec4* m_pointLightColours;
	
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