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
	// update status of all buttons
	auto& io = ImGui::GetIO();
	if (!io.WantCaptureKeyboard)
	{
		for (auto& b : s_instance->keyButtons)
			b.second.Update(s_instance->m_window, b.first);
	}

	if (!io.WantCaptureMouse)
	{
		for (auto& b : s_instance->mouseButtons)
			b.second.Update(s_instance->m_window, b.first);
	}

	s_instance->m_lastMousePosition = s_instance->m_mousePosition;
	double mouseX, mouseY;
	glfwGetCursorPos(s_instance->m_window, &mouseX, &mouseY);
	s_instance->m_mousePosition = { mouseX, mouseY };

	if (Input::Keyboard(GLFW_KEY_F10).Down())
		Window::Get()->ToggleFullscreen();

	if (Input::Keyboard(GLFW_KEY_F9).Down())
		Window::Get()->ToggleMouseCursor();
}

Input* Input::s_instance = nullptr;

void Input::KeyButton::Update(GLFWwindow* window, int key)
{
	last = down;
	down = glfwGetKey(window, key);
}

void Input::MouseButton::Update(GLFWwindow* window, int number)
{
	last = down;
	down = glfwGetMouseButton(window, number);
}
