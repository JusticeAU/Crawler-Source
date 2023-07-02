#pragma once
#include "ComponentRenderer.h"
#include "Object.h"	

class Camera;
class ComponentAnimator;
class ComponentAnimationBlender;

using glm::mat4;
using glm::vec3;

class ComponentSkinnedRenderer : public ComponentRenderer
{
public:
	ComponentSkinnedRenderer(Object* parent);
	ComponentSkinnedRenderer(Object* parent, std::istream& istream);
	ComponentSkinnedRenderer(Object* parent, ordered_json j);
	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void OnParentChange() override;

	void BindBoneTransform();

	Component* Clone(Object* parent);
protected:
	ComponentAnimator* animator = nullptr;
	ComponentAnimationBlender* animationBlender = nullptr;
};
