#pragma once
#include "Component.h"
#include "Graphics.h"
#include <string>

class Model;
class UniformBuffer;

using std::string;
using glm::mat4;

class ComponentAnimator : public Component
{
public:
	ComponentAnimator(Object* parent) : Component("Animator", Component_Animator, parent) {};
	ComponentAnimator(Object* parent, std::istream& istream);
	
	void Update(float deltatime) override;
	void DrawGUI() override;

	void Write(std::ostream& ostream) override;

	void OnParentChange() override;

	void UpdateBoneMatrixBuffer(float frameTime);
	void ProcessNode(float frameTime, int animationIndex, Object* node, mat4 accumulated);

	// dependencies
	Model* model = nullptr;

	// Animation state
	int selectedBone = 0;
	int selectedAnimation = 0;
	string animationName = "";
	bool loopAnimation = true;
	bool playAnimation = true;
	float animationSpeed = 1.0f;
	float animationTime = 0.0f;

	mat4* boneTransforms = nullptr;
	UniformBuffer* boneTransfomBuffer = nullptr;
};