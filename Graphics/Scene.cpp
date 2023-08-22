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
	m_pointLightPositions = new vec3[MAX_LIGHTS];
	m_pointLightColours = new vec3[MAX_LIGHTS];
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

	// Update point light arrays - right now we send this in as ivec and vec4s - should turn this in to a more dynamic uniform buffer.
	for (int i = 0; i < m_pointLights.size(); i++)
	{
		m_pointLightColours[i] = m_pointLights[i].colour * m_pointLights[i].intensity;
		m_pointLightPositions[i] = m_pointLights[i].position;
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
	renderer->DrawBackBuffer();
}

void Scene::DrawGUI()
{	
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Scene", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
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

		unsigned int lightsAtStartOfFrame = m_pointLights.size();
		if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::BeginDisabled();
		if (ImGui::Button("New Point Light"))
			m_pointLights.push_back(Light());
		if (lightsAtStartOfFrame == MAX_LIGHTS) ImGui::EndDisabled();

		// Draw all point lights
		for (int i = 0; i < m_pointLights.size(); i++)
		{
			ImGui::PushID(i);
			float pointCol[3] = { m_pointLights[i].colour.r, m_pointLights[i].colour.g, m_pointLights[i].colour.b, };
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
		objects[i]->CleanUpComponents();
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
	Object* o = new Object(s_instance->objectCount++); // NOTE This was objectCount++ in order to increment all object numbers, but honestly we just dont need IDs for everything.. yet?
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
	return (int)s_instance->m_pointLights.size();
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

	// Point lights
	int numPointLights = (int)m_pointLights.size();
	for (int i = 0; i < numPointLights; i++)
		scene_lighting_pointLights.push_back(m_pointLights[i]);

	scene_lighting["pointLights"] = scene_lighting_pointLights;
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

	// point lights
	ordered_json pointLights = lighting.at("pointLights");
	m_pointLights.clear();
	if (pointLights.is_null() == false)
	{
		//m_pointLights.resize(pointLights.size());
		for (auto it = pointLights.begin(); it != pointLights.end(); it++)
		{
			Light light = it.value();
			m_pointLights.push_back(light);
		}
	}

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