#pragma once
#include "Graphics.h"
#include <vector>

class Scene;
class ComponentCamera;
class FrameBuffer;
class Texture;

class ShaderProgram;
class Object;

using std::vector;
using glm::vec3;

class SceneRenderer
{
public:
	SceneRenderer();

	void DrawGUI();
	void DrawShadowMappingGUI(); // not used right now.

	void RenderScene(Scene* scene, ComponentCamera* cameraView);
	void RenderSceneShadowMaps(Scene* scene, ComponentCamera* camera);
	void RenderSceneObjectPick(Scene* scene, ComponentCamera* camera);
	void RenderSceneGizmos(Scene* scene, ComponentCamera* camera);
	void DrawBackBuffer();

	static bool ShouldCull(vec3 position);
	void SetCullingCamera(int sceneCameraIndex);
	
	// General Config
	static bool msaaEnabled;
	static bool ssaoEnabled;
	static ComponentCamera* frustumCullingCamera;
	static float frustumCullingForgiveness;

protected:
	// Render Buffers
	FrameBuffer* frameBufferRaw;
	FrameBuffer* frameBufferBlit;
	FrameBuffer* frameBufferProcessed;

	// VSync
	bool vsyncEnabled = true;

	// Frustum Culling
	bool frustumCullingEnabled = true;
	int frustumCullingCameraIndex = -1;


	// MSAA
	int msaaSamples = 4;

	// SSAO
	float ssaoRadius = 0.5f;
	float ssaoBias = 0.025f;
	int ssaoKernelTaps = 16;
	FrameBuffer* ssaoGBuffer;
	FrameBuffer* ssaoFBO;
	FrameBuffer* ssaoBlurFBO;
	FrameBuffer* ssaoBlurFBO2;
	bool ssaoGaussianBlur = true;
	bool ssaoBlur = true;
	Texture* ssaoNoiseTexture;
	vector<glm::vec3> ssaoKernel;
	vector<glm::vec3> ssaoNoise;
	void ssaoGenerateKernel(int size);
	void ssaoGenerateNoise();

	// Shadow Mapping
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

	FrameBuffer* shadowMapDevOutput;
	ShaderProgram* depthMapOutputShader;

	// Object Picking
	FrameBuffer* objectPickBuffer;

	// Gizmos
	ShaderProgram* gizmoShader = nullptr;
	Object* lightGizmo = nullptr; // reusable object to place the light bulb model and render it.

	// Stats
	float renderLastFrameTime = 0.0f;
	float renderTotalSamples[100];
	int sampleIndex = 0;
};

