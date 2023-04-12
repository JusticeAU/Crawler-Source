#include "Camera.h"
#include "FrameBuffer.h"

Camera::Camera(float aspect, GLFWwindow* window, string name)
{
	position = { -6.25, 6.5, 16.38 };
	m_horizontal = -71.4f;
	m_vertical = -3.9f;

	this->aspect = aspect;

	if(s_instance == nullptr)
		s_instance = this;
	this->window = window;

	this->name = name;
	frameBuffer = new FrameBuffer(1600, 900);

	UpdateMatrix();
}

void Camera::Update(float delta)
{
	if (glfwGetMouseButton(window, 1)) // only accept inputs if rightclick is held down.
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // lock and hide the cursor whilst moving so it stays in the window.

		// Update camera rotation
		vec2 mouseDelta = Input::GetMouseDelta();
		m_horizontal += lookSpeed * mouseDelta.x;
		m_vertical -= lookSpeed * mouseDelta.y;
		// clamp to avoid gimbal lock
		m_vertical = glm::clamp(m_vertical, -80.0f, 80.0f);

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			Move(-right * moveSpeed * delta);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			Move(right * moveSpeed * delta);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			Move(forward * moveSpeed * delta);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			Move(-forward * moveSpeed * delta);

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			Move(up * moveSpeed * delta);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			Move(-up * moveSpeed * delta);

		UpdateMatrix();
	}
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // unlock the mouse cursor
}

void Camera::Move(glm::vec3 delta)
{
	position += delta;
}

void Camera::DrawGUI()
{
	ImGui::SetNextWindowPos({ 1300, 0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 273 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
	ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SliderFloat("Move Speed", &moveSpeed, 0.1f, 50.0f);
	ImGui::SliderFloat("Look Speed", &lookSpeed, 0.01f, 1.0f);
	if (ImGui::SliderFloat("Near Clip", &nearClip, 0.001f, 5.0f))
		UpdateMatrix();
	if(ImGui::SliderFloat("Far Clip", &farClip, 5.0f, 10000.0f))
		UpdateMatrix();

	if(ImGui::DragFloat3("Position", &position[0]))
		UpdateMatrix();
	if(ImGui::DragFloat2("Angle", &m_horizontal))
		UpdateMatrix();

	ImGui::Text("Controls:");
	ImGui::Text("Right Click +");
	ImGui::Text("Mouse to Look");
	ImGui::Text("WSAD Move");
	ImGui::Text("QE Down/Up");
	ImGui::Text("F10 Fullscreen");


	ImGui::End();
}

glm::mat4 Camera::GetMatrix() { return matrix; }

FrameBuffer* Camera::GetFrameBuffer()
{
	return frameBuffer;
}

void Camera::ResizeFrameBuffer(int width, int height)
{
	frameBuffer->Resize(width, height);
}

void Camera::UpdateMatrix()
{
	float thetaR = glm::radians(m_horizontal);
	float phiR = glm::radians(m_vertical);

	//calculate the forward, right and up axis for the camera
	forward = { cos(phiR) * cos(thetaR), sin(phiR), cos(phiR) * sin(thetaR) };
	right = { -sin(thetaR), 0, cos(thetaR) };
	up = { cos(thetaR) * -sin(phiR) , cos(phiR), -sin(phiR) * sin(thetaR)};

	view = glm::lookAt(position, position + forward, glm::vec3(0, 1, 0));
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;
}

Camera* Camera::s_instance = nullptr;