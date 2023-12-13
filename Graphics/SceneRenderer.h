#pragma once
#include "Graphics.h"
#include "SceneRendererBatched.h"
#include "Mesh.h"
#include <vector>
#include "BillboardList.h"

class Scene;
class ComponentCamera;
class ComponentRenderer;
class ComponentBillboard;
class ComponentParticleSystem;

class FrameBuffer;
class Texture;
class CameraFrustum;

class ShaderProgram;
class Object;

class UniformBuffer;

using std::vector;
using glm::vec3;

// SceneRenderer will take a scene and run it through the rendering pipeline!
class SceneRenderer
{
public:
	SceneRenderer();
	
	void SetQuality();

	void DrawGUI();
	void DrawStatistics();
	void DrawShadowCubeMappingGUI();
	void DrawShadowMappingGUI(); // not used right now.

	void Prepare(Scene* scene);

	void RenderScene(Scene* scene, ComponentCamera* cameraView);
	void RenderSceneShadowCubeMaps(Scene* scene);
	void RenderSceneShadowMaps(Scene* scene, ComponentCamera* camera);
	void RenderSceneObjectPick(Scene* scene, ComponentCamera* camera);
	void RenderTransparent(Scene* scene, ComponentCamera* camera);
	
	void RenderParticleSystems(ComponentCamera* camera);

	void RenderSceneGizmos(Scene* scene, ComponentCamera* camera);
	
	void RenderLines(ComponentCamera* camera);

	void CleanUp(Scene* scene);
	
	void DrawBackBuffer();

	static bool compareIndexDistancePair(std::pair<int, float> a, std::pair<int, float> b);

	static bool ShouldCull(const glm::vec3* points);
	void SetCullingCamera(int sceneCameraIndex);

	void SetStaticShadowMapsDirty() { pointLightShadowMapsStaticDirty = true; };
	
	// General Config
	static bool fxaaEnabled;
	static bool ssaoEnabled;
	static bool bloomEnabled;
	static ComponentCamera* frustumCullingCamera;
	static CameraFrustum* cullingFrustum;
	static bool frustumCullingShowBounds;
	
	static bool currentPassIsStatic;
	static bool currentPassIsSplit;

	static float ambient;
	static float gamma;

	static float shadowMapRealtimeMaxDistance;

	static int ssaoKernelTaps;

	// This stores the batches for the Opaque pass. May get scoped out to other passes.
	static RenderBatch renderBatch;

	// Billboard stuff
	static void AddBillBoardDraw(ComponentBillboard* billboard);
	static std::map<Texture*, BillboardList*> billboardLists;

	// Particle stuff!
	static void AddParticleDraw(ComponentParticleSystem* ps);
	static std::vector<ComponentParticleSystem*> particleSystems;

protected:
	// Render Buffers
	FrameBuffer* frameBufferRaw;
	FrameBuffer* frameBufferProcessed;
	FrameBuffer* frameBufferCurrent = nullptr;
	FrameBuffer* gBuffer;

	// VSync
	bool vsyncEnabled = true;

	// Frustum Culling
	bool frustumCullingEnabled = true;
	int frustumCullingCameraIndex = -1;

	// SSAO
	float ssaoRadius = 0.25f;
	float ssaoBias = 0.025f;

	FrameBuffer* ssaoFBO;
	FrameBuffer* ssaoPingFBO;
	FrameBuffer* ssaoPongFBO;
	FrameBuffer* ssaoPostProcess;
	bool ssaoGaussianBlur = true;
	bool ssaoBlur = true;
	Texture* ssaoNoiseTexture;
	vector<glm::vec3> ssaoKernel;
	vector<glm::vec3> ssaoNoise;
	void ssaoGenerateKernel(int size);
	void ssaoGenerateNoise();

	// Bloom
	FrameBuffer* bloomPingFBO;
	FrameBuffer* bloomPongFBO;
	int bloomBlurTaps = 2;

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
	bool showAdvancedStats = false;
	float renderLastFrameTime = 0.0f;
	float renderTotalSamples[100];
	int sampleIndex = 0;
public:
	struct Statistics
	{
		int tris = 0;
		int drawCalls = 0;
		int materialBatches = 0;
		int shaderBatches = 0;
	};
	static Statistics statistic;

	// Point Light Shadow Map Dev
public:
	static FrameBuffer* pointlightCubeMapArrayStatic;
	static FrameBuffer* pointlightCubeMapArrayDynamic;
	UniformBuffer* pointLightPositionBuffer;
	UniformBuffer* pointLightColourBuffer;
	bool pointLightShadowMapsStaticDirty = true;

	struct CameraDirection
	{
		GLenum CubemapFace;
		glm::vec3 target;
		glm::vec3 up;
	};

	CameraDirection cubeMapDirections[6] =
	{
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f) },
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f) },
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f) }
	};

	float FOV = 90.0f;
	float aspect = 1.0f;
	float nearNum = 0.1f;
	float farNum = 15.0f;

	// Transparent Rendering Dev - This will get factored in to the material batcher system.
	static vector<std::pair<ComponentRenderer*, int>> transparentCalls;
};

