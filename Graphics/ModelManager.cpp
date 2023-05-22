#include "ModelManager.h"
#include "Graphics.h"
#include "LogUtils.h"
#include "Model.h"
#include "Mesh.h"
#include "Animation.h"
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

Animation* ModelManager::GetAnimation(string name)
{
	auto iterator = s_instance->animations.find(name);
	if (iterator == s_instance->animations.end())
		return nullptr;
	else
		return iterator->second;
}

void ModelManager::DrawGUI()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Models", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::BeginDisabled();
	int resourceCount = (int)s_instance->resources.size();
	ImGui::DragInt("Model Count", &resourceCount);
	for (auto m : s_instance->resources)
	{
		ImGui::Text(m.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();

	ImGui::Begin("Animations", nullptr);
	ImGui::BeginDisabled();
	int animationCount = (int)s_instance->animations.size();
	ImGui::DragInt("Animation Count", &animationCount);
	for (auto a : s_instance->animations)
	{
		ImGui::Text(a.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

ModelManager::ModelManager()
{
	resources.emplace("_null", nullptr);
	
	Model* primitive = new Model();
	primitive->meshes.push_back(MeshManager::GetMesh("_cube"));
	resources.emplace("_cube", primitive);
	
	primitive = new Model();
	primitive->meshes.push_back(MeshManager::GetMesh("_quad"));
	resources.emplace("_quad", primitive);

	primitive = new Model();
	primitive->meshes.push_back(MeshManager::GetMesh("_fsQuad"));
	resources.emplace("_fsQuad", primitive);

	LoadAllFiles();
}

void ModelManager::LoadFromFile(const char* filename)
{
	// create an instance so we can easily configure it.
	Assimp::Importer importer;

	// disable pivot preserving, not needed for our purposes and gummys up the node heirarchy with noise.
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace	|
		aiProcess_FlipWindingOrder	|
		aiProcess_LimitBoneWeights);

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
		Animation* anim = new Animation();
		anim->name = scene->mAnimations[i]->mName.C_Str();
		anim->name = filename + anim->name;
		string log = "Processing Animation: " + anim->name;
		LogUtils::Log(log.c_str());
		anim->duration = scene->mAnimations[i]->mDuration;
		anim->ticksPerSecond = scene->mAnimations[i]->mTicksPerSecond;
		anim->numChannels = scene->mAnimations[i]->mNumChannels;

		for (int j = 0; j < anim->numChannels; j++) // for each channel (bone) in the animation
		{
			Animation::AnimationChannel channel;
			channel.name = scene->mAnimations[i]->mChannels[j]->mNodeName.C_Str();
			int keyCount = scene->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			for (int k = 0; k < keyCount; k++) // for each key frame in the channel in the animation
			{
				Animation::AnimationKey newKey;
				newKey.time = scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime; // Thus far for FBXs I have seen that keyframes exist at the same time for pos/rot/scale, so we can grab the time from any.
				newKey.position = vec3_cast(scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue);
				newKey.rotation = quat_cast(scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue);
				newKey.scale = vec3_cast(scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue);
				channel.keys.push_back(newKey);
			}
			anim->channels.emplace(channel.name, channel);
		}

		model->animations.push_back(anim);
		animations.emplace(anim->name,anim);
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