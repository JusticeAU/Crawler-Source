#pragma once
#include "ComponentRenderer.h"
#include "Object.h"	

class ComponentAnimator;

class ComponentSkinnedRenderer : public ComponentRenderer
{
public:
	ComponentSkinnedRenderer(Object* parent);
	ComponentSkinnedRenderer(Object* parent, std::istream& istream);
	void Draw() override;

	void OnParentChange() override;

	void BindBoneTransform();
protected:
	ComponentAnimator* animator = nullptr;
};
