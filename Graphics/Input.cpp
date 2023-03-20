#include "Input.h"

Input::Input(GLFWwindow* window)
{
	m_window = window;
	s_instance = this;

	m_mousePosition = { 0,0 };
	m_lastMousePosition = { 0,0 };

}

void Input::Update()
{
	m_lastMousePosition = m_mousePosition;
	double mouseX, mouseY;
	glfwGetCursorPos(m_window, &mouseX, &mouseY);
	m_mousePosition = { mouseX, mouseY };
}

Input* Input::s_instance = nullptr;