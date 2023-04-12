#pragma once
#include "ComponentRenderer.h"
#include "Object.h"	

class Camera;
class ComponentAnimator;

using glm::mat4;
using glm::vec3;

class ComponentSkinnedRenderer : public ComponentRenderer
{
public:
	ComponentSkinnedRenderer(Object* parent);
	ComponentSkinnedRenderer(Object* parent, std::istream& istream);
	void Draw(mat4 pv, vec3 position) override;

	void OnParentChange() override;

	void BindBoneTransform();
protected:
	ComponentAnimator* animator = nullptr;
};
