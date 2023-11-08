#pragma once
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "glm.hpp"

#include <string>
#include <vector>
#include <map>

using glm::vec2;

// basic wrapper for GLFW input
// Will later be used as to handle communication through GLFW -> Imgui -> Application.

class Input
{
public:
	enum class InputType
	{
		Keyboard,
		Mouse,
		Gamepad
	};
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
	class GamepadState
	{
	public:
		const float axesTriggerThreshold = 0.75f;
		
		bool Down(int GLFW_GAMEPAD) { return stateCurrent.buttons[GLFW_GAMEPAD] && !statePrevious.buttons[GLFW_GAMEPAD]; };
		bool Pressed(int GLFW_GAMEPAD) { return stateCurrent.buttons[GLFW_GAMEPAD]; };
		bool Up(int GLFW_GAMEPAD) { return !stateCurrent.buttons[GLFW_GAMEPAD] && statePrevious.buttons[GLFW_GAMEPAD]; };
		float Axes(int GLFW_GAMEPAD) { return stateCurrent.axes[GLFW_GAMEPAD]; };
		bool AxesDown(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] > axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] < axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] < -axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] > -axesTriggerThreshold;
		};
		bool AxesPressed(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] > axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] < -axesTriggerThreshold;
		};
		bool AxesUp(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] < axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] > axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] > -axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] < -axesTriggerThreshold;
		};

		GLFWgamepadstate stateCurrent;
		GLFWgamepadstate statePrevious;
	};
	class InputAlias
	{
	public:
		bool Down();
		bool Pressed();
		bool Up();

		void RegisterKeyButton(int GLFW_KEY);
		void RegisterMouseButton(int GLFW_MOUSE_BUTTON);
		void RegisterGamepadButton(int GLFW_GAMEPAD_BUTTON);
		void RegisterGamepadAxis(int GLFW_GAMEPAD_AXES, bool downIsNegativeValue = false);

		std::vector<int> keys;
		std::vector<int> clicks;
		std::vector<int> gamepadButtons;
		std::vector<int> gamepadPostiveAxes;
		std::vector<int> gamepadNegativeAxes;
	};
public:
	static void Init(GLFWwindow* window);
	static void Update();
	static vec2 GetMousePosPixel();
	static vec2 GetMousePosNDC();
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
	static KeyButton& Keyboard(int GLFW_KEY) { return s_instance->keyButtons[GLFW_KEY]; };
	static MouseButton& Mouse(int number) { return s_instance->mouseButtons[number]; };
	static GamepadState& Gamepad() { return s_instance->gamepad; };
	static InputAlias& Alias(std::string alias) { return s_instance->aliases[alias]; };

	static void SetGamepadStatus(int joystickID, bool connected);
	static bool IsGamepadConnected() { return s_instance->isGamepadConnected; }

	static InputType GetLastInputType() { return s_instance->m_lastInputType; };

	//static bool LastInputWasFromGamepad() { return s_instance->lastInputWasGamepad; };
	//static bool LastInputWasFromKeyboard() { return s_instance->lastInputWasKeyboard; };
	//static bool LastInputWasFromMoused() { return s_instance->lastInputWasMouse; };


protected:
	Input(GLFWwindow* window);
	static Input* s_instance;
	GLFWwindow* m_window;

	InputType m_lastInputType = InputType::Mouse;

	bool lastInputWasKeyboard = false;

	vec2 m_mousePosition;
	vec2 m_lastMousePosition;
	bool lastInputWasMouse = true;

	std::map<int, KeyButton> keyButtons;
	std::map<int, MouseButton> mouseButtons;

	// Gamepad
	bool isGamepadConnected = false;
	bool lastInputWasGamepad = false;
	GamepadState gamepad;

	// Aliasing
	std::map<std::string, InputAlias> aliases;

	bool IsAnyKeyboardInput();
	bool IsAnyMouseInput();
	bool IsAnyGamepadInput();
};

void GLFWJoystickConnectedCallback(int joystickID, int eventID);