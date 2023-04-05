#include "Camera.h"

Camera::Camera(float aspect, GLFWwindow* window)
{
	position = { -6.25, 6.5, 16.38 };
	m_horizontal = -71.4f;
	m_vertical = -3.9f;

	this->aspect = aspect;

	view = glm::translate(glm::mat4(1), position);
	projection = glm::perspective((float)3.14159 / 4, aspect, .1f, 100.0f);
	matrix = projection * view;

	s_instance = this;
	this->window = window;

	UpdateMatrix();
}

void Camera::Update(float delta)
{
	if (glfwGetMouseButton(window, 1))
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Camera::Move(glm::vec3 delta)
{
	position += delta;
}

void Camera::DrawGUI()
{
	ImGui::Begin("Camera");
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

	ImGui::End();
}

glm::mat4 Camera::GetMatrix() { return matrix; }

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