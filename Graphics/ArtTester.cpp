#include "ArtTester.h"
#include "Window.h"
#include <string>
#include "ModelManager.h"
#include "Scene.h"
#include "ComponentModel.h"

using std::string;

Crawl::ArtTester::ArtTester()
{
}

void Crawl::ArtTester::Activate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), &ModelDropCallback);
}

void Crawl::ArtTester::Deactivate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), NULL);
}

void Crawl::ModelDropCallback(GLFWwindow* window, int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		std::cout << paths[i] << std::endl;
		string filepath = paths[i];
		string extension = filepath.substr(filepath.length() - 4, 4);
		if (extension == ".fbx")
		{
			std::cout << "dropped an fbx" << std::endl;
			ModelManager::s_instance->LoadFromFile(filepath.c_str());
			ComponentModel* model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			model->model = ModelManager::GetModel(filepath);
			ComponentModel* renderer = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
			renderer->model = model->model;
			renderer->OnParentChange();
		}
	}
}
