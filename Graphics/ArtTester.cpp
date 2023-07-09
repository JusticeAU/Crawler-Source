#include "ArtTester.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "Model.h"
#include "Animation.h"
#include "ComponentFactory.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "LogUtils.h"
#include <string>
#include <filesystem>
#include "serialisation.h"

using std::string;
namespace fs = std::filesystem;

Crawl::ArtTester::ArtTester()
{
	s_instance = this;
	UpdateStagingFolders();
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
	ImGui::Begin("Staging");
	
	// configure
	if (ImGui::InputText("Name", &stagedName))
		UpdateStagingFolders();

	ImGui::SameLine();
	if (ImGui::BeginCombo("Asset Type", stagedType.c_str()))
	{
		for (int i = 0; i < 4; i++)
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
		for (string& str : stagingModels)
			ImGui::Selectable(str.c_str());	
		ImGui::Unindent();
	ImGui::Text("Materials");
		ImGui::Indent();
		for (string& str : stagingMaterials)
			ImGui::Selectable(str.c_str());
		ImGui::Unindent();
	ImGui::Text("Textures");
		ImGui::Indent();
		for (string& str : stagingTextures)
			ImGui::Selectable(str.c_str());
		ImGui::Unindent();
	ImGui::Text("Object");
		ImGui::Indent();
		string objectName = assetPath + stagedName + ".object";
		ImGui::Selectable(objectName.c_str());
		ImGui::Unindent();
	ImGui::EndDisabled();

	// Save
	if (ImGui::Button("Export"))
		ExportStaging(false);

	ImGui::End();
}

void Crawl::ArtTester::ExportStaging(bool preview)
{
	// Create staging folder
	if (preview)
	{
		stagingModels.clear();
		stagingMaterials.clear();
		stagingTextures.clear();
	}
	else
		fs::create_directories(stagingFolder);

	// Create Object JSON
	Object* object = Scene::s_instance->objects[1];
	ordered_json objectJSON = *object;
	objectJSON["name"] = stagedName;

	// copy model if needed
	string modelName = objectJSON["components"][0]["model"];
	if (modelName.substr(0, 1) != "_" && modelName.substr(0, 7) != "crawler" && modelName.substr(0, 6) != "engine")
	{
		if (preview)
			stagingModels.push_back(assetPath + stagedName + ".fbx");
		else
		{
			fs::copy_file(modelName, stagingPath + stagedName + ".fbx", fs::copy_options::overwrite_existing);
			objectJSON["components"][0]["model"] = assetPath + stagedName + ".fbx";
		}
	}

	// copy materials and textures
	for (int i = 0; i < objectJSON["components"][1]["materials"].size(); i++)
	{
		string materialName = objectJSON["components"][1]["materials"][i];
		if (materialName.substr(0, 7) != "crawler" && materialName.substr(0, 6) != "engine")
		{

			// Staged material, need to copy
			Material savedMaterial = *MaterialManager::GetMaterial(materialName);
			string newMaterialName = stagedName + "_" + savedMaterial.name;

			// updates names of each texture as required
			if (savedMaterial.albedoMap != nullptr && savedMaterial.albedoMapName.substr(0, 7) != "crawler" && savedMaterial.albedoMapName.substr(0, 6) != "engine")
			{
				if (preview)
					stagingTextures.push_back(assetPath + newMaterialName + "_albedo.png");
				else
				{
					fs::copy_file(savedMaterial.albedoMapName, stagingPath + stagedName + "_" + savedMaterial.name + "_albedo.png", fs::copy_options::overwrite_existing);
					savedMaterial.albedoMapName = assetPath + newMaterialName + "_albedo.png";
				}
			}

			if (savedMaterial.normalMap != nullptr && savedMaterial.normalMapName.substr(0, 7) != "crawler" && savedMaterial.normalMapName.substr(0, 6) != "engine")
			{
				if (preview)
					stagingTextures.push_back(assetPath + newMaterialName + "_normal.png");
				else
				{
					fs::copy_file(savedMaterial.normalMapName, stagingPath + stagedName + "_" + savedMaterial.name + "_normal.png", fs::copy_options::overwrite_existing);
					savedMaterial.normalMapName = assetPath + newMaterialName + "_normal.png";
				}
			}

			if (savedMaterial.metallicMap != nullptr && savedMaterial.metallicMapName.substr(0, 7) != "crawler" && savedMaterial.metallicMapName.substr(0, 6) != "engine")
			{
				if (preview)
					stagingTextures.push_back(assetPath + newMaterialName + "_metallic.png");
				else
				{
					fs::copy_file(savedMaterial.metallicMapName, stagingPath + stagedName + "_" + savedMaterial.name + "_metallic.png", fs::copy_options::overwrite_existing);
					savedMaterial.metallicMapName = assetPath + newMaterialName + "_metallic.png";
				}
			}

			if (savedMaterial.roughnessMap != nullptr && savedMaterial.roughnessMapName.substr(0, 7) != "crawler" && savedMaterial.roughnessMapName.substr(0, 6) != "engine")
			{
				if (preview)
					stagingTextures.push_back(assetPath + newMaterialName + "_roughness.png");
				else
				{
					fs::copy_file(savedMaterial.roughnessMapName, stagingPath + stagedName + "_" + savedMaterial.name + "_roughness.png", fs::copy_options::overwrite_existing);
					savedMaterial.roughnessMapName = assetPath + newMaterialName + "_roughness.png";
				}
			}

			if (savedMaterial.aoMap != nullptr && savedMaterial.aoMapName.substr(0, 7) != "crawler" && savedMaterial.aoMapName.substr(0, 6) != "engine")
			{
				if (preview)
					stagingTextures.push_back(assetPath + newMaterialName + "_ao.png");
				else
				{
					fs::copy_file(savedMaterial.aoMapName, stagingPath + stagedName + "_" + savedMaterial.name + "_ao.png", fs::copy_options::overwrite_existing);
					savedMaterial.aoMapName = assetPath + newMaterialName + "_ao.png";
				}
			}

			if (preview)
				stagingMaterials.push_back(assetPath + newMaterialName + ".material");
			else
			{
				// save the new material to disk
				savedMaterial.filePath = stagingPath + newMaterialName + ".material";
				savedMaterial.SaveToFile();

				// update reference to the  material in the JSON
				objectJSON["components"][1]["materials"][i] = assetPath + newMaterialName + ".material";
			}
		}
	}

	if (preview)
		return;

	// Save object configuration
	WriteJSONToDisk(stagingPath + stagedName + ".object", objectJSON);

	// open folder to location
	string working = fs::current_path().string();
	string command = "explorer " + working + "\\staging";
	system(command.c_str());
}

void Crawl::ArtTester::UpdateStagingFolders()
{
	stagingFolder = "staging/model/" + typesFolders[type] + stagedName + "/";						// staging/model/monster_name/
	stagingPath = "staging/model/" + typesFolders[type] + stagedName + "/" + typesFolders[type];	// staging/model/monster_name/monster_
	assetPath = "crawler/model/" + typesFolders[type] + stagedName + "/" + typesFolders[type];		// crawler/model/monster_name/monster_
	ExportStaging(true);
}

void Crawl::ArtTester::ModelDropCallBack(int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		string filepath = paths[i];
		string extension = filepath.substr(filepath.length() - 4, 4);
		LogUtils::Log(paths[i]);

		FileType type = GetFileType(paths[i]);

		switch (type)
		{
		case FileType::Model:
		{
			LogUtils::Log("Dropped a Model file");
			ModelManager::s_instance->LoadFromFile(filepath.c_str());
			model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
			rendererSkinned = (ComponentSkinnedRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_SkinnedRenderer);
			animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
			model->model = ModelManager::GetModel(filepath);
			model->modelName = filepath;
			if (model->model->animations.size() > 0) // has animations, should use skinned renderer
			{
				// Configure animated stuff
				hasAnimations = true;
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
					rendererSkinned->materialArray[i] = MaterialManager::GetMaterial("engine/model/materials/SkinnedLambertBlue.material");
				}

				// Clear off maybe unneeded stuff
				if (renderer != nullptr)
					renderer->markedForDeletion = true;

			}
			else
			{
				hasAnimations = false;

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
					renderer->materialArray[i] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
			}

			Refresh();
			break;
		}
		case FileType::Texture:
		{
			// Decompose name | name, meshname, mapname
			string filename = paths[i];
			int extensionPos = filename.find_last_of('.');
			int mapNamePos = filename.find_last_of('_');
			int meshNamePos = filename.find_last_of('_', mapNamePos - 1);
			int fileNamePos = filename.find_last_of('\\', meshNamePos - 1);
			string mapName = filename.substr(mapNamePos + 1, extensionPos - mapNamePos - 1);
			string meshName = filename.substr(meshNamePos + 1, mapNamePos - meshNamePos - 1);
			string name = filename.substr(fileNamePos + 1, meshNamePos - fileNamePos - 1);

			// check if there is a material existing, otherwise create one
			Material* currentMaterial = nullptr;
			currentMaterial = MaterialManager::GetMaterial(meshName);
			if (currentMaterial == nullptr)
			{
				currentMaterial = new Material();
				currentMaterial->name = meshName;
				MaterialManager::PushMaterial(meshName, currentMaterial);
			}

			// load texture
			TextureManager::s_instance->LoadFromFile(paths[i]);
			// assign in to the map
			if (mapName == "albedo" || mapName == "colour" || mapName == "base")
			{
				currentMaterial->albedoMapName = paths[i];
				currentMaterial->albedoMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "normal" || mapName == "nmap")
			{
				currentMaterial->normalMapName = paths[i];
				currentMaterial->normalMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "metallic" || mapName == "metalness")
			{
				currentMaterial->metallicMapName = paths[i];
				currentMaterial->metallicMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "roughness" || mapName == "rough")
			{
				currentMaterial->roughnessMapName = paths[i];
				currentMaterial->roughnessMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "ao")
			{
				currentMaterial->aoMapName = paths[i];
				currentMaterial->aoMap = TextureManager::GetTexture(paths[i]);
			}

			break;
		}
		default:
		{
			break;
		}
		}
	}
	ExportStaging(true);
}

void Crawl::ArtTester::Refresh()
{
	s_instance->renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
	if (s_instance->renderer && s_instance->renderer->markedForDeletion)
		s_instance->renderer == nullptr;
	s_instance->rendererSkinned = (ComponentSkinnedRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_SkinnedRenderer);
	if (s_instance->rendererSkinned && s_instance->rendererSkinned->markedForDeletion)
		s_instance->rendererSkinned == nullptr;
	s_instance->animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
	if (s_instance->animator && s_instance->animator->markedForDeletion)
		s_instance->animator == nullptr;

}

void Crawl::ModelDropCallback(GLFWwindow* window, int count, const char** paths)
{
	ArtTester::s_instance->ModelDropCallBack(count, paths);
}

Crawl::FileType Crawl::ArtTester::GetFileType(string filename)
{
	int extensionPos = filename.find_last_of('.');
	string extension = filename.substr(extensionPos+1, filename.length() - extensionPos);

	if (extension == "FBX" || extension == "fbx" || extension == "obj")
		return FileType::Model;
	else if (extension == "jpg" || extension == "png")
		return FileType::Texture;

	return FileType::Other;
}

Crawl::ArtTester* Crawl::ArtTester::s_instance = nullptr;