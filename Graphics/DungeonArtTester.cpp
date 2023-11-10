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
#include <string>
#include <filesystem>
#include "serialisation.h"

#include "LogUtils.h"
#include "StringUtils.h"

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
	Scene::SetCameraByName("Player Camera");
	Refresh();

}

void Crawl::ArtTester::Deactivate()
{
	glfwSetDropCallback(Window::GetWindow()->GetGLFWwindow(), NULL);
}

void Crawl::ArtTester::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ 500, 600 });
	ImGui::Begin("Crawl Art Test",0, ImGuiWindowFlags_NoMove);
	if(ImGui::BeginCombo("Camera", Scene::GetCameraIndex() == -1 ? "Free" : "Player"))
	{
		if (ImGui::Selectable("Free"))
			Scene::SetCameraByName();
		if (ImGui::Selectable("Player"))
			Scene::SetCameraByName("Player Camera");

		ImGui::EndCombo();
	}

	if (Scene::GetCameraIndex() == 0)
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
		Scene::s_instance->objects[1]->SetDirtyTransform();

	if (ImGui::SliderFloat3("Rotation", &Scene::s_instance->objects[1]->localRotation[0], -180, 180))
		Scene::s_instance->objects[1]->SetDirtyTransform();

	if (ImGui::DragFloat3("Scale", &Scene::s_instance->objects[1]->localScale[0],0.1f, -5, 5))
		Scene::s_instance->objects[1]->SetDirtyTransform();

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

	// Configure
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

	// Preview
	ImGui::Text("Output Preview");
	ImGui::BeginDisabled();
	ImGui::Text("Model");
		ImGui::Indent();
		for (string& str : stagingModels)
			ImGui::Selectable(str.c_str());	
		
		if (ShouldWriteObjectFile())
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
	{
		ExportStaging(false);
		ImGui::OpenPopup("Added Assets to Game Data");
	}


	if (ImGui::BeginPopupModal("Added Assets to Game Data"))
	{
		ImGui::Text("Added Staged Assets to Game Data - Check your Source Control Changes.");
		if (ImGui::Button("OK."))
			ImGui::CloseCurrentPopup();
	}

	ImGui::End();
}

void Crawl::ArtTester::DrawModelSelector()
{
	if (ImGui::BeginCombo("Load Existing Model", selectedModel == "" ? "Select Object" : selectedModel.c_str(), ImGuiComboFlags_HeightLargest))
	{
		for (auto& path : models)
		{
			bool selected = path == selectedModel;
			if (ImGui::Selectable(path.c_str(), selected))
			{
				selectedModel = path;
				model->markedForDeletion = true;
				renderer->markedForDeletion = true;
				if(animator)
					animator->markedForDeletion = true;
				Object* o = model->GetComponentParentObject();
				o->Update(0);
				Scene::s_instance->objects[1]->LoadFromJSON(ReadJSONFromDisk(path));
				Scene::s_instance->objects[1]->SetDirtyTransform();

				RefreshComponentReferences();
				if(animator) // If there is an animator, force it to update so that there's a 'current' animation created.
					animator->Update(0);

				SetStagingFromExistingAsset();
			}
			if (selected) ImGui::SetItemDefaultFocus();
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

// This function is dual purpose based on the 'preview' parameter.
// Because the logic is so similar it'd be a lot of redundant code to seperate updating the preview of the staging area vs actually exporting the staged area.
// Essentially, if preview is false, then an actual export is done. If it's not, then all the work is done and fields are populared, but no data is written to the filesystem.
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
			// prepare the material - we definately need to update our path to it,
			//  and if its a material we havent seen before in the loop, we need to copy it and the texture files too.
			Material savedMaterial = *MaterialManager::GetMaterial(materialName);
			string newMaterialName = stagedName + "_" + savedMaterial.name;

			// update reference to the material in the JSON - this must happen every iteration.
			if(!preview) objectJSON["components"][1]["materials"][i] = materialPath + newMaterialName + ".material";
			
			// Check we havent already processed this new material from a previosu iteration.
			bool duplicate = false;
			for (auto& matStr : stagingMaterials)
			{
				if (matStr == materialPath + newMaterialName + ".material") duplicate = true;
			}

			if (duplicate) // We already processed it, so none of the below needs to happen.
				continue;

			// Process the new material. We'll update all the filenames, references to them, and copy the textures in.
			// Update names of each texture as required
			if (savedMaterial.albedoMap != nullptr && savedMaterial.albedoMapName.substr(0, 7) != "crawler" && savedMaterial.albedoMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_albedo.tga");
				if(!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.albedoMapName, texturePath + stagedName + "_" + savedMaterial.name + "_albedo.tga");
					savedMaterial.albedoMapName = texturePath + newMaterialName + "_albedo.tga";
				}
			}

			if (savedMaterial.normalMap != nullptr && savedMaterial.normalMapName.substr(0, 7) != "crawler" && savedMaterial.normalMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_normal.tga");
				if (!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.normalMapName, texturePath + stagedName + "_" + savedMaterial.name + "_normal.tga");
					savedMaterial.normalMapName = texturePath + newMaterialName + "_normal.tga";
				}
			}

			if (savedMaterial.metallicMap != nullptr && savedMaterial.metallicMapName.substr(0, 7) != "crawler" && savedMaterial.metallicMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_metallic.tga");
				if(!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.metallicMapName, texturePath + stagedName + "_" + savedMaterial.name + "_metallic.tga");
					savedMaterial.metallicMapName = texturePath + newMaterialName + "_metallic.tga";
				}
			}

			if (savedMaterial.roughnessMap != nullptr && savedMaterial.roughnessMapName.substr(0, 7) != "crawler" && savedMaterial.roughnessMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_roughness.tga");
				if(!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.roughnessMapName, texturePath + stagedName + "_" + savedMaterial.name + "_roughness.tga");
					savedMaterial.roughnessMapName = texturePath + newMaterialName + "_roughness.tga";
				}
			}

			if (savedMaterial.aoMap != nullptr && savedMaterial.aoMapName.substr(0, 7) != "crawler" && savedMaterial.aoMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_ao.tga");
				if (!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.aoMapName, texturePath + stagedName + "_" + savedMaterial.name + "_ao.tga");
					savedMaterial.aoMapName = texturePath + newMaterialName + "_ao.tga";
				}
			}

			if (savedMaterial.emissiveMap != nullptr && savedMaterial.emissiveMapName.substr(0, 7) != "crawler" && savedMaterial.emissiveMapName.substr(0, 6) != "engine")
			{
				stagingTextures.push_back(texturePath + newMaterialName + "_emissive.tga");
				if (!preview)
				{
					Texture::RewriteTGAwithRLE(savedMaterial.emissiveMapName, texturePath + stagedName + "_" + savedMaterial.name + "_emissive.tga");
					savedMaterial.emissiveMapName = texturePath + newMaterialName + "_emissive.tga";
				}
			}

			stagingMaterials.push_back(materialPath + newMaterialName + ".material");

			if(!preview)
			{
				// save the new material to disk
				savedMaterial.filePath = materialPath + newMaterialName + ".material";
				savedMaterial.SaveToFile();
			}
		}
	}

	if (preview)
		return;

	if(ShouldWriteObjectFile())
		WriteJSONToDisk(modelPath + stagedName + ".object", objectJSON);
}

void Crawl::ArtTester::UpdateStagingFolders()
{
	modelPath = "crawler/model/" + typesFolders[type];			// crawler/model/monster_
	materialPath = "crawler/material/" + typesFolders[type];	// crawler/material/monster_
	texturePath = "crawler/texture/" + typesFolders[type];		// crawler/texture/monster_
	ExportStaging(true);
}

// This function is some simple logic to cehck if we should be creating a new .object file for an existing asset. It's pretty hacky but does the job.
bool Crawl::ArtTester::ShouldWriteObjectFile()
{
	return stagingModels.size() > 0 || renderer->modifiedMaterials && model->modelName != "crawler/model/arttest_plane/arttest_plane.fbx";
}

// This function is called when the user selects a model from the list of existing models. Its updates the staging name to reflect how this model was configured.
// It helps the user ensure the names are the same so their updated assets match and overwrite correctly.
void Crawl::ArtTester::SetStagingFromExistingAsset()
{
	// Get the index of the last slash, so that we can ignore subdirectories.
	int lastSlashIndex = selectedModel.find_last_of('/');
	string fullAssetName = selectedModel.substr(lastSlashIndex + 1, selectedModel.size() - lastSlashIndex);

	// Ge the index of the first underscore within so we can seperate the asset type from its name.
	int firstUnderscore = fullAssetName.find_first_of('_');
	string assetType = fullAssetName.substr(0, firstUnderscore);
	string assetName = fullAssetName.substr(firstUnderscore + 1, fullAssetName.size() - firstUnderscore - 7 - 1); // the -7 is ".object"

	// Do some pretty basic logic to reconfigure the staging area based on the name of type. If there is no match then its assumed to be an 'other' type.
	if (assetType == "monster")
	{
		type = 0;
		stagedType = types[0];
	}
	else if(assetType == "tile")
	{
		type = 1;
		stagedType = types[1];
	}
	else if(assetType == "interactable")
	{
		type = 2;
		stagedType = types[2];
	}
	else if(assetType == "door")
	{
		type = 3;
		stagedType = types[3];
	}
	else if(assetType == "decoration")
	{
		type = 4;
		stagedType = types[4];
	}
	else // other
	{
		type = 5;
		stagedType = types[5];
		assetName = fullAssetName.substr(0, fullAssetName.size() - 7);
	}

	stagedName = assetName;
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
			selectedModel = "";
			ModelManager::s_instance->LoadFromFile(filepath.c_str(), Crawl::CRAWLER_TRANSFORM);
			model = (ComponentModel*)Scene::s_instance->objects[1]->GetComponent(Component_Model);
			renderer = (ComponentRenderer*)Scene::s_instance->objects[1]->GetComponent(Component_Renderer);
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
				animator->isPlaying = true;
				animator->StartAnimation(model->model->animations[0]->name, true);
				animator->OnParentChange();
				renderer->model = model->model;
				renderer->OnParentChange();
				for (int i = 0; i < renderer->submeshMaterials.size(); i++)
				{
					renderer->submeshMaterials[i] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
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

				for (int i = 0; i < renderer->submeshMaterials.size(); i++)
					renderer->submeshMaterials[i] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
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
			TextureManager::s_instance->CreateTextureFromFile(paths[i]);
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