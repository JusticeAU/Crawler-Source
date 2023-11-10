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

#include "LineRenderer.h"

ComponentRenderer::ComponentRenderer(Object* parent) : Component("Renderer", Component_Renderer, parent)
{
	closestPointLightUBO = new UniformBuffer(sizeof(int) * 8);
	Scene::AddRendererComponent(this);
}

ComponentRenderer::ComponentRenderer(Object* parent, nlohmann::ordered_json j) : ComponentRenderer(parent)
{
	auto matsJSON = j.at("materials");
	if (j.at("materials").is_null() != true)
	{
		for (auto it = matsJSON.begin(); it != matsJSON.end(); it++)
		{
			submeshMaterials.push_back(MaterialManager::GetMaterial(it.value()));
		}
		submeshBounds.resize(submeshMaterials.size());
	}
	
	j.at("receivesShadows").get_to(receivesShadows);
	j.at("castsShadows").get_to(castsShadows);
	if (j.contains("dontFrustumCull")) dontFrustumCull = true;

}

ComponentRenderer::~ComponentRenderer()
{
	Scene::RemoveRendererComponent(this);
	delete closestPointLightUBO;
}

void ComponentRenderer::Update(float delta)
{
	if (componentParent->wasDirtyTransform) // check if we should update our bounding boxes
	{
		for (int i = 0; i < model->meshes.size(); i++)
		{
			mat4 rotation = componentParent->transform * model->modelTransform;
			Mesh::AABB& aabb = model->meshes[i]->aabb;
			submeshBounds[i].points[0] = vec3(rotation * vec4(aabb.lowerA, 1));
			submeshBounds[i].points[1] = vec3(rotation * vec4(aabb.lowerB, 1));
			submeshBounds[i].points[2] = vec3(rotation * vec4(aabb.lowerC, 1));
			submeshBounds[i].points[3] = vec3(rotation * vec4(aabb.lowerD, 1));
			submeshBounds[i].points[4] = vec3(rotation * vec4(aabb.upperA, 1));
			submeshBounds[i].points[5] = vec3(rotation * vec4(aabb.upperB, 1));
			submeshBounds[i].points[6] = vec3(rotation * vec4(aabb.upperC, 1));
			submeshBounds[i].points[7] = vec3(rotation * vec4(aabb.upperD, 1));
		}
	}
}

void ComponentRenderer::UpdateClosestLights()
{
	vec3 position = componentParent->GetWorldSpacePosition();
	int numLights = Scene::GetNumPointLights();
	std::vector<std::pair<int, float>> lightDistances;
	for (int i = 0; i < numLights; i++)
	{
		glm::vec3 lightPosition = vec3(Scene::GetPointLightPositions()[i]);
		float distanceSquared = glm::length2(position - lightPosition);
		lightDistances.push_back(std::pair(i, distanceSquared));
	}

	// sort
	std::sort(lightDistances.begin(), lightDistances.end(), SceneRenderer::compareIndexDistancePair);

	// get 8 closest lights!
	for (int i = 0; i < 8; i++)
	{
		if (i < numLights)	closestPointLightIndices[i] = lightDistances[i].first;
		else				closestPointLightIndices[i] = -1;
	}

	closestPointLightUBO->SendData(&closestPointLightIndices);
}

void ComponentRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	// check if this thing should draw in this pass or not
	if (SceneRenderer::currentPassIsSplit && componentParent->isStatic != SceneRenderer::currentPassIsStatic) return;

	if (model == nullptr) return;

	for (int i = 0; i < model->meshes.size(); i++)
	{
		// Confirm a material is assigned
		if (submeshMaterials[i] == nullptr) return;

		// Frustum Cull
		if (SceneRenderer::cullingFrustum && !isAnimated && SceneRenderer::ShouldCull(submeshBounds[i].points))
		{
			if(mode == DrawMode::BatchedOpaque && SceneRenderer::frustumCullingShowBounds) DebugDrawBoundingBox(i, { 0.4, 0.0, 0 });
			continue;
		}


		// Prepare
		material = submeshMaterials[i];
		switch (mode)
		{
		case DrawMode::BatchedOpaque:
		{
			// basic transparensy pass testing
			if (material->blendMode == Material::BlendMode::Transparent)
			{
				SceneRenderer::transparentCalls.push_back(std::pair(this, i));
				continue;
			}

			SceneRenderer::renderBatch.AddDraw(this, i);
			if(SceneRenderer::frustumCullingShowBounds) DebugDrawBoundingBox(i, { 0, 0.4, 0 });
			break;
		}
		case DrawMode::Standard:
		{
			// basic transparensy pass testing
			if (material->blendMode == Material::BlendMode::Transparent)
			{
				SceneRenderer::transparentCalls.push_back(std::pair(this, i));
				continue;
			}

			BindShader();
			ApplyMaterials();
			BindMatricies(pv, position);
			SetUniforms();
			if (isAnimated)
				BindBoneTransform();
			model->DrawSubMesh(i);

			break;
		}
		case DrawMode::ObjectPicking:
		{
			if (componentParent->id == 0) // Don't waste ur time buddy.
				break;

			// basic transparent stuff doesnt draw here
			if (material->blendMode == Material::BlendMode::Transparent) return;

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
			if (!castsShadows) return;
			if (material->blendMode == Material::BlendMode::Transparent) return;

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
			if (!castsShadows) return;
			if (material->blendMode == Material::BlendMode::Transparent) return;

			ShaderProgram* shader = isAnimated ? ShaderManager::GetShaderProgram("engine/shader/lightPointShadowMapSkinned") : ShaderManager::GetShaderProgram("engine/shader/lightPointShadowMap");
			shader->Bind();
			glm::mat4 pvm = pv * componentParent->transform * model->modelTransform;

			// Positions and Rotations
			shader->SetMatrixUniform("pvmMatrix", pvm);
			shader->SetMatrixUniform("mMatrix", componentParent->transform * model->modelTransform);
			shader->SetVector3Uniform("cameraPosition", position);
			if (isAnimated)
				BindBoneTransform();

			// Check for alpha cutoff and configured if required.
			if (material != nullptr && material->blendMode == Material::BlendMode::AlphaCutoff)
			{
				shader->SetBoolUniform("useAlphaCutoff", true);
				material->albedoMap->Bind(4);
				shader->SetIntUniform("albedoMap", 4);
			}
			else shader->SetBoolUniform("useAlphaCutoff", false);

			model->DrawSubMesh(i);
			break;
		}
		case DrawMode::SSAOgBuffer:
		{
			if (material->blendMode == Material::BlendMode::Transparent) return;

			ShaderProgram* ssaoGeoShader = ShaderManager::GetShaderProgram("engine/shader/SSAOGeometryPass");
			// Positions and Rotations
			glm::mat4 modelMat = componentParent->transform * model->modelTransform;
			glm::mat4 pvm = pv * modelMat;
			ssaoGeoShader->SetMatrixUniform("pvmMatrix", pvm);
			ssaoGeoShader->SetMatrixUniform("model", modelMat);
			ssaoGeoShader->SetMatrixUniform("mMatrix", modelMat);

			if (isAnimated)
				BindBoneTransform();

			// Check for alpha cutoff and configured if required.
			if (material != nullptr && submeshMaterials[i]->blendMode == Material::BlendMode::AlphaCutoff)
			{
				ssaoGeoShader->SetBoolUniform("useAlphaCutoff", true);
				material->albedoMap->Bind(4);
				ssaoGeoShader->SetIntUniform("albedoMap", 4);
			}
			else ssaoGeoShader->SetBoolUniform("useAlphaCutoff", false);

			model->DrawSubMesh(i);

			break;
		}
		case DrawMode::Blended:
		{
			LogUtils::Log("Shouldn't get here now??");

			BindShader();
			ApplyMaterials();
			BindMatricies(pv, position);
			DrawModel();
			break;
		}
		}
	}
}

void ComponentRenderer::DrawGUI()
{
	ImGui::Checkbox("Do Not Frustum Cull", &dontFrustumCull);

	// material settings per submesh
	for (int i = 0; i < submeshMaterials.size(); i++)
	{
		ImGui::PushID(i);
		// Mesh Name
		ImGui::Text(model->GetMesh(i)->name.c_str());

		// Material
		string materialStr = "Mesh Material";
		if (ImGui::BeginCombo(materialStr.c_str(), submeshMaterials[i] != nullptr ? submeshMaterials[i]->name.c_str() : "NULL", ImGuiComboFlags_HeightLargest))
		{
			for (auto m : *MaterialManager::Materials())
			{
				const bool is_selected = (m.second == submeshMaterials[i]);
				if (ImGui::Selectable(m.first.c_str(), is_selected))
				{
					submeshMaterials[i] = MaterialManager::GetMaterial(m.first);
					modifiedMaterials = true;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::PopID();
	}
	ImGui::DragFloat("Emissive Scale", &emissiveScale, 0.05, 0, 1);
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
	Component* component = componentParent->GetComponent(Component_Model);
	if (component)
	{
		model = static_cast<ComponentModel*>(component)->model;
		if (model != nullptr)
		{
			submeshMaterials.resize(model->GetMeshCount());
			submeshMaterials.resize(model->GetMeshCount());
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
	glm::mat4 modelMat = componentParent->transform * model->modelTransform;
	glm::mat4 pvm = pv * modelMat;

	// Positions and Rotations
	ShaderProgram* shader = isAnimated ? material->shaderSkinned : material->shader;
	shader->SetMatrixUniform("pvmMatrix", pvm);
	shader->SetMatrixUniform("mMatrix", modelMat);
	shader->SetVector3Uniform("cameraPosition", position);
	shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
}

void ComponentRenderer::SetUniforms()
{
	// All of the below is fairly poor and just assumes that the shaders have these fields. Realistically some introspection in to the shader would be done to communicate to the application what fields they need.
	// those fields could be exposed in UI or some logic could intelligently only attemp to send data that is valid.
	// Likely part of a material system.

	// Lighting
	ShaderProgram* shader = isAnimated ? material->shaderSkinned : material->shader;

	shader->SetVector3Uniform("ambientLightColour", Scene::GetAmbientLightColour());
	shader->SetVector3Uniform("sunLightDirection", glm::normalize(Scene::GetSunDirection()));
	shader->SetVector3Uniform("sunLightColour", Scene::GetSunColour());

	// Point Lights
	int numLights = Scene::GetNumPointLights();
	shader->SetIntUniform("numLights", numLights);
	//shader->SetFloat3ArrayUniform("PointLightPositions", numLights, Scene::GetPointLightPositions());
	//shader->SetFloat3ArrayUniform("PointLightColours", numLights, Scene::GetPointLightColours());

	if (receivesShadows)
	{
		shader->SetFloatUniform("shadowBias", shadowBias);
	}
}

void ComponentRenderer::ApplyMaterials()
{
	// Material Uniforms
	ShaderProgram* shader = isAnimated ? material->shaderSkinned : material->shader;
	if (material)
	{
		// Check for alpha cutoff and configured if required.
		if (material->blendMode == Material::BlendMode::AlphaCutoff) shader->SetBoolUniform("useAlphaCutoff", true);
		else shader->SetBoolUniform("useAlphaCutoff", false);
		shader->SetFloatUniform("emissiveScale", emissiveScale);


		shader->SetVector3Uniform("Ka", material->Ka);
		shader->SetVector3Uniform("Kd", material->Kd);
		shader->SetVector3Uniform("Ks", material->Ks);
		shader->SetFloatUniform("specularPower", material->specularPower);

		if (material->mapKd)
		{
			material->mapKd->Bind(1);
			shader->SetIntUniform("diffuseTex", 1);
		}
		if (material->mapKs)
		{
			material->mapKs->Bind(2);
			shader->SetIntUniform("specularTex", 2);
		}
		if (material->mapBump)
		{
			material->mapBump->Bind(3);
			shader->SetIntUniform("normalTex", 3);
		}
		
		if (receivesShadows)
		{
			int numPointLights = Scene::GetNumPointLights();
			if (numPointLights > 0)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[0]]->BindTexture(11);
				shader->SetIntUniform("shadowMap0", 11);
			}
			if (numPointLights > 1)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[1]]->BindTexture(12);
				shader->SetIntUniform("shadowMap1", 12);
			}
			if (numPointLights > 2)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[2]]->BindTexture(13);
				shader->SetIntUniform("shadowMap2", 13);
			}
			if (numPointLights > 3)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[3]]->BindTexture(14);
				shader->SetIntUniform("shadowMap3", 14);
			}
			if (numPointLights > 4)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[4]]->BindTexture(15);
				shader->SetIntUniform("shadowMap4", 15);
			}
			if (numPointLights > 5)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[5]]->BindTexture(16);
				shader->SetIntUniform("shadowMap5", 16);
			}
			if (numPointLights > 6)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[6]]->BindTexture(17);
				shader->SetIntUniform("shadowMap6", 17);
			}
			if (numPointLights > 7)
			{
				SceneRenderer::pointLightCubeMapDynamic[closestPointLightIndices[7]]->BindTexture(18);
				shader->SetIntUniform("shadowMap7", 18);
			}
		}

		closestPointLightUBO->Bind(3);
		shader->SetUniformBlockIndex("pointLightIndicesBuffer", 3);

		// PBR
		if (!material->isPBR)
			return;

		if (material->albedoMap)
		{
			material->albedoMap->Bind(4);
			shader->SetIntUniform("albedoMap", 4);
		}
		if (material->normalMap)
		{
			material->normalMap->Bind(5);
			shader->SetIntUniform("normalMap", 5);
		}
		if (material->metallicMap)
		{
			material->metallicMap->Bind(6);
			shader->SetIntUniform("metallicMap", 6);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(6);
			shader->SetIntUniform("metallicMap", 6);
		}

		if (material->roughnessMap)
		{
			material->roughnessMap->Bind(7);
			shader->SetIntUniform("roughnessMap", 7);
		}
		if (material->aoMap)
		{
			material->aoMap->Bind(8);
			shader->SetIntUniform("aoMap", 8);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/white1x1.tga")->Bind(8);
			shader->SetIntUniform("aoMap", 8);
		}

		if (material->emissiveMap)
		{
			material->emissiveMap->Bind(9);
			shader->SetIntUniform("emissiveMap", 9);
		}
		else
		{
			TextureManager::GetTexture("engine/texture/black1x1.tga")->Bind(9);
			shader->SetIntUniform("emissiveMap", 9);
		}
	}
}

void ComponentRenderer::DrawModel()
{
	model->DrawAllSubMeshes();
}

void ComponentRenderer::BindBoneTransform()
{
	if (material == nullptr || material->shaderSkinned == nullptr)
		return;

	if (animator != nullptr && animator->boneTransfomBuffer != nullptr)
	{
		// get the shader uniform block index and set binding point - we'll just hardcode 0 for this.
		material->shaderSkinned->SetUniformBlockIndex("boneTransformBuffer", 0);
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

	copy->submeshMaterials.resize(submeshMaterials.size());
	for (int i = 0; i < submeshMaterials.size(); i++)
	{
		copy->submeshMaterials[i] = submeshMaterials[i];
	}
	return copy;
}
// Used by the SceneRenderer if frustumCullingShowBounds is enabled.
void ComponentRenderer::DebugDrawBoundingBox(int meshIndex, vec3 colour)
{
	Bounds& b = submeshBounds[meshIndex];
	LineRenderer::DrawLine(b.points[0], b.points[1], colour);
	LineRenderer::DrawLine(b.points[1], b.points[2], colour);
	LineRenderer::DrawLine(b.points[2], b.points[3], colour);
	LineRenderer::DrawLine(b.points[3], b.points[0], colour);
	
	LineRenderer::DrawLine(b.points[4], b.points[5], colour);
	LineRenderer::DrawLine(b.points[5], b.points[6], colour);
	LineRenderer::DrawLine(b.points[6], b.points[7], colour);
	LineRenderer::DrawLine(b.points[7], b.points[4], colour);
	
	LineRenderer::DrawLine(b.points[0], b.points[4], colour);
	LineRenderer::DrawLine(b.points[1], b.points[5], colour);
	LineRenderer::DrawLine(b.points[2], b.points[6], colour);
	LineRenderer::DrawLine(b.points[3], b.points[7], colour);
}
