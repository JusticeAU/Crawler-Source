#include "Input.h"
#include "Window.h"

#include "LogUtils.h"

Input::Input(GLFWwindow* window)
{
	m_window = window;
	s_instance = this;

	m_mousePosition = { 0,0 };
	m_lastMousePosition = { 0,0 };
}

void Input::Init(GLFWwindow* window)
{
	if (!s_instance)
	{
		s_instance = new Input(window);
		glfwSetJoystickCallback(GLFWJoystickConnectedCallback);
	
		if(glfwJoystickIsGamepad(0))
			SetGamepadStatus(0, true);
	}
}

void Input::Update()
{
	// update status of all buttons - We check io.WantCaptureX first to see if Imgui is taking precedence over mouse or keyb input.
	auto& io = ImGui::GetIO();
	if (!io.WantCaptureKeyboard)
	{
		for (auto& b : s_instance->keyButtons)
			b.second.Update(s_instance->m_window, b.first);
	}

	if (!io.WantCaptureMouse)
	{
		for (auto& b : s_instance->mouseButtons)
			b.second.Update(s_instance->m_window, b.first);
	}

	// Gamepad
	if (s_instance->isGamepadConnected)
	{
		s_instance->gamepad.statePrevious = s_instance->gamepad.stateCurrent;
		glfwGetGamepadState(GLFW_JOYSTICK_1, &s_instance->gamepad.stateCurrent);
	}

	
	s_instance->m_lastMousePosition = s_instance->m_mousePosition;
	double mouseX, mouseY;
	glfwGetCursorPos(s_instance->m_window, &mouseX, &mouseY);
	s_instance->m_mousePosition = { mouseX, mouseY };

	if (Input::Keyboard(GLFW_KEY_F10).Down())
		Window::Get()->ToggleFullscreen();

	if (Input::Keyboard(GLFW_KEY_F9).Down())
		Window::Get()->ToggleMouseCursor();
}

vec2 Input::GetMousePosPixel()
{
	double mouseX, mouseY;
	glfwGetCursorPos(s_instance->m_window, &mouseX, &mouseY);
	return { mouseX, mouseY };
}

vec2 Input::GetMousePosNDC()
{
	vec2 mousePosPixel = GetMousePosPixel();
	vec2 windowSizePixels = Window::GetViewPortSize();
	
	float ndcX = ((2 * mousePosPixel.x) / windowSizePixels.x) - 1;
	float ndcY = ((2 * mousePosPixel.y) / windowSizePixels.y) - 1;

	return { ndcX, -ndcY };
}

Input* Input::s_instance = nullptr;

void Input::KeyButton::Update(GLFWwindow* window, int key)
{
	last = down;
	down = glfwGetKey(window, key);
}

void Input::MouseButton::Update(GLFWwindow* window, int number)
{
	last = down;
	down = glfwGetMouseButton(window, number);
}


void Input::SetGamepadStatus(int joystickID, bool connected)
{
	if (joystickID == 0)
	{
		if (connected)
		{
			LogUtils::Log("Gamepad Connected");
			LogUtils::Log(glfwGetGamepadName(joystickID));
		}
		else LogUtils::Log("Gamepad Disconnected");
		s_instance->isGamepadConnected = connected;
	}
}

void GLFWJoystickConnectedCallback(int joystickID, int eventID)
{
	if (eventID == GLFW_CONNECTED)
	{
		Input::SetGamepadStatus(joystickID, true);
	}
	else if (eventID == GLFW_DISCONNECTED)
	{
		Input::SetGamepadStatus(joystickID, false);
	}
}

bool Input::InputAlias::Down()
{
	for (auto& key : keys) if (Input::Keyboard(key).Down()) return true;
	for (auto& click : clicks)  if (Input::Mouse(click).Down()) return true;

	if (!Input::IsGamepadConnected()) return false;
	for (auto& button : gamepadButtons) if (Input::Gamepad().Down(button)) return true;
	for (auto& axis : gamepadNegativeAxes) if (Input::Gamepad().AxesDown(axis, true)) return true;
	for (auto& axis : gamepadPostiveAxes)  if (Input::Gamepad().AxesDown(axis, false)) return true;
	return false;
}

bool Input::InputAlias::Pressed()
{
	for (auto& key : keys) if (Input::Keyboard(key).Pressed()) return true;
	for (auto& click : clicks)  if (Input::Mouse(click).Pressed()) return true;

	if (!Input::IsGamepadConnected()) return false;
	for (auto& button : gamepadButtons) if (Input::Gamepad().Pressed(button)) return true;
	for (auto& axis : gamepadNegativeAxes) if (Input::Gamepad().AxesPressed(axis, true)) return true;
	for (auto& axis : gamepadPostiveAxes)  if (Input::Gamepad().AxesPressed(axis, false)) return true;
	
	return false;
}

bool Input::InputAlias::Up()
{
	for (auto& key : keys) if (Input::Keyboard(key).Up()) return true;
	for (auto& click : clicks)  if (Input::Mouse(click).Up()) return true;

	if (!Input::IsGamepadConnected()) return false;
	for (auto& button : gamepadButtons) if (Input::Gamepad().Up(button)) return true;
	for (auto& axis : gamepadNegativeAxes) if (Input::Gamepad().AxesUp(axis, true)) return true;
	for (auto& axis : gamepadPostiveAxes)  if (Input::Gamepad().AxesUp(axis, false)) return true;
	
	return false;
}

void Input::InputAlias::RegisterKeyButton(int GLFW_KEY)
{
	// Check its not already there, otherwise add it.
	for(auto& key : keys) if (key == GLFW_KEY) return;
	keys.emplace_back(GLFW_KEY);
}

void Input::InputAlias::RegisterMouseButton(int GLFW_MOUSE_BUTTON)
{
	// Check its not already there, otherwise add it.
	for (auto& click : clicks) if (click == GLFW_MOUSE_BUTTON) return;
	clicks.emplace_back(GLFW_MOUSE_BUTTON);
}

void Input::InputAlias::RegisterGamepadButton(int GLFW_GAMEPAD_BUTTON)
{
	// Check its not already there, otherwise add it.
	for (auto& button : gamepadButtons) if (button == GLFW_GAMEPAD_BUTTON) return;
	gamepadButtons.emplace_back(GLFW_GAMEPAD_BUTTON);
}

void Input::InputAlias::RegisterGamepadAxes(int GLFW_GAMEPAD_AXES, bool downIsNegativeValue)
{
	// Check its not already there, otherwise add it.
	if (downIsNegativeValue)
	{
		for (auto& axis : gamepadNegativeAxes) if (axis == GLFW_GAMEPAD_AXES) return;
		gamepadNegativeAxes.emplace_back(GLFW_GAMEPAD_AXES);
	}
	else
	{
		for (auto& axis : gamepadPostiveAxes) if (axis == GLFW_GAMEPAD_AXES) return;
		gamepadPostiveAxes.emplace_back(GLFW_GAMEPAD_AXES);
	}
}
