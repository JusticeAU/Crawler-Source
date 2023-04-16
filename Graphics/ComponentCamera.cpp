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
#include "PostProcess.h"

ComponentCamera::ComponentCamera(Object* parent) : Component("Camera", Component_Camera, parent)
{
	view = componentParent->transform;
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;

	glm::vec2 vp = Window::GetViewPortSize();
	frameBuffer = new FrameBuffer(vp.x, vp.y, true);
	//Scene::s_instance->cameras.push_back(frameBuffer);
	TextureManager::s_instance->AddFrameBuffer(componentParent->objectName.c_str(), frameBuffer);
	
	m_frameBufferProcessed = new FrameBuffer(vp.x, vp.y, true);
	Scene::s_instance->cameras.push_back(m_frameBufferProcessed);
	string processedFBName = componentParent->objectName + "_Processed";
	TextureManager::s_instance->AddFrameBuffer(processedFBName.c_str(), m_frameBufferProcessed);


	// In Scene Editor Camera Gizmo
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
	Scene::s_instance->componentCameras.push_back(this);
}

ComponentCamera::ComponentCamera(Object* parent, std::istream& istream) : ComponentCamera(parent)
{
	FileUtils::ReadFloat(istream, nearClip);
	FileUtils::ReadFloat(istream, farClip);

	FileUtils::ReadFloat(istream, fieldOfView);
	FileUtils::ReadFloat(istream, aspect);
	dirtyConfig = true;

	int postProcessCount;
	FileUtils::ReadInt(istream, postProcessCount);
	for (int i = 0; i < postProcessCount; i++)
	{
		string ppShaderName;
		FileUtils::ReadString(istream, ppShaderName);
		PostProcess* pp = new PostProcess(componentParent->objectName + "_PP_" + ppShaderName);
		pp->SetShader(ShaderManager::GetShaderProgram(ppShaderName));
		pp->SetShaderName(ppShaderName);

		m_postProcessStack.push_back(pp);

	}
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

	auto gizmoIt = Scene::s_instance->gizmos.begin();
	while (gizmoIt != Scene::s_instance->gizmos.end())
	{
		if (*gizmoIt == cameraGizmo)
		{
			Scene::s_instance->gizmos.erase(gizmoIt);
			break;
		}
		cameraIt++;
	}
	auto componentIt = Scene::s_instance->componentCameras.begin();
	while (componentIt != Scene::s_instance->componentCameras.end())
	{
		if (*componentIt == this)
		{
			Scene::s_instance->componentCameras.erase(componentIt);
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
	ImGui::Image((ImTextureID)(m_frameBufferProcessed->GetTexture()->texID), { 260,147 }, {0,1}, {1,0});
	
	if (ImGui::Button("Add Post Process Effect"))
		ImGui::OpenPopup("popup_add_pp_effect");

	// Draw Add Component Popup if requested.
	if (ImGui::BeginPopup("popup_add_pp_effect"))
	{
		ImGui::SameLine();
		ImGui::SeparatorText("Effect");
		for (int i = 0; i < ShaderManager::GetPostProcessShaderCount(); i++)
		{
			string ppName = ShaderManager::GetPostProcessShaderName(i);
			if (ImGui::Selectable(ppName.c_str()))
			{
				PostProcess* pp = new PostProcess(componentParent->objectName+"_PP_"+ppName);
				pp->SetShader(ShaderManager::GetShaderProgram(ppName));
				pp->SetShaderName(ppName);

				m_postProcessStack.push_back(pp);
			}

		}
		ImGui::EndPopup();
	}

	// List Post Processes
	for (int i = 0; i < m_postProcessStack.size(); i++)
	{
		ImGui::PushID(i);
		if (ImGui::Button("Delete"))
		{
			delete m_postProcessStack[i];
			m_postProcessStack.erase(m_postProcessStack.begin() + i);
			i--;
		}
		else
		{
			ImGui::SameLine();
			ImGui::Text(m_postProcessStack[i]->GetName().c_str());
		}
	}
}

void ComponentCamera::Write(std::ostream& ostream)
{
	FileUtils::WriteFloat(ostream, nearClip);
	FileUtils::WriteFloat(ostream, farClip );

	FileUtils::WriteFloat(ostream, fieldOfView);
	FileUtils::WriteFloat(ostream, aspect);

	// Write post process stack
	FileUtils::WriteInt(ostream, m_postProcessStack.size());
	for (auto pp : m_postProcessStack)
	{
		FileUtils::WriteString(ostream, pp->GetShaderName());
	}
}

void ComponentCamera::RunPostProcess()
{
	// bind output of first pass (raw render with no processing)
	frameBuffer->BindTexture(20);
	// loop through post processing stack
	for (auto postProcess : m_postProcessStack)
	{
		postProcess->BindFrameBuffer();
		postProcess->Process();
		postProcess->BindOutput();
	}
	//finally, take the output and send to the processed framebuffer
	// either the raw render will be bound, or the last postprocess that ran (if any)
	m_frameBufferProcessed->BindTarget();
	PostProcess::PassThrough();
	FrameBuffer::UnBindTarget();
	FrameBuffer::UnBindTexture(20);
	
}
