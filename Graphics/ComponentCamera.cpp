#include "ComponentCamera.h"
#include "Graphics.h"
#include "FrameBuffer.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "ShaderManager.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "PostProcess.h"
#include "Model.h"

ComponentCamera::ComponentCamera(Object* parent, bool noGizmo) : Component("Camera", Component_Camera, parent)
{
	// Hold on to your seatbelt.
	// Set base matrix.
	view = componentParent->transform;
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;

	// In Scene Editor Camera Gizmo - It'd be nice to move some of this info out of here. Perhaps a Gizmo factory or something.
	if (!noGizmo)
	{
		cameraGizmo = new Object(-1, "Camera Gizmo");
		ComponentModel* componentModel = new ComponentModel(parent);
		componentModel->model = ModelManager::GetModel("engine/model/Gizmos/camera.fbx");
		componentModel->model->modelTransform = glm::rotate(glm::scale(mat4(1),glm::vec3(0.4f)), glm::radians(90.0f), {1,0,0});

		cameraGizmo->components.push_back(componentModel);

		ComponentRenderer* componentRenderer = new ComponentRenderer(parent);
		componentRenderer->model = ModelManager::GetModel("engine/model/Gizmos/camera.fbx");
		componentRenderer->materialArray.resize(1);
		componentRenderer->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/Gizmos.material");


		cameraGizmo->components.push_back(componentRenderer);

		Scene::s_instance->gizmos.push_back(cameraGizmo); // the scene gizmo renderer needs to be aware of this component.
	}
	Scene::s_instance->componentCameras.push_back(this); // Must be added here so the scene can render all in-scene cameras before rendering itself.
}

ComponentCamera::ComponentCamera(Object* parent, ordered_json j) : ComponentCamera(parent)
{
	if(j.contains("nearClip"))
		j.at("nearClip").get_to(nearClip);
	if (j.contains("farClip"))
		j.at("farClip").get_to(farClip);
	if (j.contains("fieldOfView"))
		j.at("fieldOfView").get_to(fieldOfView);
	if (j.contains("aspect"))
		j.at("aspect").get_to(aspect);
	dirtyConfig = true;

	if (j.contains("postProcess"))
	{
		auto ppJSON = j.at("postProcess");
		if (ppJSON.is_null() != true)
		{
			for (auto it = ppJSON.begin(); it != ppJSON.end(); it++)
			{
				PostProcess* pp = new PostProcess(componentParent->objectName + "_PP_" + (string)it.value());
				pp->SetShader(ShaderManager::GetShaderProgram(it.value()));
				pp->SetShaderName(it.value());
				m_postProcessStack.push_back(pp);
			}
		}
	}

}

ComponentCamera::~ComponentCamera()
{
	// remove self from scene gizmo list.
	auto gizmoIt = Scene::s_instance->gizmos.begin();
	while (gizmoIt != Scene::s_instance->gizmos.end())
	{
		if (*gizmoIt == cameraGizmo)
		{
			Scene::s_instance->gizmos.erase(gizmoIt);
			break;
		}
		gizmoIt++;
	}

	// remove self from scene component camera list.
	auto componentIt = Scene::s_instance->componentCameras.begin();
	while (componentIt != Scene::s_instance->componentCameras.end())
	{
		if (*componentIt == this)
		{
			Scene::s_instance->componentCameras.erase(componentIt);
			break;
		}
		componentIt++;
	}
	 // clear post process stack
	for (auto &pp : m_postProcessStack)
	{
		delete pp;
	}
}

void ComponentCamera::Update(float deltatime)
{
	UpdateAudioListener();
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

void ComponentCamera::UpdateViewProjectionMatrix()
{
	vec3 position = componentParent->GetWorldSpacePosition();
	view = glm::lookAt(position, position + componentParent->forward, componentParent->up);
	projection = glm::perspective(glm::radians(fieldOfView), aspect, nearClip, farClip);
	matrix = projection * view;
}

const vec3 ComponentCamera::GetRayFromNDC(glm::vec2 NDC)
{
	// convert to clip space
	glm::vec4 clipSpace = { NDC.x, NDC.y, -1.0f, 1.0f };
	// convert to eye space
	glm::mat4 inverseProjection = glm::inverse(projection);
	glm::vec4 eyeSpace = inverseProjection * clipSpace;
	eyeSpace.z = -1.0f;  eyeSpace.w = 0.0f;
	// convert to world space
	glm::mat4 inverseView = glm::inverse(view);
	glm::vec4 worldSpace = inverseView * eyeSpace;
	vec3 mouseRay{ worldSpace.x, worldSpace.y, worldSpace.z };
	mouseRay = glm::normalize(mouseRay);

	return mouseRay;
}

void ComponentCamera::SetAspect(float aspect)
{
	this->aspect = aspect;
}

// If there are any post process effects applied, they are processed here. Regardless of if there are effects or not, the frame buffer is transfered from Raw to Processed.
void ComponentCamera::RunPostProcess()
{
	Scene::ssao_ssaoBlurFBO->BindTexture(21);
	// loop through post processing stack
	for (auto postProcess : m_postProcessStack)
	{
		postProcess->BindFrameBuffer();
		postProcess->Process();
		postProcess->BindOutput();
	}
	//finally, take the output and send to the processed framebuffer
	// either the raw render will be bound, or the last postprocess that ran (if any)
	Scene::s_instance->m_frameBufferProcessed->BindTarget();
	PostProcess::PassThrough();
	FrameBuffer::UnBindTarget();
	FrameBuffer::UnBindTexture(20);
}

void ComponentCamera::UpdateAudioListener()
{
	m_audioListener.position = componentParent->GetWorldSpacePosition();
	glm::mat4 transform = componentParent->transform;

	const vec3 forward = normalize(glm::vec3(transform[2]));
	m_audioListener.forward.x = -forward.x;
	m_audioListener.forward.y = -forward.y;
}
