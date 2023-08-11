#include "Camera.h"
#include "FrameBuffer.h"
#include "Window.h"
#include "Input.h"

Camera::Camera(float aspect, string name)
{
	this->aspect = aspect;

	if(s_instance == nullptr)
		s_instance = this;

	this->name = name;
	glm::ivec2 vp = Window::GetViewPortSize();
	frameBuffer = new FrameBuffer(FrameBuffer::Type::CameraTargetMSAA);
	frameBufferBlit = new FrameBuffer(FrameBuffer::Type::CameraTargetBlit);

	UpdateMatrix();
	UpdateAudioListener();
}

void Camera::Update(float delta)
{
	if (Input::Mouse(1).Down())
	{
		Window::Get()->SetMouseCursorHidden(true);
		isAdjusting = true;
	}
	else if (Input::Mouse(1).Up())
	{
		Window::Get()->SetMouseCursorHidden(false);
		isAdjusting = false;
	}

	if (isAdjusting) // only accept inputs if rightclick is held down.
	{
		// Update camera rotation
		vec2 mouseDelta = Input::GetMouseDelta();
		m_horizontal -= lookSpeed * mouseDelta.x;
		m_vertical -= lookSpeed * mouseDelta.y;
		// clamp to avoid gimbal lock
		m_vertical = glm::clamp(m_vertical, -89.9f, 89.9f);

		if (Input::Keyboard(GLFW_KEY_A).Pressed() || Input::Keyboard(GLFW_KEY_LEFT).Pressed())
			Move(right * moveSpeed * delta);
		if (Input::Keyboard(GLFW_KEY_D).Pressed() || Input::Keyboard(GLFW_KEY_RIGHT).Pressed())
			Move(-right * moveSpeed * delta);

		if (Input::Keyboard(GLFW_KEY_W).Pressed() || Input::Keyboard(GLFW_KEY_UP).Pressed())
			Move(forward * moveSpeed * delta);
		if (Input::Keyboard(GLFW_KEY_S).Pressed() || Input::Keyboard(GLFW_KEY_DOWN).Pressed())
			Move(-forward * moveSpeed * delta);

		if (Input::Keyboard(GLFW_KEY_E).Pressed())
			Move(up * moveSpeed * delta);
		if (Input::Keyboard(GLFW_KEY_Q).Pressed())
			Move(-up * moveSpeed * delta);

		UpdateMatrix();
		UpdateAudioListener();
	}
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

void Camera::BlitFrameBuffer()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer->GetID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferBlit->GetID());
	glBlitFramebuffer(0, 0, frameBuffer->GetWidth(), frameBuffer->GetHeight(), 0, 0, frameBufferBlit->GetWidth(), frameBufferBlit->GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

vec3 Camera::GetRayFromNDC(vec2 NDC)
{
	// convert to clip space
	glm::vec4 clipSpace = { NDC.x, NDC.y, -1.0f, 1.0f };
	// convert to eye space
	glm::mat4 inverseProjection = glm::inverse(projection);
	glm::vec4 eyeSpace = inverseProjection * clipSpace;
	eyeSpace.z = -1.0f;  eyeSpace.w = 0.0f; // review this
	// convert to world space
	glm::mat4 inverseView = glm::inverse(view);
	glm::vec4 worldSpace = inverseView * eyeSpace;
	vec3 mouseRay{ worldSpace.x, worldSpace.y, worldSpace.z };
	mouseRay = glm::normalize(mouseRay);

	return mouseRay;
}

void Camera::UpdateMatrix()
{
	float thetaR = glm::radians(m_horizontal);
	float phiR = glm::radians(m_vertical);

	//calculate the forward, right and up axis for the camera
	forward = { cos(phiR) * cos(thetaR), cos(phiR) * sin(thetaR), sin(phiR) };
	right = { -sin(thetaR), cos(thetaR), 0 };
	up = { cos(thetaR) * -sin(phiR), -sin(phiR) * sin(thetaR), cos(phiR) };

	view = glm::lookAt(position, position + forward, glm::vec3(0, 0, 1));
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;
}

void Camera::UpdateAudioListener()
{
	m_audioListener.position = position;
	m_audioListener.forward = forward;
}

Camera* Camera::s_instance = nullptr;