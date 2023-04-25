#pragma once
#include "Graphics.h"
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
		int GLFW_KEY_ = 0;
		bool Down()		{ return down && !last; };
		bool Pressed()	{ return down; };
		bool Up()		{ return !down && last; };

		virtual void Update(GLFWwindow* window);
	};

	class MouseButton : public KeyButton
	{
	public:
		void Update(GLFWwindow* window) override;
	};

public:
	static void Init(GLFWwindow* window);
	static void Update();
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
	static KeyButton& Keyboard(int GLFW_KEY);
	static MouseButton& Mouse(int number);

protected:
	Input(GLFWwindow* window);
	static Input* s_instance;
	GLFWwindow* m_window;

	vec2 m_mousePosition;
	vec2 m_lastMousePosition;

	std::map<int, KeyButton> keyButtons;
	std::map<int, MouseButton> mouseButtons;


};