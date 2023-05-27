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
	FileUtils::ReadString(istream, textureName);
	texture = TextureManager::GetTexture(textureName);
	FileUtils::ReadString(istream, shaderName);
	shader = ShaderManager::GetShaderProgram(shaderName);
	FileUtils::ReadString(istream, materialName);
	material = MaterialManager::GetMaterial(materialName);
}

void ComponentRenderer::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	if (model != nullptr && shader != nullptr) // At minimum we need a model and a shader to draw something.
	{
		switch (mode)
		{
			case DrawMode::Standard:
			{
				BindShader();
				BindMatricies(pv, position);
				break;
			}
			case DrawMode::ObjectPicking:
			{
				ShaderProgram* shad = ShaderManager::GetShaderProgram("shaders/picking");
				shad->Bind();
				shad->SetUIntUniform("objectID", componentParent->id);
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shad->SetMatrixUniform("pvmMatrix", pvm);
				shad->SetMatrixUniform("mMatrix", componentParent->transform);
				shad->SetVectorUniform("cameraPosition", position);
				break;
			}
			case DrawMode::ShadowMapping:
			{
				if (!castsShadows)
					return;

				ShaderProgram* shad = ShaderManager::GetShaderProgram("shaders/simpleDepthShader");
				shad->Bind();
				glm::mat4 pvm = pv * componentParent->transform;

				// Positions and Rotations
				shad->SetMatrixUniform("pvmMatrix", pvm);
				shad->SetMatrixUniform("mMatrix", componentParent->transform);
				shad->SetVectorUniform("cameraPosition", position);
				break;
			}
		}

		SetUniforms();
		ApplyTexture();
		ApplyMaterials();
		DrawModel();
	}
}

void ComponentRenderer::DrawGUI()
{
	ImGui::Checkbox("Casts Shadows", &castsShadows);
	string textureStr = "Texture##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(textureStr.c_str(), textureName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == texture);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				texture = TextureManager::GetTexture(t.first);
				textureName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	string shaderStr = "Shader##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(shaderStr.c_str(), shaderName.c_str()))
	{
		for (auto s : *ShaderManager::ShaderPrograms())
		{
			const bool is_selected = (s.second == shader);
			if (ImGui::Selectable(s.first.c_str(), is_selected))
			{
				shader = ShaderManager::GetShaderProgram(s.first);
				shaderName = s.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	string materialStr = "Material##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(materialStr.c_str(), materialName.c_str()))
	{
		for (auto m : *MaterialManager::Materials())
		{
			const bool is_selected = (m.second == material);
			if (ImGui::Selectable(m.first.c_str(), is_selected))
			{
				material = MaterialManager::GetMaterial(m.first);
				materialName = m.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (material)
	{
		ImGui::Indent();
		if (ImGui::CollapsingHeader("Material"))
		{
			material->DrawGUI();
		}
		ImGui::Unindent();
	}

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
	FileUtils::WriteString(ostream, textureName);
	FileUtils::WriteString(ostream, shaderName);
	FileUtils::WriteString(ostream, materialName);
}

void ComponentRenderer::OnParentChange()
{
	Component* component = componentParent->GetComponent(Component_Model);
	if (component)
	{
		model = static_cast<ComponentModel*>(component)->model;
	}

	texture = TextureManager::GetTexture(textureName);
}

void ComponentRenderer::BindShader()
{
	shader->Bind();
}

void ComponentRenderer::BindMatricies(mat4 pv, vec3 position)
{
	// Combine the matricies
	glm::mat4 pvm = pv * componentParent->transform;

	// Positions and Rotations
	shader->SetMatrixUniform("pvmMatrix", pvm);
	shader->SetMatrixUniform("mMatrix", componentParent->transform);
	shader->SetVectorUniform("cameraPosition", position);
	shader->SetMatrixUniform("lightSpaceMatrix", Scene::GetLightSpaceMatrix());
}

void ComponentRenderer::SetUniforms()
{
	// All of the below is fairly poor and just assumes that the shaders have these fields. Realistically some introspection in to the shader would be done to communicate to the application what fields they need.
	// those fields could be exposed in UI or some logic could intelligently only attemp to send data that is valid.
	// Likely part of a material system.

	// Lighting
	shader->SetVectorUniform("ambientLightColour", Scene::GetAmbientLightColour());
	shader->SetVectorUniform("sunLightDirection", glm::normalize(Scene::GetSunDirection()));
	shader->SetVectorUniform("sunLightColour", Scene::GetSunColour());
	// Point Lights
	int numLights = Scene::GetNumPointLights();
	shader->SetIntUniform("numLights", numLights);
	shader->SetFloat3ArrayUniform("PointLightPositions", numLights, Scene::GetPointLightPositions());
	shader->SetFloat3ArrayUniform("PointLightColours", numLights, Scene::GetPointLightColours());

	shader->SetIntUniform("objectID", componentParent->id);

	if (receivesShadows)
	{
		shader->SetFloatUniform("shadowBias", shadowBias);
	}
}

void ComponentRenderer::ApplyTexture()
{
	// Texture Uniforms
	if (texture)
	{
		texture->Bind(1);
		shader->SetIntUniform("diffuseTex", 1);
	}

	// bind shadow map
	shader->SetIntUniform("shadowMap", 5);
}

void ComponentRenderer::ApplyMaterials()
{
	// Material Uniforms
	if (material)
	{
		shader->SetVectorUniform("Ka", material->Ka);
		shader->SetVectorUniform("Kd", material->Kd);
		shader->SetVectorUniform("Ks", material->Ks);
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

		// PBR
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

	}
}

void ComponentRenderer::DrawModel()
{
	model->Draw();
}

Component* ComponentRenderer::Clone(Object* parent)
{
	ComponentRenderer* copy = new ComponentRenderer(parent);
	copy->castsShadows = castsShadows;
	copy->material = material;
	copy->materialName = materialName;
	copy->model = model;
	copy->receivesShadows = receivesShadows;
	copy->shader = shader;
	copy->shaderName = shaderName;
	copy->shadowBias = shadowBias;
	copy->texture = texture;
	copy->textureName = textureName;
	return copy;
}