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
	// Hold on to your seatbelt.
	// Set base matrix.
	view = componentParent->transform;
	projection = glm::perspective((float)3.14159 / 4, aspect, nearClip, farClip);
	matrix = projection * view;
	
	// Initilise framebuffers. We create two initially. 1 to render the scene to, then a 2nd for the final post processing effects to land to.
	glm::vec2 vp = Window::GetViewPortSize();
	m_frameBufferRaw = new FrameBuffer(vp.x, vp.y, true);
	TextureManager::s_instance->AddFrameBuffer(componentParent->objectName.c_str(), m_frameBufferRaw); // add the texture to the manager so we can bind it to meshes and stuff.
	
	m_frameBufferProcessed = new FrameBuffer(vp.x, vp.y, true);
	Scene::s_instance->cameras.push_back(m_frameBufferProcessed);
	string processedFBName = componentParent->objectName + "_Processed";
	TextureManager::s_instance->AddFrameBuffer(processedFBName.c_str(), m_frameBufferProcessed);


	// In Scene Editor Camera Gizmo - It'd be nice to move some of this info out of here. Perhaps a Gizmo factory or something.
	cameraGizmo = new Object(-1, "Camera Gizmo");
	ComponentModel* componentModel = new ComponentModel(parent);
	componentModel->model = ModelManager::GetModel("models/Gizmos/camera.fbx");
	cameraGizmo->components.push_back(componentModel);

	ComponentRenderer* componentRenderer = new ComponentRenderer(parent);
	componentRenderer->model = ModelManager::GetModel("models/Gizmos/camera.fbx");
	componentRenderer->texture = TextureManager::GetTexture("models/colour_blue.bmp");
	componentRenderer->shader = ShaderManager::GetShaderProgram("shaders/gizmoShader");
	cameraGizmo->components.push_back(componentRenderer);
	
	Scene::s_instance->gizmos.push_back(cameraGizmo); // the scene gizmo renderer needs to be aware of this component.
	Scene::s_instance->componentCameras.push_back(this); // Must be added here so the scene can render all in-scene cameras before rendering itself.
}

ComponentCamera::ComponentCamera(Object* parent, std::istream& istream) : ComponentCamera(parent)
{
	FileUtils::ReadFloat(istream, nearClip);
	FileUtils::ReadFloat(istream, farClip);

	FileUtils::ReadFloat(istream, fieldOfView);
	FileUtils::ReadFloat(istream, aspect);
	dirtyConfig = true;

	// see if there were any post process effects applied and create them.
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
	// Remove self from scene camera list.
	auto cameraIt = Scene::s_instance->cameras.begin();
	while (cameraIt != Scene::s_instance->cameras.end())
	{
		if (*cameraIt == m_frameBufferProcessed)
		{
			Scene::s_instance->cameras.erase(cameraIt);
			break;
		}
		cameraIt++;
	}

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

	// delete Raw and Processed FBs from manager, then from memory.
	TextureManager::s_instance->RemoveFrameBuffer(componentParent->objectName.c_str());
	delete m_frameBufferRaw;

	string processedFBName = componentParent->objectName + "_Processed";
	TextureManager::s_instance->RemoveFrameBuffer(processedFBName.c_str());
	delete m_frameBufferProcessed;

}

void ComponentCamera::Update(float deltatime)
{
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
		FileUtils::WriteString(ostream, pp->GetShaderName()); // All thats needed at this point in time.
}

void ComponentCamera::UpdateViewProjectionMatrix()
{
	view = glm::inverse(componentParent->transform);
	projection = glm::perspective(glm::radians(fieldOfView), aspect, nearClip, farClip);
	matrix = projection * view;
}

// binds the cameras base framebuffer, ready to render meshes to. This clears the color and depth buffer of the camera too.
void ComponentCamera::SetAsRenderTarget()
{
	m_frameBufferRaw->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// If there are any post process effects applied, they are processed here. Regardless of if there are effects or not, the frame buffer is transfered from Raw to Processed.
void ComponentCamera::RunPostProcess()
{
	// bind output of first pass (raw render with no processing)
	m_frameBufferRaw->BindTexture(20);
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