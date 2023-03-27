#include "Scene.h"
#include "FileUtils.h"
#include <fstream>

Scene::Scene()
{
	m_pointLightPositions = new vec3[MAX_LIGHTS];
	m_pointLightColours = new vec3[MAX_LIGHTS];

}

Scene::~Scene()
{
	for (auto o : objects)
		o->DeleteAllChildren();

	objects.clear();
}

void Scene::Init()
{
	s_instance = new Scene();
	Scene::SetClearColour({ 0.25f, 0.25, 0.25 });
}

void Scene::Update(float deltaTime)
{
	// Do stuff
	for (auto o : objects)
		o->Update(deltaTime);

	// Update point light arrays
	for (int i = 0; i < m_pointLights.size(); i++)
	{
		m_pointLightColours[i] = m_pointLights[i].colour * m_pointLights[i].intensity;
		m_pointLightPositions[i] = m_pointLights[i].position;
	}
}

void Scene::DrawObjects()
{
	for (auto o : objects)
		o->Draw();
}

void Scene::DrawGUI()
{
	ImGui::Begin("Scene");
	if (ImGui::Button("Save"))
		Save();
	ImGui::SameLine();
	if (ImGui::Button("Load"))
		Load();

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

		if (m_pointLights.size() == MAX_LIGHTS) ImGui::BeginDisabled();
		if (ImGui::Button("New Point Light"))
			m_pointLights.push_back(Light());
		if (m_pointLights.size() == MAX_LIGHTS) ImGui::EndDisabled();

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
			ImGui::PopID();
		}
	}

	if (ImGui::Button("New Object"))
		Scene::CreateObject();

	for (auto o : objects)
		o->DrawGUI();

	ImGui::End();
}

void Scene::CleanUp()
{
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->CleanUpChildren();
		if (objects[i]->markedForDeletion)
		{
			objects.erase(objects.begin() + i);
			i--;
		}
	}
}

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
	return s_instance->m_sunDirection;
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
	return s_instance->m_pointLights.size();
}

void Scene::Save()
{
	// Create/Open file for writing
	std::ofstream out("scenes/default.scene", std::ofstream::binary);

	// Serialize Scene Configuration
	
	// Clear Colour
	FileUtils::WriteVec(out, clearColour);
	FileUtils::WriteVec(out, m_ambientColour);
	
	// Scene Light Configuration
	FileUtils::WriteVec(out, m_sunColour);
	FileUtils::WriteVec(out, m_sunDirection);
	
	// Point Lights
	int numPointLights = m_pointLights.size();
	FileUtils::WriteInt(out, numPointLights);
	for (int i = 0; i < numPointLights; i++)
	{
		FileUtils::WriteVec(out, m_pointLights[i].position);
		FileUtils::WriteVec(out, m_pointLights[i].colour);
		FileUtils::WriteFloat(out, m_pointLights[i].intensity);
	}
	
	// Serialize Objects and Child Objects recursively
	int numObjects = objects.size();
	FileUtils::WriteInt(out, numObjects);
	for (int i = 0; i < numObjects; i++)
		objects[i]->Write(out);

	out.flush();
	out.close();
}

void Scene::Load()
{

	// Create/Open file for writing
	std::ifstream in("scenes/default.scene", std::ifstream::binary);

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
	objects.clear();

	//  Objects and Child Objects
	int numObjects;
	FileUtils::ReadInt(in, numObjects);
	for(int i = 0; i < numObjects; i++)
	{
		auto o = Scene::CreateObject();
		o->Read(in);
	}
	in.close();
}



Scene* Scene::s_instance = nullptr;