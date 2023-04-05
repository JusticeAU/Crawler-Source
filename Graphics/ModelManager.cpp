#include "ModelManager.h"
#include "Graphics.h"
#include "LogUtils.h"
#include "Model.h"
#include "Mesh.h"
#include "MeshManager.h"
#include "Object.h"
#include "ai2glm.h"

#include "assimp/scene.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

#include <filesystem>

using std::vector;
namespace fs = std::filesystem;

void ModelManager::Init()
{
	if (!s_instance) s_instance = new ModelManager();
	else LogUtils::Log("Tried to Init ModelManager when it was already initialised");
}

Model* ModelManager::GetModel(string name)
{
    auto iterator = s_instance->resources.find(name);
    if (iterator == s_instance->resources.end())
        return nullptr;
    else
        return iterator->second;
}

void ModelManager::DrawGUI()
{
	ImGui::Begin("Model Manager");
	ImGui::BeginDisabled();
	int resourceCount = (int)s_instance->resources.size();
	ImGui::DragInt("Model Count", &resourceCount);
	for (auto m : s_instance->resources)
	{
		ImGui::Text(m.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

ModelManager::ModelManager()
{
	resources.emplace("_null", nullptr);
	LoadAllFiles();
}

void ModelManager::LoadFromFile(const char* filename)
{
	// create an instance so we can easily configure it.
	Assimp::Importer importer;

	// disable pivot preserving, not needed for our purposes and gummys up the node heirarchy with noise.
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace);

	// Create a new model to start pushing our data in to.
	Model* model = new Model();

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* inMesh = scene->mMeshes[i];
		if (inMesh->mNumBones > 0)
		{
			if (model->boneStructure == nullptr) model->boneStructure = new Model::BoneStructure();

		}
		string name = filename;
		string subname = name + "_" + scene->mMeshes[i]->mName.C_Str();
		Mesh* loadedMesh = MeshManager::LoadFromAiMesh(inMesh, model->boneStructure, subname.c_str());
		model->meshes.push_back(loadedMesh);
	}

	// Load nodes in to loadedMesh->childNodes.
	Object* rootNode = new Object(0, scene->mRootNode->mName.C_Str());
	model->childNodes = rootNode;
	rootNode->localTransform = mat4_cast(scene->mRootNode->mTransformation);
	MeshManager::CopyNodeHierarchy(scene, scene->mRootNode, rootNode, model->boneStructure);

	// Load Animation Data
	for (unsigned int i = 0; i < scene->mNumAnimations; i++) // for each animation
	{
		Model::Animation anim;
		anim.name = scene->mAnimations[i]->mName.C_Str();
		string log = "Processing Animation: " + anim.name;
		LogUtils::Log(log.c_str());
		anim.duration = scene->mAnimations[i]->mDuration;
		anim.ticksPerSecond = scene->mAnimations[i]->mTicksPerSecond;
		anim.numChannels = scene->mAnimations[i]->mNumChannels;

		for (int j = 0; j < anim.numChannels; j++) // for each channel in the animation
		{
			Model::Animation::AnimationChannel channel;
			channel.name = scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str();
			int keyCount = scene->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			for (int k = 0; k < keyCount; k++) // for each key in the channel in the animation
			{
				channel.keys[k].position = vec3_cast(scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue);
				channel.keys[k].rotation = quat_cast(scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue);
				channel.keys[k].scale = vec3_cast(scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue);
			}
			anim.channels.emplace(channel.name, channel);
		}

		model->animations.push_back(anim);
	}
	resources.emplace(filename, model);
}

void ModelManager::LoadAllFiles()
{
	LogUtils::Log("Loading models");
	for (auto d : fs::recursive_directory_iterator("models"))
	{
		if (d.path().extension() == ".obj" || d.path().extension() == ".fbx" || d.path().extension() == ".gltf")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}
	}
}

ModelManager* ModelManager::s_instance = nullptr;