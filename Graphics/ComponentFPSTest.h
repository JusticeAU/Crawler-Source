#pragma once
#include "Component.h"
#include "Window.h"

class ComponentAnimator;

class ComponentFPSTest : public Component
{
public:
	ComponentFPSTest(Object* parent) : Component("FPSTest", Component_FPSTest, parent) {};

	void Update(float delta) override;

	void OnParentChange() override;
private:
	int clipCapacity = 7;
	int clipRounds = 7;

	GLFWwindow* window = nullptr;

	ComponentAnimator* animator = nullptr;

	string nextAnimation = "";
	bool nextLooping = true;
	float nextAnimTime = 0.0f;

	bool fireDown = false;
	bool reloadDown = false;
	bool moveDown = false;

	Object* mainObject = nullptr;

	float rotationX = 0.0f;
	float rotationY = 0.0f;
};

