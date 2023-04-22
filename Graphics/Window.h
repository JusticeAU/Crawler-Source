#pragma once
#include "Graphics.h"

class Window
{
public:
	Window(int width, int height, const char* title, GLFWmonitor* = nullptr);
	static Window* GetWindow() { return s_instance; };
	static const glm::ivec2 GetViewPortSize() { return s_instance->m_viewPortSize; };
	void ToggleFullscreen();
	bool GetFullscreen() { return fullScreen; };
	void ToggleMouseCursor();
	void SetMouseCursorHidden(bool hidden);
	GLFWwindow* GetGLFWwindow() { return m_window; }
	static Window* Get() { return s_instance; };
	glm::ivec2 m_windowSize;
	glm::ivec2 m_viewPortSize;
protected:
	GLFWwindow* m_window;
	
	bool fullScreen = false;
	bool showMouseCursor = true;

	glm::ivec2 m_windowPos;
	static Window* s_instance;
};