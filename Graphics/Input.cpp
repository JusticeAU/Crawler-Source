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
		{
			b.second.Update(s_instance->m_window);
		}
	}

	if (!io.WantCaptureMouse)
	{
		for (auto& b : s_instance->mouseButtons)
		{
			b.second.Update(s_instance->m_window);
		}
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

Input::KeyButton& Input::Keyboard(int GLFW_KEY)
{
	if (s_instance->keyButtons.find(GLFW_KEY) == s_instance->keyButtons.end())
	{
		Input::KeyButton button;
		button.GLFW_KEY_ = GLFW_KEY;
		s_instance->keyButtons.emplace(GLFW_KEY, button);
	}

	return s_instance->keyButtons[GLFW_KEY];
}

Input::MouseButton& Input::Mouse(int number)
{
	if (s_instance->mouseButtons.find(number) == s_instance->mouseButtons.end())
	{
		Input::MouseButton button;
		button.GLFW_KEY_ = number;
		s_instance->mouseButtons.emplace(number, button);
	}

	return s_instance->mouseButtons[number];
}

Input* Input::s_instance = nullptr;

void Input::KeyButton::Update(GLFWwindow* window)
{
	last = down;
	down = glfwGetKey(window, GLFW_KEY_);
}

void Input::MouseButton::Update(GLFWwindow* window)
{
	last = down;
	down = glfwGetMouseButton(window, GLFW_KEY_);
}
