#pragma once
#include "Component.h"
#include "Graphics.h"
#include <string>

const int MAX_BONES = 200;

class Model;
class Animation;
class UniformBuffer;

using std::string;
using glm::mat4;

class ComponentAnimator : public Component
{
public:
	ComponentAnimator(Object* parent);
	ComponentAnimator(Object* parent, ordered_json j);
	
	~ComponentAnimator();
	ComponentAnimator(ComponentAnimator& other) = delete;
	const ComponentAnimator& operator=(const ComponentAnimator& other) = delete;

	
	void Update(float deltatime) override;
	void DrawGUI() override;

	void OnParentChange() override;

	Component* Clone(Object* parent);

	void UpdateBoneMatrixBuffer();
	void ProcessNode(Object* node, mat4 accumulated);

	void StartAnimation(string name, bool loop = false);
	void BlendToAnimation(string name, float transitionTime, float offset = 0.0f, bool loop = false);

	struct AnimationState
	{
		Animation* animation = nullptr;
		float animationSpeedScale = 1.0f;
		bool looping = true;
		float position = 0.0f;

		void Update(float delta);
	};

	bool isPlaying = false;

	// dependencies
	Model* model = nullptr;

	mat4* boneTransforms = nullptr;
	UniformBuffer* boneTransfomBuffer = nullptr;

	// New stuff, transition
	AnimationState* current = nullptr;
	AnimationState* next = nullptr;
	float transitionTime = 0.3f;
	float transitionProgress = 0.0f;
	float transitionWeight = 0.0f;
	bool transitionsShouldLoop = false;
	float transitionBlendTime = 0.1f;
};