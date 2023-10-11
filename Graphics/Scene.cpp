#include "Scene.h"
#include "SceneEditorCamera.h"
#include "SceneRenderer.h"

#include "FileUtils.h"
#include "ModelManager.h"
#include "Model.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "ComponentFactory.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "AudioManager.h"

#include "FrameBuffer.h"
#include "TextureManager.h"
#include "Window.h"
#include "MeshManager.h"

#include "MathUtils.h"
#include "LogUtils.h"
#include "PostProcess.h"

#include "Input.h"

#include "serialisation.h"

#include <fstream>
#include <filesystem>
#include <random>
namespace fs = std::filesystem;

Scene* Scene::s_instance = nullptr;
SceneEditorCamera* Scene::s_editorCamera = nullptr;
SceneRenderer* Scene::renderer = nullptr;
unordered_map<string, Scene*> Scene::s_instances;

Scene::Scene(string name) : sceneName(name)
{
	// Create light pos and col arrays for sending to shaders.
	m_pointLightPositions = new vec4[MAX_POINTLIGHTS];
	m_pointLightColours = new vec4[MAX_POINTLIGHTS];
}
Scene::~Scene()
{
	for (auto o : objects) delete o;

	objects.clear();
}

void Scene::Init()
{
	/*s_instance = new Scene();
	s_instances.emplace("default", s_instance);*/
}

Scene* Scene::NewScene(string name)
{
	Scene* scene = new Scene(name);
	s_instances.emplace(name, scene);
	return scene;
}

void Scene::CreateSceneEditorCamera()
{
	s_editorCamera = new SceneEditorCamera();
}

void Scene::Update(float deltaTime)
{
	// Update all Objects
	for (auto o : objects)
		o->Update(deltaTime);
}

void Scene::UpdatePointLightData()
{
	// all renderers check the closest lights to them
	for (auto& rendererComponent : m_rendererComponents)
		rendererComponent->UpdateClosestLights();

	// update the data sent to the gpu about each scene light
	for (int i = 0; i < m_pointLightComponents.size(); i++)
	{
		m_pointLightColours[i] = glm::vec4(m_pointLightComponents[i]->colour * m_pointLightComponents[i]->intensity,0);
		m_pointLightPositions[i] = glm::vec4(m_pointLightComponents[i]->GetComponentParentObject()->GetWorldSpacePosition(), 0);
	}
}

void Scene::Render()
{
	if (requestedObjectSelection)
	{
		requestedSelectionPosition = Input::GetMousePosPixel();
		renderer->RenderSceneObjectPick(this, cameraCurrent);
	}
	// renderer->RenderSceneShadowMaps(this, cameraCurrent); // cameraCurrent isnt used in this context but we might come back to it.
	renderer->RenderScene(this, cameraCurrent);

	if(drawGizmos)
		renderer->RenderSceneGizmos(this, cameraCurrent);

	renderer->RenderLines(cameraCurrent);
	renderer->DrawBackBuffer();
}

void Scene::RenderBakedShadowMaps()
{
	renderer->currentPassIsStatic = true;
	renderer->currentPassIsSplit = true;
	renderer->RenderSceneShadowCubeMaps(this);
}

void Scene::DrawGUI()
{	
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	int lightCount = m_pointLightComponents.size();
	ImGui::InputInt("Lights", &lightCount);
	int renderComponentCount = m_rendererComponents.size();
	ImGui::InputInt("render component count", &renderComponentCount);
	if (ImGui::Button("Save"))
		SaveJSON();
	ImGui::SameLine();
	if (ImGui::Button("Load"))
		ImGui::OpenPopup("popup_load_scene");

	// Draw scene file list if requested
	if (ImGui::BeginPopup("popup_load_scene"))
	{
		ImGui::SameLine();
		ImGui::SeparatorText("Scene Name");
		for (auto d : fs::recursive_directory_iterator("engine/scene"))
		{
			if (d.path().has_extension() && d.path().extension() == ".scene")
			{
				string foundSceneName = d.path().filename().string();
				string foundScenePath = d.path().relative_path().string();
				if (ImGui::Selectable(foundScenePath.c_str()))
				{
					sceneFilename = foundSceneName;
					LoadJSON(); // TODO - sanitize these paths properly 
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
		SetCameraIndex(cameraIndex);

	ImGui::Checkbox("Draw Gizmos", &drawGizmos);

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
	}

	if (ImGui::Button("New Object"))
		Scene::CreateObject();

	drawn3DGizmo = false;
	for (int i = 0; i < objects.size(); i++)
	{
		ImGui::PushID(i);
		objects[i]->DrawGUI();
		ImGui::PopID();
	}

	ImGui::End();

	//if (Scene::GetSelectedObject() > 0)
	//{
	//	// Draw Guizmo - very simple implementation - TODO have a 'selected object' context and mousewheel scroll through translate, rotate, scale options - rotate will need to be reworked.
	//	ImGuizmo::SetRect(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
	//	mat4 view, projection;
	//	view = Camera::s_instance->GetView();
	//	projection = Camera::s_instance->GetProjection();
	//	if (ImGuizmo::Manipulate((float*)&view, (float*)&projection, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, (float*)&s_instance->selectedObject->localTransform))
	//		s_instance->selectedObject->dirtyTransform = true;
	//	Scene::s_instance->drawn3DGizmo = true;
	//}
}


void Scene::DrawCameraGUI()
{
	s_editorCamera->DrawGUI();
}

// This will destroy all objects (and their children) marked for deletion.
void Scene::CleanUp()
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->markedForDeletion)
		{
			delete objects[i];
			objects.erase(objects.begin() + i);
			i--;
		}
		else
		{
			objects[i]->CleanUpComponents();
			objects[i]->CleanUpChildren();
		}
	}
}

// Creates and object and adds it to the parent or scene hierarchy. If you want an object but dont want it added to the heirarchy, then call new Object.
Object* Scene::CreateObject(Object* parent)
{
	Object* o = new Object(s_instance->objectCount); // NOTE This was objectCount++ in order to increment all object numbers, but honestly we just dont need IDs for everything.. yet?
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
Object* Scene::DuplicateObject(Object* object, Object* newParent)
{
	// Copy object properties
	Object* o = CreateObject(newParent);
	o->objectName = object->objectName;
	o->SetLocalPosition(object->localPosition);
	o->SetLocalRotation(object->localRotation);
	o->SetLocalScale(object->localScale);

	// Copy object components and children
	for (auto& component : object->components)
		o->components.push_back(component->Clone(o));

	for (auto& child : object->children)
		Object* c = DuplicateObject(child, o);

	// Refresh
	o->RefreshComponents();
	return o;
}

void Scene::SetClearColour()
{
	glClearColor(s_instance->clearColour.r, s_instance->clearColour.g, s_instance->clearColour.b, 1);
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
	return (int)s_instance->m_pointLightComponents.size();
}

void Scene::AddPointLight(ComponentLightPoint* light)
{
	s_instance->m_pointLightComponents.push_back(light);
	renderer->SetStaticShadowMapsDirty();
}

void Scene::RemovePointLight(ComponentLightPoint* light)
{
	for (int i = 0; i < Scene::s_instance->m_pointLightComponents.size(); i++)
	{
		if (s_instance->m_pointLightComponents[i] == light)
		{
			s_instance->m_pointLightComponents.erase(s_instance->m_pointLightComponents.begin() + i);
			break;
		}
	}
}

void Scene::AddRendererComponent(ComponentRenderer* rendererComponent)
{
	s_instance->m_rendererComponents.push_back(rendererComponent);
}

void Scene::RemoveRendererComponent(ComponentRenderer* rendererComponent)
{
	for (int i = 0; i < Scene::s_instance->m_rendererComponents.size(); i++)
	{
		if (s_instance->m_rendererComponents[i] == rendererComponent)
		{
			s_instance->m_rendererComponents.erase(s_instance->m_rendererComponents.begin() + i);
			break;
		}
	}
}

ComponentCamera* Scene::GetCameraByIndex(int index)
{
	if (index == -1)
		return s_editorCamera->camera;
	else return s_instance->componentCameras[index];
}

ComponentCamera* Scene::GetCurrentCamera()
{
	if (s_instance->cameraIndex == -1) return s_editorCamera->camera;
	else return s_instance->componentCameras[s_instance->cameraIndex];
}

// Index 0 will always be the editor camera.
// Indicies above 1 will be in world cameras and we look at the componentCameras array for the scene and -1 index in to it at render time.
void Scene::SetCameraIndex(int index)
{
	// Check we're not out of bounds
	if (index >= s_instance->componentCameras.size() || index < -1) index = -1;
	s_instance->cameraIndex = index;


	if (index == -1) s_instance->cameraCurrent = s_editorCamera->camera;
	else s_instance->cameraCurrent = s_instance->componentCameras[index];

	renderer->SetCullingCamera(index);
	AudioManager::SetAudioListener(s_instance->cameraCurrent->GetAudioListener());
}

void Scene::SetCameraByName(string name)
{
	if (name == "Editor Camera") SetCameraIndex(-1);
	else
	{
		for (int i = 0; i < s_instance->componentCameras.size(); i++)
		{
			if (s_instance->componentCameras[i]->GetComponentParentObject()->objectName == name)
			{
				SetCameraIndex(i);
				return;
			}
		}
	}
	LogUtils::Log("Error: Unable to find camera: " + name);
}

void Scene::SetSelectedObject(unsigned int selected)
{
	std::cout << selected << std::endl;
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

Object* Scene::FindObjectWithName(string objectName)
{
	for (auto o : s_instance->objects)
		if (o->objectName == objectName) return o;

	for (auto o : s_instance->objects)
		o->FindObjectWithName(objectName);


	return nullptr;
}

void Scene::SetAllObjectsStatic()
{
	for (auto& o : objects) o->Update(0.0f); // this fixes all dirty transforms.
	for (auto& o : objects) o->SetStatic(true);
}

void Scene::SaveJSON()
{
	// Create the JSON structures
	ordered_json output;
	ordered_json scene_lighting;
	ordered_json scene_lighting_pointLights;
	ordered_json objectsJSON;

	// meta data
	output["type"] = "scene";
	output["version"] = 1;

	// add lighting
	scene_lighting["clearColour"] = clearColour;
	scene_lighting["ambientColour"] = m_ambientColour;
	scene_lighting["sunColour"] = m_sunColour;
	scene_lighting["sunDirection"] = m_sunDirection;

	output["lighting"] = scene_lighting;

	// Populate objects
	for (int i = 0; i < objects.size(); i++)
		objectsJSON.push_back(*objects[i]);

	output["objects"] = objectsJSON;

	// write to disk
	WriteJSONToDisk(sceneFilename, output);
}

void Scene::LoadJSON(string sceneName)
{
	sceneFilename = sceneName;
	LoadJSON();
}

void Scene::LoadJSON()
{
	// Load the JSON object
	ordered_json input = ReadJSONFromDisk(sceneFilename);
	
	// check version lol
	// load the lighitng data

	ordered_json lighting = input.at("lighting");
	SetClearColour((vec3)lighting["clearColour"]);
	m_ambientColour = lighting.at("ambientColour");
	m_sunColour = lighting.at("sunColour");
	m_sunDirection = lighting.at("sunDirection");

	// clear current objects
	for (auto object = objects.begin(); object != objects.end(); object++)
	{
		auto obj = *object;
		delete obj;
	}
	objects.clear();
	gizmos.clear();

	// read in objects - oooohhh boy here we go.
	ordered_json objectsJSON = input["objects"];

	if (objectsJSON.is_null() == false)
	{
		for (auto it = objectsJSON.begin(); it != objectsJSON.end(); it++)
		{
			auto o = Scene::CreateObject();
			o->LoadFromJSON(it.value());
		}
	}
}

mat4 Scene::GetLightSpaceMatrix()
{
	return s_instance->lightSpaceMatrix;
}