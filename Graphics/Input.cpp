#include "Input.h"
#include "Window.h"

#include "LogUtils.h"

#include "TourBox.h"

void Input::DrawGUI()
{
	s_instance->DrawTourBoxConfig();
}

Input::Input(GLFWwindow* window)
{
	m_window = window;
	s_instance = this;

	m_mousePosition = { 0,0 };
	m_lastMousePosition = { 0,0 };
}

void Input::DrawTourBoxConfig()
{
	ImGui::SetNextWindowPos({ 400,400 }, ImGuiCond_Once);
	ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_Once);
	ImGui::Begin("TourBox Configuration");
	ImGui::InputInt("COM Port", &tourBoxComPort, 1, 1);
	if (ImGui::Button(tourBoxConnected ? "Reconnect" : "Connect"))
	{
		if (tourBoxConnected) delete tourBox;

		tourBox = new TourBox("COM" + std::to_string(tourBoxComPort));
		tourBoxConnected = true;
	}

	ImGui::End();
}

bool Input::IsAnyKeyboardInput()
{
	for (int i = 0; i <= GLFW_KEY_LAST; i++) if (glfwGetKey(m_window, i) == GLFW_PRESS) return true;
	return false;
}

bool Input::IsAnyMouseInput()
{
	if (m_isMouseButtonInput) return true;
	if (m_isMousePointerInput) return true;

	return false;
}

bool Input::IsAnyGamepadInput()
{
	// Buttons
	for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) if (Input::Gamepad().Pressed(i)) return true;
	// Joysticks
	for (int i = 0; i <= GLFW_GAMEPAD_AXIS_RIGHT_Y; i++)
	{
		if (Input::Gamepad().AxesPressed(i)) return true;
		if (Input::Gamepad().AxesPressed(i,true)) return true;
	}
	// Triggers
	if (Input::Gamepad().AxesPressed(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER)) return true;
	if (Input::Gamepad().AxesPressed(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER)) return true;
	return false;
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
	if (s_instance->IsAnyKeyboardInput()) s_instance->m_lastInputType = InputType::Keyboard;

	if (!io.WantCaptureMouse)
	{
		for (auto& b : s_instance->mouseButtons)
			b.second.Update(s_instance->m_window, b.first);
	}

	// Mouse
	s_instance->m_lastMousePosition = s_instance->m_mousePosition;
	double mouseX, mouseY;
	glfwGetCursorPos(s_instance->m_window, &mouseX, &mouseY);
	s_instance->m_mousePosition = { mouseX, mouseY };
	
	// check for pointer delta
	s_instance->m_isMousePointerInput = (s_instance->m_mousePosition != s_instance->m_lastMousePosition);

	// check all mouse buttons
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		if (glfwGetMouseButton(s_instance->m_window, i))
		{
			s_instance->m_isMouseButtonInput;
			break;
		}
	}

	if (s_instance->IsAnyMouseInput()) s_instance->s_instance->m_lastInputType = InputType::Mouse;

	// Gamepad
	if (s_instance->isGamepadConnected)
	{
		s_instance->gamepad.statePrevious = s_instance->gamepad.stateCurrent;
		glfwGetGamepadState(GLFW_JOYSTICK_1, &s_instance->gamepad.stateCurrent);

		if (s_instance->IsAnyGamepadInput()) s_instance->s_instance->m_lastInputType = InputType::Gamepad;
	}

	// TourBox - oh yeah baby.
	if (s_instance->tourBox && s_instance->tourBoxConnected)
		s_instance->tourBox->Update();


	// Misc - move to graphics or something?
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

bool Input::IsAnyMousePointerInput()
{
	return s_instance->m_isMousePointerInput;
}

bool Input::IsAnyMouseButtonInput()
{
	return s_instance->m_isMouseButtonInput;
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
	for (auto& tourBoxButton : tourBoxButtonCodes) if (Input::TourBoxButtonDown(tourBoxButton)) return true;
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
	for (auto& tourBoxButton : tourBoxButtonCodes) if (Input::TourBoxButtonPressed(tourBoxButton)) return true;
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
	for (auto& tourBoxButton : tourBoxButtonCodes) if (Input::TourBoxButtonUp(tourBoxButton)) return true;
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

void Input::InputAlias::RegisterGamepadAxis(int GLFW_GAMEPAD_AXES, bool downIsNegativeValue)
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

void Input::InputAlias::RegisterTourBoxButton(TourBoxCode code)
{
	for (auto& tourBoxButtonCode : tourBoxButtonCodes) if (tourBoxButtonCode == code) return;
	tourBoxButtonCodes.emplace_back(code);
}
