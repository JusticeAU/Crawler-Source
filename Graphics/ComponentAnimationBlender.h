#pragma once
#include "Component.h"
#include "Graphics.h"
#include <string>

//const int MAX_BONES = 100;

class Model;
class UniformBuffer;

using std::string;
using glm::mat4;

class ComponentAnimationBlender : public Component
{
public:
	ComponentAnimationBlender(Object* parent) : Component("AnimationBlender", Component_AnimationBlender, parent) {};
	ComponentAnimationBlender(Object* parent, std::istream& istream);

	~ComponentAnimationBlender();
	ComponentAnimationBlender(ComponentAnimationBlender& other) = delete;
	const ComponentAnimationBlender& operator=(const ComponentAnimationBlender& other) = delete;


	void Update(float deltatime) override;
	void DrawGUI() override;

	void OnParentChange() override;

	void UpdateBoneMatrixBuffer();
	void ProcessNode(float frameTimeA, int animationIndexA, float frameTimeB, int animationIndexB, float weightAB, Object* node, mat4 accumulated);

	// dependencies
	Model* model = nullptr;

	// Animation state
	mat4* boneTransforms = nullptr;
	UniformBuffer* boneTransfomBuffer = nullptr;

	// Blender configuration
	int animationA = 0;
	string animationAName = "";
	float animationATime = 0.0f;
	int animationB = 0;
	string animationBName = "";
	float animationBTime = 0.0f;
	float weightAB = 0.5f;
};