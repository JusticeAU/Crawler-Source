#pragma once
#include "Graphics.h"

using glm::vec2;

class Input
{
public:
	Input(GLFWwindow* window);
	GLFWwindow* m_window;
	void Update();
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
protected:
	static Input* s_instance;

	vec2 m_mousePosition;
	vec2 m_lastMousePosition;
};