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

class RenderBatch
{
public:
	enum class RenderPass
	{
		Shadow,
		gBuffer,
		Opaque
	};

	void SetRenderPass(RenderPass pass) { renderPass = pass; }

	void SetCameraMatricies(ComponentCamera* camera);
	void AddDraw(ComponentRenderer* renderer, int subMesh);
	void DrawBatches();
	void ClearBatches();

	RenderPass renderPass;
	Batch renderBatch;
	
	glm::mat4 projectionViewMatrix;
	glm::mat4 viewMatrix;
	glm::vec3 cameraPosition;
};