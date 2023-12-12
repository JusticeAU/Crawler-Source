#pragma once
#include "Graphics.h"
#include <string>

// Window is a container for the glfw window. It allows allows other classes within the application to access things like window size and the ability to toggle fullscreen or mouse hiding.
class Window
{
public:
	// Creates a fullscreen borderless window on the primary monitor.
	Window(const char* title); 
	// Creates a Window with decoration of a particular size.
	Window(int width, int height, const char* title);
	// Returns the instance of this container class window.
	static Window* Get() { return s_instance; };

	static const glm::ivec2 GetViewPortSize() { return s_instance->m_viewPortSize; };
	static void SetViewPortSize(glm::ivec2 size) { s_instance->m_viewPortSize = size; };
	static void SetWindowTitle(std::string title);
	static double GetTime() { return glfwGetTime(); }
	
	void ToggleFullscreen();
	bool IsFullscreen() { return fullScreen; };

	// Positions the window in the centre of the primary monitor.
	void RecentreWindow();

	// Toggles whether the mouse cursor is hidden or not
	void ToggleMouseCursor();
	void SetMouseCursorHidden(bool hidden);

	// returns the wrapped GLFW window
	GLFWwindow* GetGLFWwindow() { return m_window; }
protected:
	glm::ivec2 m_fullScreenSize;
	glm::ivec2 m_windowSize = { 1600, 900 };
	glm::ivec2 m_viewPortSize;
	GLFWwindow* m_window;
	
	bool fullScreen = false;
	bool showMouseCursor = true;

	glm::ivec2 m_windowPos;
	bool m_windowHasPosition = false;
	static Window* s_instance;
};