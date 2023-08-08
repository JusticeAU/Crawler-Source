#include "DungeonArtTester.h"
#include "Dungeon.h"
#include "Scene.h"
#include "Window.h"
#include "ModelManager.h"
#include "Model.h"
#include "Animation.h"
#include "ComponentFactory.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
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

	model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
	renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);

	// Load existing model paths
	string folder = "crawler/model/";
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".object")
			models.push_back(d.path().generic_string());
	}
}

void Crawl::ArtTester::Update(float delta)
{
	if (Window::GetTime() > timeLastRefreshed + 1.0f)
	{
		UpdateStagingFolders();
		timeLastRefreshed = Window::GetTime();
	}
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
	if (MaterialManager::MaterialSelected)
	{
		Material* selected = MaterialManager::MaterialSelected;
		ImGui::SetNextWindowPos({ 550, 10 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize({ 600, 400 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Material");
		
		bool canEditName = true;
		if (selected->name.substr(0,7) == "crawler")
			canEditName = false;

		if (!canEditName)
			ImGui::BeginDisabled();

		string newName = selected->name;
		if (ImGui::InputText("Name", &newName, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Material Manager change name
			MaterialManager::RemoveMaterial(selected->name);
			MaterialManager::PushMaterial(newName, selected);
			// change name
			selected->name = newName;
		}

		if (!canEditName)
			ImGui::EndDisabled();
		selected->DrawGUI();
		ImGui::End();
	}

	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ 500, 600 });
	ImGui::Begin("Crawl Art Test",0, ImGuiWindowFlags_NoMove);
	if(ImGui::BeginCombo("Camera", Scene::GetCameraIndex() == 0 ? "Free" : "Player"))
	{
		if (ImGui::Selectable("Free"))
			Scene::SetCameraIndex(0);
		if (ImGui::Selectable("Player"))
			Scene::SetCameraIndex(1);

		ImGui::EndCombo();
	}

	if (Scene::GetCameraIndex() == 1)
	{
		if (ImGui::InputInt("Player Distance", &playerViewDistance, 1))
		{
			if(playerViewDistance > MAX_VIEW_DISTANCE) playerViewDistance = MAX_VIEW_DISTANCE;
			if (playerViewDistance < 1) playerViewDistance = 1;
			Scene::s_instance->objects[2]->SetLocalPosition({ 0, -playerViewDistance * DUNGEON_GRID_SCALE, 0 });
		}
	}

	DrawModelSelector();

	ImGui::Text("");
	ImGui::Text("Rendering");
	ImGui::Indent();
	ImGui::Text("Transform (Preview Only)");
	if (ImGui::DragFloat3("Position", &Scene::s_instance->objects[1]->localPosition[0],0.1f, -5, 5))
		Scene::s_instance->objects[1]->dirtyTransform = true;

	if (ImGui::SliderFloat3("Rotation", &Scene::s_instance->objects[1]->localRotation[0], -180, 180))
		Scene::s_instance->objects[1]->dirtyTransform = true;

	if (ImGui::DragFloat3("Scale", &Scene::s_instance->objects[1]->localScale[0],0.1f, -5, 5))
		Scene::s_instance->objects[1]->dirtyTransform = true;

	renderer->DrawGUI();
	
	if (animator)
	{
		ImGui::Unindent();
		ImGui::Text("");
		ImGui::Text("Animation");
		ImGui::Indent();
		animator->DrawGUI();
	}
	ImGui::Unindent();

	ImGui::End();

	DrawGUIStaging();
}

void Crawl::ArtTester::DrawGUIStaging()
{
	ImGui::SetNextWindowPos({ 800,18 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 500, 600 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Staging");
	
	// configure
	if (ImGui::InputText("Name", &stagedName))
		UpdateStagingFolders();

	
	if (ImGui::BeginCombo("Asset Type", stagedType.c_str()))
	{
		for (int i = 0; i < 6; i++)
		{
			const bool is_selected = (stagedType == types[i]);
			if (ImGui::Selectable(types[i].c_str(), is_selected))
			{
				type = i;
				stagedType = types[i];
				UpdateStagingFolders();
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// preview
	ImGui::Text("Output Preview");
	ImGui::BeginDisabled();
	ImGui::Text("Model");
		ImGui::Indent();
		for (string& str : stagingModels)
			ImGui::Selectable(str.c_str());	
		if (stagingModels.size() > 0)
		{
			string objectName = modelPath + stagedName + ".object";
			ImGui::Selectable(objectName.c_str());
		}
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
	ImGui::EndDisabled();

	// Save
	ImGui::Text("");
	if (ImGui::Button("Add To Game Data"))
		ExportStaging(false);

	ImGui::End();
}

void Crawl::ArtTester::DrawModelSelector()
{
	if (ImGui::BeginCombo("Load Existing Model", "Select"))
	{
		for (auto& path : models)
		{
			if (ImGui::Selectable(path.c_str(), false))
			{
				model->markedForDeletion = true;
				renderer->markedForDeletion = true;
				if(animator)
					animator->markedForDeletion = true;
				Object* o = model->GetComponentParentObject();
				o->Update(0);
				Scene::s_instance->objects[1]->LoadFromJSON(ReadJSONFromDisk(path));
				Scene::s_instance->objects[1]->dirtyTransform = true;

				RefreshComponentReferences();
				if(animator) // If there is an animator, force it to update so that there's a 'current' animation created.
					animator->Update(0);
			}
		}
		ImGui::EndCombo();
	}
}

void Crawl::ArtTester::RefreshComponentReferences()
{
	model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
	renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
	animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
}

void Crawl::ArtTester::ExportStaging(bool preview)
{
	// Clear the staging data.
	stagingModels.clear();
	stagingMaterials.clear();
	stagingTextures.clear();


	// Create Object JSON
	Object* object = Scene::s_instance->objects[1];
	ordered_json objectJSON = *object;
	objectJSON["name"] = stagedName;

	// Reset the transform for this export
	objectJSON["transform"]["position"]["x"] = 0.0f;
	objectJSON["transform"]["position"]["y"] = 0.0f;
	objectJSON["transform"]["position"]["z"] = 0.0f;

	objectJSON["transform"]["rotation"]["x"] = 0.0f;
	objectJSON["transform"]["rotation"]["y"] = 0.0f;
	objectJSON["transform"]["rotation"]["z"] = 0.0f;

	objectJSON["transform"]["scale"]["x"] = 1.0f;
	objectJSON["transform"]["scale"]["y"] = 1.0f;
	objectJSON["transform"]["scale"]["z"] = 1.0f;

	// copy model if needed
	string modelName = objectJSON["components"][0]["model"];
	if (modelName.substr(0, 1) != "_" && modelName.substr(0, 7) != "crawler" && modelName.substr(0, 6) != "engine")
	{
		stagingModels.push_back(modelPath + stagedName + ".fbx");
		if(!preview)
		{
			fs::copy_file(modelName, modelPath + stagedName + ".fbx", fs::copy_options::overwrite_existing);
			objectJSON["components"][0]["model"] = modelPath + stagedName + ".fbx";
		}
	}

	// copy materials and textures
	for (int i = 0; i < objectJSON["components"][1]["materials"].size(); i++)
	{
		string materialName = objectJSON["components"][1]["materials"][i];
		if (materialName.substr(0, 7) != "crawler" && materialName.substr(0, 6) != "engine" && materialName.substr(0, 4) != "NULL")
		{
			// Staged material, need to copy
			Material savedMaterial = *MaterialManager::GetMaterial(materialName);
			string newMaterialName = stagedName + "_" + savedMaterial.name;
			
			// Check we havent processed this material already from a previous submesh
			bool duplicate = false;
			for (auto& matStr : stagingMaterials)
			{
				if (matStr == materialPath + newMaterialName + ".material") duplicate = true;
			}
			if (duplicate)
				break;

			// updates names of each texture as required
			if (savedMaterial.albedoMap != nullptr && savedMaterial.albedoMapName.substr(0, 7) != "crawler" && savedMaterial.albedoMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_albedo.tga");
				if(!preview)
				{
					fs::copy_file(savedMaterial.albedoMapName, texturePath + stagedName + "_" + savedMaterial.name + "_albedo.tga", fs::copy_options::overwrite_existing);
					savedMaterial.albedoMapName = texturePath + newMaterialName + "_albedo.tga";
				}
			}

			if (savedMaterial.normalMap != nullptr && savedMaterial.normalMapName.substr(0, 7) != "crawler" && savedMaterial.normalMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_normal.tga");
				if (!preview)
				{
					fs::copy_file(savedMaterial.normalMapName, texturePath + stagedName + "_" + savedMaterial.name + "_normal.tga", fs::copy_options::overwrite_existing);
					savedMaterial.normalMapName = texturePath + newMaterialName + "_normal.tga";
				}
			}

			if (savedMaterial.metallicMap != nullptr && savedMaterial.metallicMapName.substr(0, 7) != "crawler" && savedMaterial.metallicMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_metallic.tga");
				if(!preview)
				{
					fs::copy_file(savedMaterial.metallicMapName, texturePath + stagedName + "_" + savedMaterial.name + "_metallic.tga", fs::copy_options::overwrite_existing);
					savedMaterial.metallicMapName = texturePath + newMaterialName + "_metallic.tga";
				}
			}

			if (savedMaterial.roughnessMap != nullptr && savedMaterial.roughnessMapName.substr(0, 7) != "crawler" && savedMaterial.roughnessMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_roughness.tga");
				if(!preview)
				{
					fs::copy_file(savedMaterial.roughnessMapName, texturePath + stagedName + "_" + savedMaterial.name + "_roughness.tga", fs::copy_options::overwrite_existing);
					savedMaterial.roughnessMapName = texturePath + newMaterialName + "_roughness.tga";
				}
			}

			if (savedMaterial.aoMap != nullptr && savedMaterial.aoMapName.substr(0, 7) != "crawler" && savedMaterial.aoMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_ao.tga");
				if (!preview)
				{
					fs::copy_file(savedMaterial.aoMapName, texturePath + stagedName + "_" + savedMaterial.name + "_ao.tga", fs::copy_options::overwrite_existing);
					savedMaterial.aoMapName = texturePath + newMaterialName + "_ao.tga";
				}
			}

			if (savedMaterial.emissiveMap != nullptr && savedMaterial.emissiveMapName.substr(0, 7) != "crawler" && savedMaterial.emissiveMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_emissive.tga");
				if (!preview)
				{
					fs::copy_file(savedMaterial.emissiveMapName, texturePath + stagedName + "_" + savedMaterial.name + "_emissive.tga", fs::copy_options::overwrite_existing);
					savedMaterial.emissiveMapName = texturePath + newMaterialName + "_emissive.tga";
				}
			}

			stagingMaterials.push_back(materialPath + newMaterialName + ".material");

			if(!preview)
			{
				// save the new material to disk
				savedMaterial.filePath = materialPath + newMaterialName + ".material";
				savedMaterial.SaveToFile();

				// update reference to the  material in the JSON
				objectJSON["components"][1]["materials"][i] = materialPath + newMaterialName + ".material";
			}
		}
	}

	if (preview)
		return;

	if(stagingModels.size() > 0)
		WriteJSONToDisk(modelPath + stagedName + ".object", objectJSON);
}

void Crawl::ArtTester::UpdateStagingFolders()
{
	//stagingFolder = "staging/model/";						// staging/model/monster_name/
	//stagingPath = "staging/model/" + typesFolders[type];	// staging/model/monster_name/monster_
	
	modelPath = "crawler/model/" + typesFolders[type];		// crawler/model/monster_name/monster_
	materialPath = "crawler/material/" + typesFolders[type];		// crawler/model/monster_name/monster_
	texturePath = "crawler/texture/" + typesFolders[type];		// crawler/model/monster_name/monster_
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
			ModelManager::s_instance->LoadFromFile(filepath.c_str(), Crawl::CRAWLER_TRANSFORM);
			model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
			//rendererSkinned = (ComponentSkinnedRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_SkinnedRenderer);
			animator = (ComponentAnimator*)Scene::s_instance->objects[1]->GetComponent(Component_Animator);
			model->model = ModelManager::GetModel(filepath);
			model->modelName = filepath;
			if (model->model->animations.size() > 0) // has animations, should attach an animator
			{
				// Configure animated stuff
				if (animator == nullptr)
				{
					animator = (ComponentAnimator*)ComponentFactory::NewComponent(Scene::s_instance->objects[1], Component_Animator);
					Scene::s_instance->objects[1]->components.push_back(animator);
				}
				animator->model = model->model;
				animator->StartAnimation(model->model->animations[0]->name);
				animator->OnParentChange();
				renderer->model = model->model;
				renderer->OnParentChange();
				for (int i = 0; i < renderer->materialArray.size(); i++)
				{
					renderer->materialArray[i] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
				}
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
			int meshNamePos = filename.find_last_of('\\', mapNamePos - 1);
			int fileNamePos = filename.find_last_of('\\', meshNamePos - 1);
			string mapName = filename.substr(mapNamePos + 1, extensionPos - mapNamePos - 1);
			string meshName = filename.substr(meshNamePos + 1, mapNamePos - meshNamePos - 1);


			// check if there is a material existing, otherwise create one
			Material* currentMaterial = nullptr;
			currentMaterial = MaterialManager::GetMaterial(meshName);
			if (currentMaterial == nullptr)
			{
				currentMaterial = new Material();
				currentMaterial->name = meshName;
				currentMaterial->shader = ShaderManager::GetShaderProgram("engine/shader/PBR");
				currentMaterial->shaderName = "engine/shader/PBR";
				currentMaterial->shaderSkinned = ShaderManager::GetShaderProgram("engine/shader/PBRSkinned");
				currentMaterial->shaderSkinnedName = "engine/shader/PBRSkinned";
				currentMaterial->isPBR = true;
				MaterialManager::PushMaterial(meshName, currentMaterial);
			}

			// load texture
			TextureManager::s_instance->LoadFromFile(paths[i]);
			// assign in to the map
			if (mapName == "albedo")
			{
				currentMaterial->albedoMapName = paths[i];
				currentMaterial->albedoMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "normal")
			{
				currentMaterial->normalMapName = paths[i];
				currentMaterial->normalMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "metallic")
			{
				currentMaterial->metallicMapName = paths[i];
				currentMaterial->metallicMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "roughness")
			{
				currentMaterial->roughnessMapName = paths[i];
				currentMaterial->roughnessMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "ao")
			{
				currentMaterial->aoMapName = paths[i];
				currentMaterial->aoMap = TextureManager::GetTexture(paths[i]);
			}
			else if (mapName == "emissive")
			{
				currentMaterial->emissiveMapName = paths[i];
				currentMaterial->emissiveMap = TextureManager::GetTexture(paths[i]);
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

	if (extension == "FBX" || extension == "fbx")
		return FileType::Model;
	else if (extension == "tga")
		return FileType::Texture;

	return FileType::Other;
}

Crawl::ArtTester* Crawl::ArtTester::s_instance = nullptr;