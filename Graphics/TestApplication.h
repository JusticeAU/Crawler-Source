#pragma once
#include "Application.h"
#include "glm.hpp"

class TestApplication : public Application
{
public:
	TestApplication();
	void Update(float delta) override;
protected:
	glm::vec4 fromColour;
	glm::vec4 toColour;
	bool to = true;
	float t = 0.0f;
};