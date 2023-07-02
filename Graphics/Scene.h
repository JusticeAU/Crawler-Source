#pragma once
#include "Object.h"
#include "Light.h"
#include <vector>
#include <string>

#include "DungeonEditor.h"
#include "DungeonPlayer.h"
#include "ArtTester.h"

using std::vector;
using std::to_string;

class Model;
class ShaderProgram;
class FrameBuffer;
class Mesh;
class Camera;
class ComponentCamera;

class PostProcess;

class Scene
{
public:
	~Scene();
	static void Init();

	static Object* CreateObject(Object* parent = nullptr);
	static Object* CreateObject(string name, Object * parent = nullptr);
	static Object* DuplicateObject(Object* object);
	
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
	static unsigned int GetSelectedObject() { return s_instance->selectedObjectID; }
	static void SetSelectedObject(unsigned int selected);

	static Object* FindObjectWithID(unsigned int id);

	static bool IsDungeonEditing() { return s_instance->dungeonEditingEnabled; }

	bool drawn3DGizmo = false;;

	static Scene* s_instance;
	
	vector<Object*> objects;
	vector<Object*> gizmos;
	vector<ComponentCamera*> componentCameras;

	void Update(float deltaTime);
	void Render();
	void DrawGizmos();
	void DrawCameraToBackBuffer();
	void DrawGUI();
	void CleanUp();

	void SaveJSON();
	void LoadJSON();
	vector<FrameBuffer*> cameras;
protected:
	Scene();

	void UpdateInputs();

	void RenderShadowMaps();
	void RenderSceneCameras();
	void RenderObjectPicking();
	void RenderEditorCamera();
public:
	// Dungeon Level Editing Stuff. To be moved in to own class possibly. *******
	Crawl::Dungeon* dungeon;
	Crawl::DungeonEditor dungeonEditor;
	Crawl::DungeonPlayer dungeonPlayer;

	// Scene Previewing test stuff
	Crawl::ArtTester artTester;

public:
	bool dungeonEditingEnabled = false;
	bool isDungeonGaming = false;
	bool isArtTesting = false;

	string sceneFilename = "CrawlTest.scene";
protected:
	string sceneSubfolder = "scenes/";

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

	// This is for the 'editor' camera
	FrameBuffer* outputCameraFrameBuffer;
	
	// selecting which camera ('s framebuffer) we're pushing to the backbuffer and rendering it
	int cameraIndex = 0;

	// Gizmo rendering
	ShaderProgram* gizmoShader = nullptr;
	Object* lightGizmo = nullptr; // reusable object to place the light bulb model and render it.

	// Object picking buffer dev
	FrameBuffer* objectPickBuffer = nullptr;
	unsigned int selectedObjectID = 0;
	Object* selectedObject = nullptr;

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

	float renderTime;
public:
	static glm::mat4 GetLightSpaceMatrix();

};