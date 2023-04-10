#pragma once
#include "Graphics.h"

class Window
{
public:
	Window(int width, int height, const char* title, GLFWmonitor* = nullptr);
	static Window* GetWindow() { return s_instance; };
	static const glm::ivec2 GetWindowSize();
	void ToggleFullscreen();
	bool GetFullscreen() { return fullScreen; };
	GLFWwindow* GetGLFWwindow() { return m_window; }
	static Window* Get() { return s_instance; };
	glm::ivec2 m_windowSize;
protected:
	GLFWwindow* m_window;
	
	bool fullScreen = false;

	glm::ivec2 m_windowPos;
	static Window* s_instance;
};