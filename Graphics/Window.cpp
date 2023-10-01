#include "Window.h"
#include "LogUtils.h"
#include "glfwCallbacks.h"

Window::Window(const char* title)
{
	// Set resolution and window name.
	LogUtils::Log("Creating GLFW Window.");
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vid = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, vid->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, vid->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, vid->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, vid->refreshRate);
	m_window = glfwCreateWindow(vid->width, vid->height, title, monitor, nullptr);
	glfwSetWindowSizeCallback(m_window, WindowResizeCallback);
	m_fullScreenSize = { vid->width, vid->width };
	m_viewPortSize = { vid->width, vid->height };
	glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.x);
	s_instance = this;
	fullScreen = true;
}

Window::Window(int width, int height, const char* title)
{
	// Set resolution and window name.
	LogUtils::Log("Creating GLFW Window.");
	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwSetWindowSizeCallback(m_window, WindowResizeCallback);
	m_windowSize = { width, height };
	m_viewPortSize = { width, height };
	s_instance = this;
	RecentreWindow();
}

void Window::SetWindowTitle(std::string title)
{
	glfwSetWindowTitle(s_instance->m_window, title.c_str());
}

void Window::ToggleFullscreen()
{
	fullScreen = !fullScreen;

	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vid = glfwGetVideoMode(mon);

	if (!fullScreen)
	{
		// Position the window in the centre of the screen.
		// Recalculate the offset.

		glfwSetWindowMonitor(m_window, NULL, m_windowPos.x, m_windowPos.y, m_windowSize.x, m_windowSize.y, vid->refreshRate);
		if(!m_windowHasPosition) RecentreWindow();
	}
	else
	{
		// Save previous window size for when we go back.
		glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
		m_windowHasPosition = true;
		glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);

		// Set fullscreen
		glfwWindowHint(GLFW_RED_BITS, vid->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, vid->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, vid->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, vid->refreshRate);
		m_fullScreenSize = { vid->width, vid->height };
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, vid->width, vid->height, vid->refreshRate);
	}

}

void Window::RecentreWindow()
{
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vid = glfwGetVideoMode(mon);

	m_windowPos.x = (vid->width * 0.5) - (m_windowSize.x * 0.5f);
	m_windowPos.y = (vid->height * 0.5) - (m_windowSize.y * 0.5f);
	m_windowHasPosition = true;

	glfwSetWindowPos(m_window, m_windowPos.x, m_windowPos.y);
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