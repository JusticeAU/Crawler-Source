#include "ComponentBillboard.h"
#include "TextureManager.h"
#include "Texture.h"
#include "SceneRenderer.h"

ComponentBillboard::ComponentBillboard(Object* parent) : Component("Billboard", Component_Billboard, parent)
{
}

ComponentBillboard::~ComponentBillboard()
{
}

void ComponentBillboard::Draw(mat4 pv, vec3 position, DrawMode mode)
{
	if (mode != DrawMode::Billboard) return;
	if (!texture) return;

	SceneRenderer::AddBillBoardDraw(this);
}

void ComponentBillboard::DrawGUI()
{
	// Set Texture
	if (ImGui::BeginCombo("Tex", texture == nullptr ? "NULL" : texture->name.c_str(), ImGuiComboFlags_HeightLargest))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == texture);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				texture = TextureManager::GetTexture(t.first);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

Component* ComponentBillboard::Clone(Object* parent)
{
	ComponentBillboard* copy = new ComponentBillboard(parent);
	copy->texture = texture;
	return copy;
}
