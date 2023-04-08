#include "Window.h"
#include "LogUtils.h"
#include "glfwCallbacks.h"

Window::Window(int width, int height, const char* title, GLFWmonitor* monitor)
{
	// Set resolution and window name.
	LogUtils::Log("Creating GLFW Window.");
	m_window = glfwCreateWindow(width, height, title, monitor, nullptr);
	glfwSetWindowSizeCallback(m_window, WindowResizeCallback);
	m_windowSize = { width, height };
	glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.x);
	s_instance = this;
}

void Window::ToggleFullscreen()
{
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vid = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (fullScreen)
		glfwSetWindowMonitor(m_window, NULL, m_windowPos.x, m_windowPos.y, m_windowSize.x, m_windowSize.y, vid->refreshRate);
	else
	{
		glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
		glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, vid->width, vid->height, vid->refreshRate);
	}

	fullScreen = !fullScreen;
}

Window* Window::s_instance = nullptr;