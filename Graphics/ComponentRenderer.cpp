#include "ComponentRenderer.h"
#include "ComponentModel.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "Model.h"

#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"

#include "ComponentAnimator.h"
#include "UniformBuffer.h"

#include "FrameBuffer.h"

#include "Window.h"
#include "ComponentCamera.h";

#include <string>
#include "LogUtils.h"
using std::string;

ComponentRenderer::ComponentRenderer(Object* parent, nlohmann::ordered_json j) : ComponentRenderer(parent)
{
	auto matsJSON = j.at("materials");
	if (j.at("materials").is_null() != true)
	{
		for (auto it = matsJSON.begin(); it != matsJSON.end(); it++)
			materialArray.push_back(MaterialManager::GetMaterial(it.value()));
	}
	
	j.at("frameBuffer").get_to(frameBufferName);
	frameBuffer = TextureManager::GetFrameBuffer(frameBufferName);
	j.at("receivesShadows").get_to(receivesShadows);
	j.at("castsShadows").get_to(castsShadows);

}

void ComponentRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	// check if this thing should draw in this pass or not
	if (SceneRenderer::currentPassIsSplit)
	{
		if (componentParent->isStatic != SceneRenderer::currentPassIsStatic) return;
	}

	// The culling Frustum should be set before any draw pass. If there is one set, then it is tested against.
	if (SceneRenderer::cullingFrustum)
	{
		vec3 modelPos = componentParent->GetWorldSpacePosition();
		if(SceneRenderer::ShouldCull(modelPos)) return;
	}

	if (model != nullptr && materialArray[0] != nullptr) // At minimum we need a model and a shader to draw something.
	{
		switch (mode)
		{
			case DrawMode::Standard:
			{
				for (int i = 0; i < materialArray.size(); i++)
				{
					if (materialArray[i] != nullptr)
						material = materialArray[i];
					else
						continue;
					
					if (material->shader == nullptr)
						continue;

					BindShader();
					ApplyMaterials();
					BindMatricies(pv, position);
					SetUniforms();
					if(isAnimated)
						BindBoneTransform();
					model->DrawSubMesh(i);
				}
				break;
			}
			case DrawMode::ObjectPicking:
			{
				if (componentParent->id == 0) // Don't waste ur time buddy.
					break;

				ShaderProgram* shader = isAnimated ? ShaderManager::GetShaderProgram("engine/shader/skinnedPicking") : ShaderManager::GetShaderProgram("engine/shader/picking");
				shader->Bind();
				shader->SetUIntUniform("objectID", componentParent->id);
				glm::mat4 pvm = pv * componentParent->transform * model->modelTransform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform * model->modelTransform);
				shader->SetVector3Uniform("cameraPosition", position);
				
				shader->SetIntUniform("objectID", componentParent->id);
				if (isAnimated)
					BindBoneTransform();
				DrawModel();
				break;
			}
			case DrawMode::ShadowMapping:
			{
				if (!castsShadows)
					return;

				ShaderProgram* shader = isAnimated ? ShaderManager::GetShaderProgram("engine/shader/simpleDepthShaderSkinned") : ShaderManager::GetShaderProgram("engine/shader/simpleDepthShader");
				shader->Bind();
				glm::mat4 pvm = pv * componentParent->transform * model->modelTransform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform * model->modelTransform);
				shader->SetVector3Uniform("cameraPosition", position);
				if (isAnimated)
					BindBoneTransform();
				DrawModel();
				break;
			}
			case DrawMode::ShadowCubeMapping:
			{
				if (!castsShadows)
					return;

				ShaderProgram* shader = isAnimated ? ShaderManager::GetShaderProgram("engine/shader/lightPointShadowMap") : ShaderManager::GetShaderProgram("engine/shader/lightPointShadowMap");
				shader->Bind();
				glm::mat4 pvm = pv * componentParent->transform * model->modelTransform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform * model->modelTransform);
				shader->SetVector3Uniform("cameraPosition", position);
				if (isAnimated)
					BindBoneTransform();
				DrawModel();
				break;
			}
			case DrawMode::SSAOgBuffer:
			{
				ShaderProgram* ssaoGeoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
				ssaoGeoShader->SetMatrixUniform("model", componentParent->transform * model->modelTransform);
				if (isAnimated)
					BindBoneTransform();
				DrawModel();
				break;
			}
		}


	}
}

void ComponentRenderer::DrawGUI()
{
	// material settings per submesh
	for (int i = 0; i < materialArray.size(); i++)
	{
		ImGui::PushID(i);
		// Mesh Name
		ImGui::Text(model->GetMesh(i)->name.c_str());

		// Material
		string materialStr = "Mesh Material";
		if (ImGui::BeginCombo(materialStr.c_str(), materialArray[i] != nullptr ? materialArray[i]->name.c_str() : "NULL"))
		{
			for (auto m : *MaterialManager::Materials())
			{
				const bool is_selected = (m.second == materialArray[i]);
				if (ImGui::Selectable(m.first.c_str(), is_selected))
					materialArray[i] = MaterialManager::GetMaterial(m.first);

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::PopID();
	}

	//ImGui::Checkbox("Casts Shadows", &castsShadows);

	//string targetStr = "Target##" + to_string(componentParent->id);
	//if (ImGui::BeginCombo(targetStr.c_str(), frameBufferName.c_str()))
	//{
	//	for (auto fb : *TextureManager::FrameBuffers())
	//	{
	//		const bool is_selected = (fb.second == frameBuffer);
	//		if (ImGui::Selectable(fb.first.c_str(), is_selected))
	//		{
	//			frameBuffer = TextureManager::GetFrameBuffer(fb.first);
	//			frameBufferName = fb.first;
	//		}

	//		// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
	//		if (is_selected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndCombo();
	//}
}

void ComponentRenderer::OnParentChange()
{
;

	Component* component = componentParent->GetComponent(Component_Model);
	if (component)
	{
		model = static_cast<ComponentModel*>(component)->model;
		if (model != nullptr)
		{
			materialArray.resize(model->GetMeshCount());
			isAnimated = (model->animations.size() > 0);
		}
	}

	animator = nullptr;
	animationBlender = nullptr;
	component = nullptr;
	component = componentParent->GetComponent(Component_Animator);
	if (component)
		animator = static_cast<ComponentAnimator*>(component);
}

void ComponentRenderer::BindShader()
{
	if(!isAnimated)
		material->shader->Bind();
	else
		material->shaderSkinned->Bind();
}

void ComponentRenderer::BindMatricies(mat4 pv, vec3 position)
{
	// Combine the matricies
	glm::mat4 pvm = pv * componentParent->transform * model->modelTransform;

	// Positions and Rotations
	material->shader->SetMatrixUniform("pvmMatrix", pvm);
	material->shader->SetMatrixUniform("mMatrix", componentParent->transform * model->modelTransform);
	material->shader->SetVector3Uniform("cameraPosition", position);
	material->shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
}

void ComponentRenderer::SetUniforms()
{
	// All of the below is fairly poor and just assumes that the shaders have these fields. Realistically some introspection in to the shader would be done to communicate to the application what fields they need.
	// those fields could be exposed in UI or some logic could intelligently only attemp to send data that is valid.
	// Likely part of a material system.

	// Lighting
	material->shader->SetVector3Uniform("ambientLightColour", Scene::GetAmbientLightColour());
	material->shader->SetVector3Uniform("sunLightDirection", glm::normalize(Scene::GetSunDirection()));
	material->shader->SetVector3Uniform("sunLightColour", Scene::GetSunColour());
	// Point Lights
	int numLights = Scene::GetNumPointLights();
	material->shader->SetIntUniform("numLights", numLights);
	material->shader->SetFloat3ArrayUniform("PointLightPositions", numLights, Scene::GetPointLightPositions());
	material->shader->SetFloat3ArrayUniform("PointLightColours", numLights, Scene::GetPointLightColours());

	if (receivesShadows)
	{
		material->shader->SetFloatUniform("shadowBias", shadowBias);
	}
}

void ComponentRenderer::ApplyMaterials()
{
	// Material Uniforms
	if (material)
	{
		material->shader->SetVector3Uniform("Ka", material->Ka);
		material->shader->SetVector3Uniform("Kd", material->Kd);
		material->shader->SetVector3Uniform("Ks", material->Ks);
		material->shader->SetFloatUniform("specularPower", material->specularPower);

		if (material->mapKd)
		{
			material->mapKd->Bind(1);
			material->shader->SetIntUniform("diffuseTex", 1);
		}
		if (material->mapKs)
		{
			material->mapKs->Bind(2);
			material->shader->SetIntUniform("specularTex", 2);
		}
		if (material->mapBump)
		{
			material->mapBump->Bind(3);
			material->shader->SetIntUniform("normalTex", 3);
		}

		// PBR
		if (material->albedoMap)
		{
			material->albedoMap->Bind(4);
			material->shader->SetIntUniform("albedoMap", 4);
		}
		if (material->normalMap)
		{
			material->normalMap->Bind(5);
			material->shader->SetIntUniform("normalMap", 5);
		}
		if (material->metallicMap)
		{
			material->metallicMap->Bind(6);
			material->shader->SetIntUniform("metallicMap", 6);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(6);
			material->shader->SetIntUniform("metallicMap", 6);
		}

		if (material->roughnessMap)
		{
			material->roughnessMap->Bind(7);
			material->shader->SetIntUniform("roughnessMap", 7);
		}
		if (material->aoMap)
		{
			material->aoMap->Bind(8);
			material->shader->SetIntUniform("aoMap", 8);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/white1x1.tga")->Bind(8);
			material->shader->SetIntUniform("aoMap", 8);
		}

		if (material->emissiveMap)
		{
			material->emissiveMap->Bind(9);
			material->shader->SetIntUniform("emissiveMap", 9);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(9);
			material->shader->SetIntUniform("emissiveMap", 9);
		}
		SceneRenderer::pointShadowMap2[0]->BindTexture(11);
		material->shader->SetIntUniform("shadowMap0", 11);
		SceneRenderer::pointShadowMap2[1]->BindTexture(12);
		material->shader->SetIntUniform("shadowMap1", 12);
		SceneRenderer::pointShadowMap2[2]->BindTexture(13);
		material->shader->SetIntUniform("shadowMap2", 13);
		SceneRenderer::pointShadowMap2[3]->BindTexture(14);
		material->shader->SetIntUniform("shadowMap3", 14);
	}
}

void ComponentRenderer::DrawModel()
{
	model->DrawAllSubMeshes();
}

void ComponentRenderer::BindBoneTransform()
{
	if (material == nullptr || material->shader == nullptr)
		return;

	// skinned mesh rendering
	//shader->SetIntUniform("selectedBone", animator->selectedBone); // dev testing really, used by boneWeights shader
	if (animator != nullptr && animator->boneTransfomBuffer != nullptr)
	{
		// get the shader uniform block index and set binding point - we'll just hardcode 0 for this.
		material->shader->SetUniformBlockIndex("boneTransformBuffer", 0);
		// TODO - this could be done in shader initialisation if it detected that that shader had this uniform buffer
		// It only needs to be done once per shader too, assuming im not reusing the index anywhere else (which im not)

		// Now link the UniformBufferObject to the bind point in the GL context.
		animator->boneTransfomBuffer->Bind(0);

		animator->boneTransfomBuffer->SendData(animator->boneTransforms);

		// Technically, all animated models can share the same boneTransformBuffer and just keep uploading their data in to it every frame.
		// A dedicated animation system could provide a single global buffer for the shader for this.
	}
}

Component* ComponentRenderer::Clone(Object* parent)
{
	ComponentRenderer* copy = new ComponentRenderer(parent);
	copy->castsShadows = castsShadows;
	copy->material = material;
	copy->model = model;
	copy->receivesShadows = receivesShadows;
	copy->shadowBias = shadowBias;

	copy->materialArray.resize(materialArray.size());
	for (int i = 0; i < materialArray.size(); i++)
	{
		copy->materialArray[i] = materialArray[i];
	}
	return copy;
}