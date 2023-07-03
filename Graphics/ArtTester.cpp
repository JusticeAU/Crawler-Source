#include "ArtTester.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include <string>

using std::string;

Crawl::ArtTester::ArtTester()
{
}

void Crawl::ArtTester::Activate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), &ModelDropCallback);
	Scene::ChangeScene("CrawlArtTest");
	Scene::SetCameraIndex(1);
	renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
}

void Crawl::ArtTester::Deactivate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), NULL);
}

void Crawl::ArtTester::DrawGUI()
{
	ImGui::Begin("Crawl Art Test");
	if (ImGui::Button(scaleIndex == 0 ? "Scale (PASS)" : "Scale (FAIL)"))
	{
		scaleIndex += 1;
		if (scaleIndex >= QTY_SCALES) scaleIndex = 0;
		
		Scene::s_instance->objects[1]->SetLocalScale(scales[scaleIndex]);
	}
	ImGui::SameLine();
	ImGui::Text(std::to_string(scales[scaleIndex]).c_str());

	if (ImGui::Button(upAxisIndex == 0 ? "Up (PASS)" : "Up (FAIL)"))
	{
		upAxisIndex += 1;
		if (upAxisIndex >= QTY_UP_AXIS) upAxisIndex = 0;

		Scene::s_instance->objects[1]->SetLocalRotation({upAxis[upAxisIndex], 0, 0});
	}
	ImGui::SameLine();
	ImGui::Text(std::to_string(upAxis[upAxisIndex]).c_str());

	if (ImGui::Button(forwardAxisIndex == 0 ? "Forward (PASS)" : "Forward (FAIL)"))
	{
		forwardAxisIndex += 1;
		if (forwardAxisIndex >= QTY_FORWARD_AXIS) forwardAxisIndex = 0;

		Scene::s_instance->objects[1]->SetLocalRotationZ(forwardAxis[forwardAxisIndex]);
	}
	ImGui::SameLine();
	ImGui::Text(std::to_string(forwardAxis[forwardAxisIndex]).c_str());

	if (ImGui::InputInt("Player Distance", &playerViewDistance, 1))
	{
		if(playerViewDistance > MAX_VIEW_DISTANCE) playerViewDistance = MAX_VIEW_DISTANCE;
		if (playerViewDistance < 1) playerViewDistance = 1;
		Scene::s_instance->objects[2]->SetLocalPosition({ 0, -playerViewDistance * DUNGEON_GRID_SCALE, 0 });
	}


	ImGui::Text("Loaded Model");

	renderer->DrawGUI();

	ImGui::End();
}

void Crawl::ModelDropCallback(GLFWwindow* window, int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		std::cout << paths[i] << std::endl;
		string filepath = paths[i];
		string extension = filepath.substr(filepath.length() - 4, 4);
		if (extension == ".fbx" || extension == ".obj")
		{
			std::cout << "dropped an model" << std::endl;
			ModelManager::s_instance->LoadFromFile(filepath.c_str());
			ComponentModel* model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			model->model = ModelManager::GetModel(filepath);
			ComponentModel* renderer = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
			renderer->model = model->model;
			renderer->OnParentChange();
		}
	}
}
