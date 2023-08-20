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

	void RenderScene(Scene* scene, ComponentCamera* camera);
	void RenderSceneShadowMaps(Scene* scene, ComponentCamera* camera);
	void RenderSceneObjectPick(Scene* scene, ComponentCamera* camera);
	void RenderSceneGizmos(Scene* scene, ComponentCamera* camera);
	void DrawBackBuffer();
	
	// General Config
	static bool msaaEnabled;
	static bool ssaoEnabled;

protected:
	// Render Buffers
	FrameBuffer* frameBufferRaw;
	FrameBuffer* frameBufferBlit;
	FrameBuffer* frameBufferProcessed;

	// MSAA
	int msaaSamples = 4;

	// SSAO
	int ssaoKernelSize = 64;
	float ssaoRadius = 0.5f;
	float ssaoBias = 0.025f;
	FrameBuffer* ssaoGBuffer;
	FrameBuffer* ssaoFBO;
	FrameBuffer* ssaoBlurFBO;
	Texture* ssaoNoiseTexture;
	vector<glm::vec3> ssaoKernel;
	vector<glm::vec3> ssaoNoise;

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
	float lastRenderTimeStamp;
	float renderTime;
	float samples[100];
	int sampleIndex = 0;
};

