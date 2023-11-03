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
public:
	Model* model = nullptr;
	
	vector<Material*> materialArray;
	Material* material = nullptr;
	bool isAnimated = false;

	FrameBuffer* frameBuffer = nullptr;
	string frameBufferName = "";

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