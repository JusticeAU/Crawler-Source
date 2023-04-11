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

void ComponentRenderer::Draw()
{
	if (model != nullptr && shader != nullptr) // At minimum we need a model and a shader to draw something.
	{
		BindShader();
		BindMatricies();
		SetUniforms();
		ApplyTexture();
		ApplyMaterials();
		if (frameBuffer)
			frameBuffer->BindTarget();
		DrawModel();
		if (frameBuffer)
		{
			FrameBuffer::UnBindTarget();
			glViewport(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
		}
	}
}

void ComponentRenderer::DrawGUI()
{
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
}

void ComponentRenderer::BindShader()
{
	shader->Bind();
}

void ComponentRenderer::BindMatricies()
{
	// Combine the matricies
	glm::mat4 pvm = Camera::s_instance->GetMatrix() * componentParent->transform;

	// Positions and Rotations
	shader->SetMatrixUniform("pvmMatrix", pvm);
	shader->SetMatrixUniform("mMatrix", componentParent->transform);
	shader->SetVectorUniform("cameraPosition", Camera::s_instance->GetPosition());
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
}

void ComponentRenderer::ApplyTexture()
{
	// Texture Uniforms
	if (texture)
	{
		texture->Bind(1);
		shader->SetIntUniform("diffuseTex", 1);
	}
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
	}
}

void ComponentRenderer::DrawModel()
{
	model->Draw();
}
