#pragma once
#include "Graphics.h"

using glm::vec2;

class Input
{
public:
	static void Init(GLFWwindow* window);
	static void Update();
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
protected:
	Input(GLFWwindow* window);
	static Input* s_instance;
	GLFWwindow* m_window;

	vec2 m_mousePosition;
	vec2 m_lastMousePosition;
};