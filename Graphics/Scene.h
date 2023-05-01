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
	static unsigned int GetSelectedObject() { return s_instance->selectedObject; }
	bool drawn3DGizmo = false;;

	static Scene* s_instance;
	
	vector<Object*> objects;
	vector<Object*> gizmos;
	vector<ComponentCamera*> componentCameras;

	void Update(float deltaTime);
	void DrawObjects();
	void DrawGizmos();
	void DrawCameraToBackBuffer();
	void DrawGUI();
	void CleanUp();

	void Save();
	void Load();
	vector<FrameBuffer*> cameras;
protected:
	Scene();
	string sceneSubfolder = "scenes/";
	string sceneFilename = "default.scene";

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
	unsigned int selectedObject = 0;
	bool requestedObjectSelection = false;
	glm::ivec2 requestedSelectionPosition = { 0,0 };

	// Directional light shadow map
	FrameBuffer* shadowMap;
	float orthoLeft = -50.0f;
	float orthoRight = 50.0f;
	float orthoBottom = -50.0f;
	float orthoTop = 50.0f;
	float orthoFar = 50.0f;
	float orthoNear = -50.0f;
	vec3 orthoLookAt = { -2.0f, 4.0f, -1.0f };
	vec3 orthoPosition = { 0,9,0 };

	glm::mat4 lightProjection;
	glm::mat4 lightView;
	glm::mat4 lightSpaceMatrix;

	FrameBuffer* shadowMapDevOutput;
	ShaderProgram* depthMapOutputShader;

	float renderTime;
public:
	static glm::mat4 GetLightSpaceMatrix();

};