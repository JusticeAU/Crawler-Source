#pragma once
#include "Component.h"
#include "glm.hpp"

class ComponentLightPoint : public Component
{
public:
	ComponentLightPoint(Object* parent);
	ComponentLightPoint(Object* parent, nlohmann::ordered_json j);
	~ComponentLightPoint();

	glm::vec3 colour { 1.0f, 1.0f, 1.0f };
	float intensity = 10.0f;

	void DrawGUI() override;
};