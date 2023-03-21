#include "Scene.h"

Scene::Scene()
{
	clearColour = { 0.25f, 0.25, 0.25 };
	s_instance = this;
}

Scene::~Scene()
{
	for (auto o : objects)
		o->DeleteAllChildren();

	objects.clear();
}

void Scene::Update(float deltaTime)
{
	// Do stuff
	for (auto o : objects)
		o->Update(deltaTime);
}

void Scene::DrawObjects()
{
	for (auto o : objects)
		o->Draw();
}

void Scene::DrawGUI()
{
	ImGui::Begin("Scene");
	
	if (ImGui::SliderFloat3("Clear Colour", &clearColour[0], 0, 1,"%.2f", ImGuiSliderFlags_AlwaysClamp));
		glClearColor(clearColour.x, clearColour.y, clearColour.z, 1);

	if (ImGui::Button("New Object"))
		Scene::CreateObject();

	for (auto o : objects)
	{
		o->DrawGUI();
	}
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

Scene* Scene::s_instance = nullptr;