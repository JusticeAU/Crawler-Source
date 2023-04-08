#pragma once
#include "Graphics.h"

class Window
{
public:
	Window(int width, int height, const char* title, GLFWmonitor* = nullptr);
	static Window* GetWindow() { return s_instance; };
	void ToggleFullscreen();
	bool GetFullscreen() { return fullScreen; };
	GLFWwindow* GetGLFWwindow() { return m_window; }
	static Window* Get() { return s_instance; };
protected:
	GLFWwindow* m_window;
	
	bool fullScreen = false;

	glm::ivec2 m_windowPos;
	glm::ivec2 m_windowSize;
	static Window* s_instance;
};