#include "Scene.h"

Scene::Scene()
{
	clearColour = { 0.25f, 0.25, 0.25 };
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
		objects.push_back(new Object());

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
		if (objects[i]->markedForDeletion)
		{
			objects.erase(objects.begin() + i);
			i--;
		}
	}
}
