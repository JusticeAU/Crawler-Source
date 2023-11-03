#pragma once
#include <map>
#include <vector>
#include "Graphics.h"

using std::map;
using std::vector;

class ComponentRenderer;
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
	void SetMatricies(glm::mat4 projectionView, glm::vec3 position);
	void AddDraw(ComponentRenderer* renderer, int subMesh);
	void DrawBatches();
	void ClearBatches();

	Batch renderBatch;
	glm::mat4 pv;
	glm::vec3 cameraPosition;
};