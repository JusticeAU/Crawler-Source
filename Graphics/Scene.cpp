#include "Scene.h"
#include "FileUtils.h"
#include "ModelManager.h"
#include "Model.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "ShaderManager.h"
#include "AudioManager.h"

#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Window.h"
#include "MeshManager.h"

#include "Camera.h"

#include "LogUtils.h"
#include "PostProcess.h"

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

Scene::Scene()
{
	// Create light pos and col arrays for sending to lit phong shader.
	m_pointLightPositions = new vec3[MAX_LIGHTS];
	m_pointLightColours = new vec3[MAX_LIGHTS];

	// Create our Light Gizmo for rendering - this will move to a ComponentLight and be handled there.
	lightGizmo = new Object(-1, "Light Gizmo");
	ComponentModel* lightGizmoModelComponent = new ComponentModel(lightGizmo);
	lightGizmoModelComponent->model = ModelManager::GetModel("models/Gizmos/bulb.fbx");
	lightGizmo->components.push_back(lightGizmoModelComponent);
	ComponentRenderer* lightGizmoRenderer = new ComponentRenderer(lightGizmo);
	gizmoShader = ShaderManager::GetShaderProgram("shaders/gizmoShader");
	lightGizmoRenderer->shader = gizmoShader;
	lightGizmo->components.push_back(lightGizmoRenderer);
	lightGizmoRenderer->OnParentChange();

	// Set up our framebuffer to render our chosen cameras framebuffer to.
	TextureManager::s_instance->AddFrameBuffer(Camera::s_instance->name.c_str(), Camera::s_instance->GetFrameBuffer());

	// Add editor camera to list of cameras and set our main camera to be it.
	cameras.push_back(Camera::s_instance->GetFrameBuffer());
	outputCameraFrameBuffer = cameras[0];

	// Object picking dev - Might be able to refactor this to be something that gets attached to a camera. It needs to be used by both scene editor camera, but also 'in game' camera - not unreasonable for these to be seperate implementations though.
	objectPickBuffer = new FrameBuffer(FrameBuffer::Type::ObjectPicker);
	TextureManager::s_instance->AddFrameBuffer("Objecting Picking Buffer", objectPickBuffer);

	// Shadow Mapping dev - This will get refactored in to something more robust once I've set up PBR.
	shadowMap = new FrameBuffer(FrameBuffer::Type::ShadowMap);
	shadowMapDevOutput = new FrameBuffer(FrameBuffer::Type::PostProcess);
	depthMapOutputShader = ShaderManager::GetShaderProgram("shaders/zzShadowMapDev");
}
Scene::~Scene()
{
	for (auto o : objects)
	{
		o->DeleteAllChildren();
		delete o;
	}

	objects.clear();
}

void Scene::Init()
{
	s_instance = new Scene();
	Scene::SetClearColour({ 0.25f, 0.25, 0.25 });
}

void Scene::Update(float deltaTime)
{
	UpdateInputs();

	// Updatee all Objects
	for (auto o : objects)
		o->Update(deltaTime);

	// Update point light arrays - right now we send this in as ivec and vec4s - should turn this in to a more dynamic uniform buffer.
	for (int i = 0; i < m_pointLights.size(); i++)
	{
		m_pointLightColours[i] = m_pointLights[i].colour * m_pointLights[i].intensity;
		m_pointLightPositions[i] = m_pointLights[i].position;
	}
}
void Scene::UpdateInputs()
{
	if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_D).Down())
		DuplicateObject(selectedObject);

	if (Input::Mouse(0).Down() && !ImGuizmo::IsOver())
	{
		requestedObjectSelection = true;
		requestedSelectionPosition = Input::GetMousePosPixel();
	}

	UpdateMousePosOnGrid();
}

void Scene::Render()
{
	float startTime = glfwGetTime();

	RenderShadowMaps();
	RenderSceneCameras();
	if (requestedObjectSelection)
		RenderObjectPicking();
	RenderEditorCamera();

	float endTime = glfwGetTime();

	renderTime = endTime - startTime;
}
void Scene::RenderShadowMaps()
{
	shadowMap->BindTarget();
	glClear(GL_DEPTH_BUFFER_BIT);
	// Generate shadow map VPM and camera pos
	lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
	lightView = glm::lookAt(Camera::s_instance->GetPosition(),
		Camera::s_instance->GetPosition() + Scene::GetSunDirection(),
		glm::vec3(0.0f, 1.0f, 0.0f));
	lightSpaceMatrix = lightProjection * lightView;
	for (auto& o : objects)
		o->Draw(lightSpaceMatrix, { 0,0,0 }, Component::DrawMode::ShadowMapping);

	FrameBuffer::UnBindTarget();
}
void Scene::RenderSceneCameras()
{
	// Bind recently rendered shadowmap.
	shadowMap->BindTexture(20);
	shadowMapDevOutput->BindTarget();
	PostProcess::PassThrough(depthMapOutputShader);

	shadowMap->BindTexture(5);
	// for each camera in each object, draw to that cameras frame buffer
	for (auto& c : componentCameras)
	{
		c->SetAsRenderTarget();
		c->UpdateViewProjectionMatrix();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		vec3 cameraPosition = c->GetWorldSpacePosition();
		for (auto& o : objects)
			o->Draw(c->GetViewProjectionMatrix(), cameraPosition, Component::DrawMode::Standard);
		c->RunPostProcess();
	}
}
void Scene::RenderObjectPicking()
{
	objectPickBuffer->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto& o : objects)
		o->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::ObjectPicking);

	SetSelectedObject(objectPickBuffer->GetObjectID(requestedSelectionPosition.x, requestedSelectionPosition.y));
	requestedObjectSelection = false;
}
void Scene::RenderEditorCamera()
{
	// then draw the scene using the Camera class "Editor Camera"
	Camera::s_instance->GetFrameBuffer()->BindTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& o : objects)
		o->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);

	FrameBuffer::UnBindTarget();
}

void Scene::UpdateMousePosOnGrid()
{
	vec2 NDC = Input::GetMousePosNDC();

	vec3 rayStart = Camera::s_instance->position;
	vec3 rayDir = Camera::s_instance->GetRayFromNDC(NDC);

	float scale = rayStart.y / rayDir.y;
	vec3 groundPos = rayStart - (rayDir * scale);
	gridSelected.x = groundPos.x / GRID_SCALE;
	gridSelected.y = groundPos.z / GRID_SCALE;
}

void Scene::DrawGizmos()
{
	// render light gizmos only to main 'editor' camera
	// quick wireframe rendering. Will later set up something that renders a quad billboard at the location or something.
	Camera::s_instance->GetFrameBuffer()->BindTarget();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gizmoShader->Bind();
	for (auto &light : m_pointLights)
	{
		gizmoShader->SetVectorUniform("gizmoColour", light.colour);

		vec3 localPosition, localRotation, localScale;
		localPosition = light.position;
		localScale = { 0.2f, 0.2f, 0.2f, };
		localRotation = { 0, 0, 0 };
		ImGuizmo::RecomposeMatrixFromComponents((float*)&localPosition, (float*)&localRotation, (float*)&localScale, (float*)&lightGizmo->transform);
		lightGizmo->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);
	}

	// Draw cameras (from gizmo list, all gizmos should move to here)
	gizmoShader->SetVectorUniform("gizmoColour", { 1,1,1 });
	for (auto &o : gizmos)
	{
		o->Draw(Camera::s_instance->GetMatrix(), Camera::s_instance->GetPosition(), Component::DrawMode::Standard);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	FrameBuffer::UnBindTarget();
}
void Scene::DrawCameraToBackBuffer()
{
	outputCameraFrameBuffer->BindTexture(20);
	PostProcess::PassThrough();
}
void Scene::DrawGUI()
{	
	ImGui::Begin("World Edit");
	ImGui::BeginDisabled();
	ImGui::DragInt2("Grid Selected", &gridSelected.x);
	ImGui::EndDisabled();
	ImGui::End();


	ImGui::Begin("Shadow Map Dev");
	ImGui::PushID(6969);

	ImGui::BeginDisabled();
	ImGui::InputFloat("Render Time", &renderTime, 0,0, "%0.6f");
	ImGui::EndDisabled();

	ImGui::DragFloat("Ortho Near",		&orthoNear);
	ImGui::DragFloat("Ortho Far",		&orthoFar);
	ImGui::DragFloat("Ortho Left",		&orthoLeft);
	ImGui::DragFloat("Ortho Right",		&orthoRight);
	ImGui::DragFloat("Ortho Bottom",	&orthoBottom);
	ImGui::DragFloat("Ortho Top",		&orthoTop);
	ImGui::Image((ImTextureID)(shadowMapDevOutput->GetTexture()->texID), { 512,512 }, { 0,1 }, { 1,0 });
	
	ImGui::PopID();
	ImGui::End();

	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene",0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	if (ImGui::Button("Save"))
		Save();
	ImGui::SameLine();
	if (ImGui::Button("Load"))
	{
		ImGui::OpenPopup("popup_load_scene");
		//Load();
	}

	// Draw scene file list if requested
	if (ImGui::BeginPopup("popup_load_scene"))
	{
		ImGui::SameLine();
		ImGui::SeparatorText("Scene Name");
		for (auto d : fs::recursive_directory_iterator("scenes"))
		{
			if (d.path().has_extension() && d.path().extension() == ".scene")
			{
				string foundSceneName = d.path().filename().string();
				string foundScenePath = d.path().relative_path().string();
				if (ImGui::Selectable(foundScenePath.c_str()))
				{
					sceneFilename = foundSceneName;
					Load(); // TODO - sanitize these paths properly 
				}
			}
		}
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::InputText("Name", &sceneFilename);

	ImGui::BeginDisabled();
	int obj = selectedObjectID;
	ImGui::InputInt("Selected Object", &obj);
	ImGui::EndDisabled();

	if (ImGui::InputInt("Camera Number", &cameraIndex, 1))
	{
		if (cameraIndex > cameras.size() - 1)
			cameraIndex = cameras.size() - 1;
		outputCameraFrameBuffer = cameras[cameraIndex];


		// This is fairly hacky. maybe the cameras should handle setting the audio listener themselves.
		// Perhaps scenee camera and editor camera can become same thing and then audio manager can just look at camera::main or something
		if (cameraIndex == 0)
			AudioManager::SetAudioListener(Camera::s_instance->GetAudioListener());
		else
			AudioManager::SetAudioListener(componentCameras[cameraIndex - 1]->GetAudioListener());
	}

	float clearCol[3] = { clearColour.r, clearColour.g, clearColour.b, };
	if (ImGui::ColorEdit3("Clear Colour", clearCol))
		Scene::SetClearColour({ clearCol[0], clearCol[1], clearCol[2] });
	
	if (ImGui::CollapsingHeader("Scene Lighting"))
	{
		float ambientCol[3] = { m_ambientColour.r, m_ambientColour.g, m_ambientColour.b, };
		if (ImGui::ColorEdit3("Ambient Light", ambientCol))
			SetAmbientLightColour({ ambientCol[0], ambientCol[1], ambientCol[2] });
		
		float sunCol[3] = { m_sunColour.r, m_sunColour.g, m_sunColour.b, };
		if (ImGui::ColorEdit3("Sun Colour", sunCol))
			SetSunColour({ sunCol[0], sunCol[1], sunCol[2] });

		float sunDir[3] = { m_sunDirection.x, m_sunDirection.y, m_sunDirection.z, };
		if (ImGui::SliderFloat3("Sun Direction", &sunDir[0], -1, 1, "%.3f"))
			SetSunDirection({ sunDir[0], sunDir[1], sunDir[2] });

		unsigned int lightsAtStartOfFrame = m_pointLights.size();
		if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::BeginDisabled();
		if (ImGui::Button("New Point Light"))
			m_pointLights.push_back(Light());
		if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::EndDisabled();

		// Draw all point lights
		for (int i = 0; i < m_pointLights.size(); i++)
		{
			ImGui::PushID(i);
			float pointCol[3] = { m_pointLights[i].colour.r, m_pointLights[i].colour.g, m_pointLights[i].colour.b,};
			if (ImGui::ColorEdit3("Point Light Colour", &pointCol[0]))
				m_pointLights[i].colour = { pointCol[0], pointCol[1], pointCol[2] };

			float pointPos[3] = { m_pointLights[i].position.x, m_pointLights[i].position.y, m_pointLights[i].position.z, };
			if (ImGui::DragFloat3("Point Light Position", &pointPos[0]))
				m_pointLights[i].position = { pointPos[0], pointPos[1], pointPos[2] };

			ImGui::DragFloat("Intensity", &m_pointLights[i].intensity);
			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				m_pointLights.erase(m_pointLights.begin() + i);
				i--;
			}
			ImGui::PopID();
		}
	}

	if (ImGui::Button("New Object"))
		Scene::CreateObject();
	
	drawn3DGizmo = false;
	for (auto o : objects)
		o->DrawGUI();

	ImGui::End();

	if (Scene::GetSelectedObject() > 0)
	{
		// Draw Guizmo - very simple implementation - TODO have a 'selected object' context and mousewheel scroll through translate, rotate, scale options - rotate will need to be reworked.
		ImGuizmo::SetRect(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
		mat4 view, projection;
		view = Camera::s_instance->GetView();
		projection = Camera::s_instance->GetProjection();
		if (ImGuizmo::Manipulate((float*)&view, (float*)&projection, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, (float*)&s_instance->selectedObject->localTransform))
			s_instance->selectedObject->dirtyTransform = true;
		Scene::s_instance->drawn3DGizmo = true;
	}
}

// This will destroy all objects (and their children) marked for deletion.
void Scene::CleanUp()
{
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->CleanUpChildren();
		if (objects[i]->markedForDeletion)
		{
			delete objects[i];
			objects.erase(objects.begin() + i);
			i--;
		}
	}
}

// Creates and object and adds it to the parent or scene hierarchy. If you want an object but dont want it added to the heirarchy, then call new Object.
Object* Scene::CreateObject(Object* parent)
{
	Object* o = new Object(s_instance->objectCount++);
	if (parent)
	{
		o->parent = parent;
		parent->children.push_back(o);
	}
	else
		s_instance->objects.push_back(o);

	return o;
}
Object* Scene::CreateObject(string name, Object* parent)
{
	Object* o = Scene::CreateObject(parent);
	o->objectName = name;
	return o;
}
Object* Scene::DuplicateObject(Object* object)
{
	Object* o = CreateObject(object->parent);
	o->objectName = object->objectName;

	for (auto component : object->components)
	{
		o->components.push_back(component->Clone(o));
	}

	o->localTransform = object->localTransform;
	o->dirtyTransform = true;
	o->RefreshComponents();
	SetSelectedObject(o->id);

	return o;
}

void Scene::SetClearColour(vec3 clearColour)
{
	s_instance->clearColour = clearColour;
	glClearColor(clearColour.r, clearColour.g, clearColour.b, 1);
}

vec3 Scene::GetSunColour()
{
	return s_instance->m_sunColour;
}
void Scene::SetSunColour(vec3 sunColour)
{
	s_instance->m_sunColour = sunColour;
}

vec3 Scene::GetSunDirection()
{
	return glm::normalize(s_instance->m_sunDirection);
}
void Scene::SetSunDirection(vec3 sunDirection)
{
	s_instance->m_sunDirection = sunDirection;
}

vec3 Scene::GetAmbientLightColour()
{
	return s_instance->m_ambientColour;
}
void Scene::SetAmbientLightColour(vec3 ambientColour)
{
	s_instance->m_ambientColour = ambientColour;
}

int Scene::GetNumPointLights()
{
	return (int)s_instance->m_pointLights.size();
}

void Scene::SetSelectedObject(unsigned int selected)
{
	s_instance->selectedObjectID = selected;
	s_instance->selectedObject = Scene::FindObjectWithID(selected);
}

Object* Scene::FindObjectWithID(unsigned int id)
{
	for (auto o : s_instance->objects)
	{
		if (o->FindObjectWithID(id) != nullptr) return o;
	}

	return nullptr;
}

void Scene::Save()
{
	// Create/Open file for writing
	std::ofstream out((sceneSubfolder+sceneFilename).c_str(), std::ofstream::binary);

	// Serialize Scene Configuration
	
	// Clear Colour
	FileUtils::WriteVec(out, clearColour);
	FileUtils::WriteVec(out, m_ambientColour);
	
	// Scene Light Configuration
	FileUtils::WriteVec(out, m_sunColour);
	FileUtils::WriteVec(out, m_sunDirection);
	
	// Point Lights
	int numPointLights = (int)m_pointLights.size();
	FileUtils::WriteInt(out, numPointLights);
	for (int i = 0; i < numPointLights; i++)
	{
		FileUtils::WriteVec(out, m_pointLights[i].position);
		FileUtils::WriteVec(out, m_pointLights[i].colour);
		FileUtils::WriteFloat(out, m_pointLights[i].intensity);
	}
	
	// Serialize Objects and Child Objects recursively
	int numObjects = (int)objects.size();
	FileUtils::WriteInt(out, numObjects);
	for (int i = 0; i < numObjects; i++)
		objects[i]->Write(out);

	out.flush();
	out.close();
}

void Scene::Load()
{

	// Create/Open file for writing
	std::ifstream in((sceneSubfolder+sceneFilename).c_str(), std::ifstream::binary);

	// Serialize Scene Configuration
	
	// Clear Colour
	FileUtils::ReadVec(in, clearColour);
	SetClearColour(clearColour);
	FileUtils::ReadVec(in, m_ambientColour);
	
	// Scene Light Configuration
	FileUtils::ReadVec(in, m_sunColour);
	FileUtils::ReadVec(in, m_sunDirection);
	
	// Point Lights
	int numPointLights;
	FileUtils::ReadInt(in, numPointLights);
	m_pointLights.resize(numPointLights);
	for (int i = 0; i < numPointLights; i++)
	{
		FileUtils::ReadVec(in, m_pointLights[i].position);
		FileUtils::ReadVec(in, m_pointLights[i].colour);
		FileUtils::ReadFloat(in, m_pointLights[i].intensity);
	}
	
	// Clear current objects
	for (auto object = objects.begin(); object != objects.end(); object++)
	{
		auto obj = *object;
		delete obj;
	}
	objects.clear();
	gizmos.clear();

	//  Objects and Child Objects
	int numObjects;
	FileUtils::ReadInt(in, numObjects);
	for(int i = 0; i < numObjects; i++)
	{
		auto o = Scene::CreateObject();
		o->Read(in);
	}
	in.close();

	for (auto o : objects)
	{
		o->RefreshComponents();
	}
}

mat4 Scene::GetLightSpaceMatrix()
{
	return s_instance->lightSpaceMatrix;
}

Scene* Scene::s_instance = nullptr;