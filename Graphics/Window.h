#pragma once
#include "Graphics.h"
#include <string>

// Window is a container for the glfw window. It allows allows other classes within the application to access things like window size and the ability to toggle fullscreen or mouse hiding.
class Window
{
public:
	Window(int width, int height, const char* title, GLFWmonitor* = nullptr);
	static Window* GetWindow() { return s_instance; };
	static const glm::ivec2 GetViewPortSize() { return s_instance->m_viewPortSize; };
	static void SetViewPortSize(glm::ivec2 size) { s_instance->m_viewPortSize = size; };
	static void SetWindowTitle(std::string title);

	static Window* Get() { return s_instance; };
	
	void ToggleFullscreen();
	bool GetFullscreen() { return fullScreen; };
	void ToggleMouseCursor();
	void SetMouseCursorHidden(bool hidden);
	GLFWwindow* GetGLFWwindow() { return m_window; }
	
protected:
	glm::ivec2 m_windowSize;
	glm::ivec2 m_viewPortSize;
	GLFWwindow* m_window;
	
	bool fullScreen = false;
	bool showMouseCursor = true;

	glm::ivec2 m_windowPos;
	static Window* s_instance;
};