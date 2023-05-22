#include "ComponentModel.h"

#include "Object.h"
#include "ModelManager.h"

using std::to_string;

ComponentModel::ComponentModel(Object* parent, std::istream& istream) : ComponentModel(parent)
{
	FileUtils::ReadString(istream, modelName);
	model = ModelManager::GetModel(modelName);
}

void ComponentModel::DrawGUI()
{
	string ModelStr = "Model##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(ModelStr.c_str(), modelName.c_str()))
	{
		for (auto m : *ModelManager::Resources())
		{
			const bool is_selected = (m.second == model);
			if (ImGui::Selectable(m.first.c_str(), is_selected))
			{
				model = ModelManager::GetModel(m.first);
				modelName = m.first;
				AnnounceChange();
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void ComponentModel::Write(std::ostream& ostream)
{
	FileUtils::WriteString(ostream, modelName);
}

Component* ComponentModel::Clone(Object* parent)
{
	ComponentModel* copy = new ComponentModel(parent);
	copy->model = model;
	copy->modelName = modelName;
	return copy;
}
