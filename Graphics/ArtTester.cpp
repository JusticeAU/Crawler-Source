#include "ArtTester.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "Model.h"
#include "Animation.h"
#include "ComponentFactory.h"
#include "MaterialManager.h"
#include "LogUtils.h"
#include <string>

using std::string;

Crawl::ArtTester::ArtTester()
{
	s_instance = this;
}

void Crawl::ArtTester::Activate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), &ModelDropCallback);
	Scene::ChangeScene("CrawlArtTest");
	Scene::SetCameraIndex(1);
	Refresh();

}

void Crawl::ArtTester::Deactivate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), NULL);
}

void Crawl::ArtTester::DrawGUI()
{
	MaterialManager::DrawGUI();
	
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

	ImGui::Text("Rendering");
	if (renderer)
		renderer->DrawGUI();
	else if (rendererSkinned)
	{
		rendererSkinned->DrawGUI();
		ImGui::Text("Animations");
		animator->DrawGUI();
	}

	ImGui::End();

	DrawGUIStaging();
}

void Crawl::ArtTester::DrawGUIStaging()
{
	ImGui::Begin("Staging (Mock up, not functional)");
	
	// configure
	if (ImGui::BeginCombo("Asset Type", stagedType.c_str()))
	{
		for (int i = 0; i < 3; i++)
		{
			const bool is_selected = (stagedType == types[i]);
			if (ImGui::Selectable(types[i].c_str(), is_selected))
				stagedType = types[i];

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// preview
	ImGui::Text("Output");
	ImGui::BeginDisabled();
	ImGui::Text("Model");
	ImGui::Indent();
	ImGui::Selectable("assets/monsters/red_guy/red_guy.fbx");
	ImGui::Unindent();
	ImGui::Text("Textures");
	ImGui::Indent();
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body_ao.png");
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body_albedo.png");
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body_metallic.png");
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body_normal.png");
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body_roughness.png");
	ImGui::Unindent();
	ImGui::Text("Materials");
	ImGui::Indent();
	ImGui::Selectable("assets/monsters/red_guy/red_guy_body.material");
	ImGui::Unindent();
	ImGui::Text("Object");
	ImGui::Indent();
	ImGui::Selectable("assets/monsters/red_guy/red_guy.object");
	ImGui::Unindent();

	ImGui::EndDisabled();

	// Save
	ImGui::Button("Export");
	ImGui::End();
}

void Crawl::ArtTester::Refresh()
{
	s_instance->renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
	if (s_instance->renderer->markedForDeletion)
		s_instance->renderer == nullptr;
	s_instance->rendererSkinned = (ComponentSkinnedRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_SkinnedRenderer);
	if (s_instance->rendererSkinned->markedForDeletion)
		s_instance->rendererSkinned == nullptr;
	s_instance->animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
	if (s_instance->animator->markedForDeletion)
		s_instance->animator == nullptr;

}

void Crawl::ModelDropCallback(GLFWwindow* window, int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		string filepath = paths[i];
		string extension = filepath.substr(filepath.length() - 4, 4);
		if (extension == ".fbx" || extension == ".FBX")
		{
			LogUtils::Log("Dropped a FBX file");
			ModelManager::s_instance->LoadFromFile(filepath.c_str());
			ComponentModel* model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			ComponentRenderer* renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
			ComponentSkinnedRenderer* rendererSkinned = (ComponentSkinnedRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_SkinnedRenderer);
			ComponentAnimator* animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
			model->model = ModelManager::GetModel(filepath);
			if (model->model->animations.size() > 0) // has animations, should use skinned renderer
			{
				// Configure animated stuff
				ArtTester::s_instance->hasAnimations = true;
				if (rendererSkinned == nullptr)
				{
					rendererSkinned = (ComponentSkinnedRenderer*)ComponentFactory::NewComponent(Scene::s_instance->objects[1], Component_SkinnedRenderer);
					Scene::s_instance->objects[1]->components.push_back(rendererSkinned);
				}

				if (animator == nullptr)
				{
					animator = (ComponentAnimator*)ComponentFactory::NewComponent(Scene::s_instance->objects[1], Component_Animator);
					Scene::s_instance->objects[1]->components.push_back(animator);
				}
				animator->model = model->model;
				animator->StartAnimation(model->model->animations[0]->name, true);
				animator->OnParentChange();
				rendererSkinned->model = model->model;
				rendererSkinned->OnParentChange();
				for (int i = 0; i < rendererSkinned->materialArray.size(); i++)
				{
					rendererSkinned->materialArray[i] = MaterialManager::GetMaterial("models/materials/SkinnedLambertBlue.material");
				}

				// Clear off maybe unneeded stuff
				if (renderer != nullptr)
					renderer->markedForDeletion = true;

			}
			else
			{
				ArtTester::s_instance->hasAnimations = false;
				
				if (renderer == nullptr)
				{
					renderer = (ComponentRenderer*)ComponentFactory::NewComponent(Scene::s_instance->objects[1], Component_Renderer);
					Scene::s_instance->objects[1]->components.push_back(renderer);
				}

				// Clear off animator component
				if (animator != nullptr)
					animator->markedForDeletion = true;
				if (rendererSkinned != nullptr)
					rendererSkinned->markedForDeletion = true;


				renderer->model = model->model;
				renderer->OnParentChange();

				for (int i = 0; i < renderer->materialArray.size(); i++)
					renderer->materialArray[i] = MaterialManager::GetMaterial("models/materials/LambertBlue.material");
			}

			ArtTester::Refresh();
		}
		else
			LogUtils::Log("File dropped was not a FBX - get out of here!");
	}
}

Crawl::ArtTester* Crawl::ArtTester::s_instance = nullptr;