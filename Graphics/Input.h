#pragma once
#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glad.h"
#include "glm.hpp"

#include <string>
#include <vector>
#include <map>

using glm::vec2;

// Basic wrapper for GLFW input
class Input
{
public:
	enum class InputType
	{
		Keyboard,
		Mouse,
		Gamepad
	};
	// Base class for handling keyboard button presses.
	class KeyButton
	{
	protected:
		bool down = false;
		bool last = false;

	public:
		// The button went down this frame and was up last frame.
		bool Down()		{ return down && !last; };	
		// The button is down.
		bool Pressed()	{ return down; };		
		// The button went up this frame and was down last frame.
		bool Up()		{ return !down && last; };

		virtual void Update(GLFWwindow* window, int index);
	};
	// Child class of KeyButton, reimplmented to check mousebuttons instead. Behaves the same.
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
		// Returns the current value of an axis. -1 to 1 range.
		float Axes(int GLFW_GAMEPAD) { return stateCurrent.axes[GLFW_GAMEPAD]; };
		// An Axis is considered down if it breached the 'axesTriggerThreshold' const value this frame. Some Axes will move towards 1, others will move towards -1, so a boolean is available here.
		bool AxesDown(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] > axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] < axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] < -axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] > -axesTriggerThreshold;
		};
		// An Axis is considered pressed if it has breached the 'axesTriggerThreshold' const value. Some Axes will move towards 1, others will move towards -1, so a boolean is available here.
		bool AxesPressed(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] > axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] < -axesTriggerThreshold;
		};
		// An Axis is considered up if previous it was pressed and has returned from the 'axesTriggerThreshold' const value this frame. Some Axes will move towards 1, others will move towards -1, so a boolean is available here.
		bool AxesUp(int GLFW_GAMEPAD, bool downIsNegativeValue = false)
		{
			if (!downIsNegativeValue) return stateCurrent.axes[GLFW_GAMEPAD] < axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] > axesTriggerThreshold;
			else return stateCurrent.axes[GLFW_GAMEPAD] > -axesTriggerThreshold && statePrevious.axes[GLFW_GAMEPAD] < -axesTriggerThreshold;
		};

		// It's much simpler to hold the entire curretn and previous state of the gamepad.
		GLFWgamepadstate stateCurrent;
		GLFWgamepadstate statePrevious;
	};
	// An InputAlias is a key that can have a bunch of different inputs mapped to it. Allows you to check if "Interact" is pressed and have that be true for both a keyboard or gamepad input.
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
	// Creates the singleton, configures the gamepad callback and checks if one is plugged in.
	static void Init(GLFWwindow* window);
	// Call this once per frame in the engine update.
	static void Update();
	// Returns the mouse position in pixels with the origin being the top left of the window frame buffer. Can report negative numbers and numbers greater than the buffer if the cursor is outside.
	static vec2 GetMousePosPixel();
	// Returns the mouse position in Normalised Device Coordinates. The range will exceed NDC if the cursor is outside the window framebuffer.
	static vec2 GetMousePosNDC();
	// Returns the difference in pixels between the last position and the current position of the mouse cursor.
	static vec2 GetMouseDelta() { return s_instance->m_mousePosition - s_instance->m_lastMousePosition; };
	static bool IsAnyMousePointerInput();
	static bool IsAnyMouseButtonInput();
	// Poll the status of a particular Keyboard button.
	static KeyButton& Keyboard(int GLFW_KEY) { return s_instance->keyButtons[GLFW_KEY]; };
	// Poll the status of a particular Mouse button.
	static MouseButton& Mouse(int number) { return s_instance->mouseButtons[number]; };
	// Poll the status of the current active gamepad.
	static GamepadState& Gamepad() { return s_instance->gamepad; };
	// Poll the status of a particular Alias.
	static InputAlias& Alias(std::string alias) { return s_instance->aliases[alias]; };

	// used internally by the joystick connected callback. If a joystick connects, and it's the first joystick, then it will be set as active.
	static void SetGamepadStatus(int joystickID, bool connected);
	static bool IsGamepadConnected() { return s_instance->isGamepadConnected; }

	// Returns the input device that was last used.
	static InputType GetLastInputType() { return s_instance->m_lastInputType; };

protected:
	Input(GLFWwindow* window);
	static Input* s_instance;
	GLFWwindow* m_window;

	InputType m_lastInputType = InputType::Mouse;
	bool m_isMousePointerInput = false;
	bool m_isMouseButtonInput = false;


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