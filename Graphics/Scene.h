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
	void UpdateSceneEditorCamera(float deltaTime);
	void Render();
	void DrawGizmos();
	void DrawCameraToBackBuffer();
	void DrawGUI();
	void DrawGraphicsGUI();
	static void DrawCameraGUI();
	void CleanUp();

	void SaveJSON();
	void LoadJSON(string sceneName);
	void LoadJSON();
	vector<FrameBuffer*> cameras;
protected:
	Scene();

	void UpdateInputs();

	void RenderShadowMaps();
	void RenderSceneCameras();
	void RenderObjectPicking();
	void RenderEditorCamera();

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
protected:
	glm::vec3* m_pointLightPositions;
	glm::vec3* m_pointLightColours;

	// This is for the 'editor' camera
	FrameBuffer* outputCameraFrameBuffer;
	
	// selecting which camera ('s framebuffer) we're pushing to the backbuffer and rendering it
	int cameraIndex = 0;

	// Gizmo rendering
	ShaderProgram* gizmoShader = nullptr;
	Object* lightGizmo = nullptr; // reusable object to place the light bulb model and render it.

	// Object picking buffer dev
	static FrameBuffer* objectPickBuffer;
	unsigned int selectedObjectID = 0;
	Object* selectedObject = nullptr;
public:
	unsigned int objectPickedID = 0;

protected:
	bool requestedObjectSelection = false;
	glm::ivec2 requestedSelectionPosition = { 0,0 };

	// Directional light shadow map
	FrameBuffer* shadowMap;
	float orthoLeft = -100.0f;
	float orthoRight = 100.0f;
	float orthoBottom = -100.0f;
	float orthoTop = 100.0f;
	float orthoFar = 120.0f;
	float orthoNear = -120.0f;

	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;

	FrameBuffer* shadowMapDevOutput;
	ShaderProgram* depthMapOutputShader;

	float lastRenderTimeStamp;
	float renderTime;
	float samples[100];
	int sampleIndex = 0;

	// Graphics
	// MSAA
public:
	bool MSAAEnabled = true;
	int MSAASamples = 4;

	// SSAO Dev
	bool ssao_enabled = true;
	int ssao_kernelSize = 64;
	float ssao_radius = 0.5f;
	float ssao_bias = 0.025f;
public:
	static FrameBuffer* ssao_gBuffer;
	static FrameBuffer* ssao_ssaoFBO;
	static FrameBuffer* ssao_ssaoBlurFBO;
protected:
	static Texture* ssao_noiseTexture;


	static std::vector<glm::vec3> ssaoKernel;
	static std::vector<glm::vec3> ssaoNoise;

public:
	static glm::mat4 GetLightSpaceMatrix();

};