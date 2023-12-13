#pragma once
#include "Component.h"

class Texture;

class ComponentBillboard : public Component
{
public:
	ComponentBillboard(Object* parent);
	~ComponentBillboard();

	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void DrawGUI() override;

	Texture* texture = nullptr;

	Component* Clone(Object* parent);
};