#pragma once
#include "Component.h"

#include <string>
#include <vector>

class Camera;
class Model;
class ShaderProgram;
class Texture;
class Material;
class FrameBuffer;
class ComponentAnimator;
class ComponentAnimationBlender;
class UniformBuffer;

using std::string;
using std::vector;

const int RENDERER_MAX_LIGHTS = 4;

class ComponentRenderer : public Component
{
public:
	ComponentRenderer(Object* parent);
	ComponentRenderer(Object* parent, nlohmann::ordered_json j);
	~ComponentRenderer();

	void Update(float delta) override;
	void UpdateClosestLights();

	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void DrawGUI() override;

	virtual void OnParentChange() override;

	void BindShader();
	void BindMatricies(mat4 pv, vec3 position);
	void SetUniforms();
	void ApplyMaterials();
	void DrawModel();
	void BindBoneTransform();

	Component* Clone(Object* parent);

	void DebugDrawBoundingBox(int meshIndex, vec3 colour = { 1.0, 1.0, 1.0 });
public:
	Model* model = nullptr;
	bool isAnimated = false;
	
	vector<Material*> submeshMaterials;
	Material* material = nullptr;

	// Every mesh has an AABB, but mostly meshes will be involved in a scene hierarchy, so we should take the AABB and combine it with our model matrix to go get an OBB.
	// We cache it here for each instance of a submesh.
	struct Bounds
	{
		glm::vec3 points[8];
	};
	vector<Bounds> submeshBounds;

	bool castsShadows = true;
	bool receivesShadows = true;
	float shadowBias = 0.005f;
	bool dontFrustumCull = false;
	float emissiveScale = 1.0f;

	bool modifiedMaterials = false; // This is used for Editor APIs to see if it needs consider reserialising the model configuration.

	ComponentAnimator* animator = nullptr;
	ComponentAnimationBlender* animationBlender = nullptr;

	int closestPointLightIndices[8] = {-1, -1, -1, -1, -1, -1, -1, -1 };
	UniformBuffer* closestPointLightUBO;
};