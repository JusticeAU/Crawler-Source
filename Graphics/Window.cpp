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
	m_viewPortSize = { width, height };
	glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.x);
	s_instance = this;
}

void Window::SetWindowTitle(std::string title)
{
	glfwSetWindowTitle(s_instance->m_window, title.c_str());
}

void Window::ToggleFullscreen()
{
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vid = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (fullScreen)
	{
		glfwSetWindowMonitor(m_window, NULL, m_windowPos.x, m_windowPos.y, m_windowSize.x, m_windowSize.y, vid->refreshRate);
		glfwSwapInterval(1);
	}
	else
	{
		// Save previous window size for when we go back.
		glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
		glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);

		// Set fullscreen
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, vid->width, vid->height, vid->refreshRate);
		glfwSwapInterval(1);
	}

	fullScreen = !fullScreen;
}

void Window::ToggleMouseCursor()
{
	showMouseCursor = !showMouseCursor;

	if (showMouseCursor)
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::SetMouseCursorHidden(bool hidden)
{
	showMouseCursor = !hidden;

	if (showMouseCursor)
	{
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE); 
	}
	else
	{
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // Enable raw mouse data rather than window pixel based.
	}
}

Window* Window::s_instance = nullptr;