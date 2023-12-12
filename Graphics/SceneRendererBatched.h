#pragma once
#include <map>
#include <vector>
#include "Graphics.h"

using std::map;
using std::vector;

class ComponentRenderer;
class ComponentCamera;
class Model;
class ShaderProgram;
class Material;

struct MeshBatch
{
	map<int, std::vector<ComponentRenderer*>> meshBatches;
};

struct ModelBatch
{
	map<Model*, MeshBatch> modelBatches;
};

struct MaterialBatch
{
	map<Material*, ModelBatch> materialBatches;
};

struct Batch
{
public:
	map<ShaderProgram*, MaterialBatch> shaderBatches;
};

// This handles draw calls created by the SceneRenderer. The SceneRenderer sends draw calls in to this system to be organised - then this system will Draw them as efficiently as it can.
class RenderBatch
{
public:
	enum class RenderPass
	{
		Shadow,
		gBuffer,
		Opaque
	};

	// Call this before sending in draw calls or calling DrawBatches. It sets a context for the kind of pass we are doing.
	void SetRenderPass(RenderPass pass) { renderPass = pass; }

	void SetCameraMatricies(ComponentCamera* camera);
	// Add a Draw Call. It will get groupped with shaders and materials.
	void AddDraw(ComponentRenderer* renderer, int subMesh);
	// Draws all batches and then clears.
	void DrawBatches();
	void ClearBatches();

	RenderPass renderPass;
	Batch renderBatch;
	
	glm::mat4 projectionViewMatrix;
	glm::mat4 viewMatrix;
	glm::vec3 cameraPosition;
};