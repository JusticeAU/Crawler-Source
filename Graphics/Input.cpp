#include "Input.h"
#include "Window.h"

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

	if (glfwGetKey(s_instance->m_window, GLFW_KEY_F10) == GLFW_PRESS && s_instance->fullScreenReleased)
	{
		s_instance->fullScreenReleased = false;
		Window::Get()->ToggleFullscreen();
	}

	if (glfwGetKey(s_instance->m_window, GLFW_KEY_F10) == GLFW_RELEASE)
		s_instance->fullScreenReleased = true;

	if (glfwGetKey(s_instance->m_window, GLFW_KEY_F9) == GLFW_PRESS && s_instance->hideCursorReleased)
	{
		s_instance->hideCursorReleased = false;
		Window::Get()->ToggleMouseCursor();
	}

	if (glfwGetKey(s_instance->m_window, GLFW_KEY_F9) == GLFW_RELEASE)
		s_instance->hideCursorReleased = true;
}

Input* Input::s_instance = nullptr;