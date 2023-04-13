#include "ComponentCamera.h"
#include "Graphics.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "ShaderManager.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"

ComponentCamera::ComponentCamera(Object* parent) : Component("Camera", Component_Camera, parent)
{
	view = componentParent->transform;
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;

	glm::vec2 vp = Window::GetViewPortSize();
	frameBuffer = new FrameBuffer(vp.x, vp.y, true);
	Scene::s_instance->cameras.push_back(frameBuffer);
	TextureManager::s_instance->AddFrameBuffer(componentParent->objectName.c_str(), frameBuffer);
	
	cameraGizmo = new Object(-1, "Camera Gizmo");
	ComponentModel* componentModel = new ComponentModel(parent);
	componentModel->model = ModelManager::GetModel("models/Gizmos/camera.fbx");
	cameraGizmo->components.push_back(componentModel);

	ComponentRenderer* componentRenderer = new ComponentRenderer(parent);
	componentRenderer->model = ModelManager::GetModel("models/Gizmos/camera.fbx");
	componentRenderer->texture = TextureManager::GetTexture("models/colour_blue.bmp");
	componentRenderer->shader = ShaderManager::GetShaderProgram("shaders/gizmoShader");
	cameraGizmo->components.push_back(componentRenderer);
	Scene::s_instance->gizmos.push_back(cameraGizmo);
}

ComponentCamera::ComponentCamera(Object* parent, std::istream& istream) : ComponentCamera(parent)
{
	FileUtils::ReadFloat(istream, nearClip);
	FileUtils::ReadFloat(istream, farClip);

	FileUtils::ReadFloat(istream, fieldOfView);
	FileUtils::ReadFloat(istream, aspect);
	dirtyConfig = true;
}

ComponentCamera::~ComponentCamera()
{
	auto cameraIt = Scene::s_instance->cameras.begin();
	while (cameraIt != Scene::s_instance->cameras.end())
	{
		if (*cameraIt == frameBuffer)
		{
			Scene::s_instance->cameras.erase(cameraIt);
			break;
		}
		cameraIt++;
	}
}

void ComponentCamera::Update(float deltatime)
{
	if (componentParent->dirtyTransform || dirtyConfig)
	{
		view = glm::inverse(componentParent->transform);
		projection = glm::perspective(glm::radians(fieldOfView), aspect, nearClip, farClip);
		matrix = projection * view;
		dirtyConfig = false;
	}
}

void ComponentCamera::DrawGUI()
{
	if (ImGui::SliderFloat("Near Clip", &nearClip, 0.001f, 5.0f))
		dirtyConfig = true;
	if (ImGui::SliderFloat("Far Clip", &farClip, 5.0f, 10000.0f))
		dirtyConfig = true;
	if (ImGui::SliderFloat("Field of View", &fieldOfView, 0, 180))
		dirtyConfig = true;
	if (ImGui::SliderFloat("Aspect Ratio", &aspect,0, 3))
		dirtyConfig = true;
	glm::vec2 size = { 100,100 };
	ImGui::Image((ImTextureID)(frameBuffer->GetTexture()->texID), { 260,147 }, {0,1}, {1,0});

}

void ComponentCamera::Write(std::ostream& ostream)
{
	FileUtils::WriteFloat(ostream, nearClip);
	FileUtils::WriteFloat(ostream, farClip );

	FileUtils::WriteFloat(ostream, fieldOfView);
	FileUtils::WriteFloat(ostream, aspect);
}