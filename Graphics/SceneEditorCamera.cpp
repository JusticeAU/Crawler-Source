#include "SceneEditorCamera.h"
#include "Object.h";
#include "ComponentFactory.h";
#include "AudioManager.h"
#include "Scene.h"
#include "ShaderManager.h"
#include "Primitives.h"

#include "Input.h"
#include "Window.h"

SceneEditorCamera::SceneEditorCamera()
{
	const vec3 defaultPosition(-7.5, 7.5, 15);
	const float defaultZ = -135.0f;
	const float defaultX = -45.0f;

	object = new Object(0, "Editor Camera");
	object->SetLocalPosition(defaultPosition);
	object->SetLocalRotationZ(defaultZ);
	object->SetLocalRotationX(defaultX);

	camera = new ComponentCamera(object, true);
	object->components.push_back(camera);
	AudioManager::SetAudioListener(camera->GetAudioListener());

	// Add to Scene Camera - this post process is now part of the rendering pipeline rahter than a specific camera effect. it needs to happen before the transparensy pass.
	//string ppName = "engine/shader/postProcess/SSAO";
	//PostProcess* pp = new PostProcess(camera->GetComponentParentObject()->objectName + "_PP_" + ppName);
	//pp->SetShader(ShaderManager::GetShaderProgram(ppName));
	//pp->SetShaderName(ppName);
	//camera->m_postProcessStack.push_back(pp);
}

void SceneEditorCamera::Update(float deltaTime)
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

		vec3 lookRotation = { 0,0,0 };
		lookRotation.z = -mouseDelta.x;
		lookRotation.x = -mouseDelta.y;
		object->AddLocalRotation(lookRotation * lookSpeed);

		if (Input::Keyboard(GLFW_KEY_A).Pressed() || Input::Keyboard(GLFW_KEY_LEFT).Pressed())
			object->AddLocalPosition(-object->right * moveSpeed * deltaTime);
		if (Input::Keyboard(GLFW_KEY_D).Pressed() || Input::Keyboard(GLFW_KEY_RIGHT).Pressed())
			object->AddLocalPosition(object->right * moveSpeed * deltaTime);

		if (Input::Keyboard(GLFW_KEY_W).Pressed() || Input::Keyboard(GLFW_KEY_UP).Pressed())
			object->AddLocalPosition(object->forward * moveSpeed * deltaTime);
		if (Input::Keyboard(GLFW_KEY_S).Pressed() || Input::Keyboard(GLFW_KEY_DOWN).Pressed())
			object->AddLocalPosition(-object->forward * moveSpeed * deltaTime);

		if (Input::Keyboard(GLFW_KEY_E).Pressed())
			object->AddLocalPosition(object->up * moveSpeed * deltaTime);
		if (Input::Keyboard(GLFW_KEY_Q).Pressed())
			object->AddLocalPosition(-object->up * moveSpeed * deltaTime);

		camera->UpdateFrustum();
	}
	object->Update(deltaTime);
}

void SceneEditorCamera::DrawGUI()
{
		ImGui::SetNextWindowPos({ 1300, 0 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize({ 300, 273 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
		ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::SliderFloat("Move Speed", &moveSpeed, 0.1f, 50.0f);
		ImGui::SliderFloat("Look Speed", &lookSpeed, 0.01f, 0.5f);
		ImGui::SliderFloat("Near Clip", &camera->nearClip, 0.001f, 5.0f);
		ImGui::SliderFloat("Far Clip", &camera->farClip, 5.0f, 10000.0f);
		ImGui::InputFloat3("Position", &object->localPosition.x);
		ImGui::InputFloat3("Rotation", &object->localRotation.x);


		ImGui::Text("Controls:");
		ImGui::Text("Right Click +");
		ImGui::Text("Mouse to Look");
		ImGui::Text("WSAD Move");
		ImGui::Text("QE Down/Up");
		ImGui::Text("F10 Fullscreen");


		ImGui::End();
}
