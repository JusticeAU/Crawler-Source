#pragma once
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "glm.hpp"
#include <map>

using glm::vec2;

// basic wrapper for GLFW input
// Will later be used as to handle communication through GLFW -> Imgui -> Application.

class Input
{
public:
	class KeyButton
	{
	protected:
		bool down = false;
		bool last = false;

	public:
		bool Down()		{ return down && !last; };	// The button went down this frame and was up last frame.
		bool Pressed()	{ return down; };			// The button is down.
		bool Up()		{ return !down && last; };	// The button went up this frame and was down last frame.

		virtual void Update(GLFWwindow* window, int index);
	};
	class MouseButton : public KeyButton
	{
	public:
		virtual void Update(GLFWwindow* window, int index) override;
	};

public:
	static void Init(GLFWwindow* window);
	static void Update();
	static vec2 GetMousePosPixel();
	static vec2 GetMousePosNDC();
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
	static KeyButton& Keyboard(int GLFW_KEY) { return s_instance->keyButtons[GLFW_KEY]; };
	static MouseButton& Mouse(int number) { return s_instance->mouseButtons[number]; };

protected:
	Input(GLFWwindow* window);
	static Input* s_instance;
	GLFWwindow* m_window;

	vec2 m_mousePosition;
	vec2 m_lastMousePosition;

	std::map<int, KeyButton> keyButtons;
	std::map<int, MouseButton> mouseButtons;


};