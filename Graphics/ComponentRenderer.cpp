#include "ComponentRenderer.h"
#include "ComponentModel.h"
#include "Scene.h"
#include "Camera.h"
#include "Model.h"

#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"

#include "FrameBuffer.h"

#include "Window.h"

#include <string>
#include "LogUtils.h"
using std::string;

ComponentRenderer::ComponentRenderer(Object* parent, std::istream& istream) : ComponentRenderer(parent)
{
	string line;
	
	// load in the quantity of materials saved for this renderer
	int meshQty = FileUtils::ReadInt(istream, meshQty);
	materialArray.resize(meshQty);

	// load in each material name.
	for (int i = 0; i < meshQty; i++)
	{
		FileUtils::ReadString(istream, line);
		materialArray[i] = MaterialManager::GetMaterial(line);
	}
}

void ComponentRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
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
					model->DrawSubMesh(i);
				}
				break;
			}
			case DrawMode::ObjectPicking:
			{
				ShaderProgram* shader = ShaderManager::GetShaderProgram("shaders/picking");
				shader->Bind();
				shader->SetUIntUniform("objectID", componentParent->id);
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform);
				shader->SetVectorUniform("cameraPosition", position);
				
				shader->SetIntUniform("objectID", componentParent->id);
				DrawModel();
				break;
			}
			case DrawMode::ShadowMapping:
			{
				if (!castsShadows)
					return;

				ShaderProgram* shader = ShaderManager::GetShaderProgram("shaders/simpleDepthShader");
				shader->Bind();
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shader->SetMatrixUniform("pvmMatrix", pvm);
				shader->SetMatrixUniform("mMatrix", componentParent->transform);
				shader->SetVectorUniform("cameraPosition", position);
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

		if (materialArray[i])
		{
			if (ImGui::CollapsingHeader("Material"))
				materialArray[i]->DrawGUI();
		}

		ImGui::PopID();
	}

	ImGui::Checkbox("Casts Shadows", &castsShadows);

	string targetStr = "Target##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(targetStr.c_str(), frameBufferName.c_str()))
	{
		for (auto fb : *TextureManager::FrameBuffers())
		{
			const bool is_selected = (fb.second == frameBuffer);
			if (ImGui::Selectable(fb.first.c_str(), is_selected))
			{
				frameBuffer = TextureManager::GetFrameBuffer(fb.first);
				frameBufferName = fb.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void ComponentRenderer::Write(std::ostream& ostream)
{
	// write the quantity of materials
	FileUtils::WriteInt(ostream, (int)materialArray.size());
	// write each material if its valid.
	for (int i = 0; i < materialArray.size(); i++)
	{
		if (materialArray[i] != nullptr)
			FileUtils::WriteString(ostream, materialArray[i]->name);
		else
			FileUtils::WriteString(ostream, "NULL");
	}
}

void ComponentRenderer::OnParentChange()
{
	Component* component = componentParent->GetComponent(Component_Model);
	if (component)
	{
		model = static_cast<ComponentModel*>(component)->model;
		if (model != nullptr)
			materialArray.resize(model->GetMeshCount());
	}
}

void ComponentRenderer::BindShader()
{
	material->shader->Bind();
}

void ComponentRenderer::BindMatricies(mat4 pv, vec3 position)
{
	// Combine the matricies
	glm::mat4 pvm = pv * componentParent->transform;

	// Positions and Rotations
	material->shader->SetMatrixUniform("pvmMatrix", pvm);
	material->shader->SetMatrixUniform("mMatrix", componentParent->transform);
	material->shader->SetVectorUniform("cameraPosition", position);
	material->shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
}

void ComponentRenderer::SetUniforms()
{
	// All of the below is fairly poor and just assumes that the shaders have these fields. Realistically some introspection in to the shader would be done to communicate to the application what fields they need.
	// those fields could be exposed in UI or some logic could intelligently only attemp to send data that is valid.
	// Likely part of a material system.

	// Lighting
	material->shader->SetVectorUniform("ambientLightColour", Scene::GetAmbientLightColour());
	material->shader->SetVectorUniform("sunLightDirection", glm::normalize(Scene::GetSunDirection()));
	material->shader->SetVectorUniform("sunLightColour", Scene::GetSunColour());
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
		material->shader->SetVectorUniform("Ka", material->Ka);
		material->shader->SetVectorUniform("Kd", material->Kd);
		material->shader->SetVectorUniform("Ks", material->Ks);
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

	}
}

void ComponentRenderer::DrawModel()
{
	model->DrawAllSubMeshes();
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