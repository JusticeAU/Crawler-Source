#include "Input.h"

Input::Input(GLFWwindow* window)
{
	m_window = window;
	s_instance = this;

	m_mousePosition = { 0,0 };
	m_lastMousePosition = { 0,0 };

}

void Input::Init(GLFWwindow* window)
{
	if (!s_instance) s_instance = new Input(window);
}

void Input::Update()
{

	s_instance->m_lastMousePosition = s_instance->m_mousePosition;
	double mouseX, mouseY;
	glfwGetCursorPos(s_instance->m_window, &mouseX, &mouseY);
	s_instance->m_mousePosition = { mouseX, mouseY };
}

Input* Input::s_instance = nullptr;