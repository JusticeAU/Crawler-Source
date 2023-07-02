#include "ComponentModel.h"

#include "Object.h"
#include "ModelManager.h"
#include "Model.h"

using std::to_string;

ComponentModel::ComponentModel(Object* parent, nlohmann::ordered_json j) : ComponentModel(parent)
{
	if (j.contains("model"))
	{
		j.at("model").get_to(modelName);
		model = ModelManager::GetModel(modelName);
	}
}

void ComponentModel::DrawGUI()
{
	string ModelStr = "Model##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(ModelStr.c_str(), model == nullptr? "NULL" : model->name.c_str()))
	{
		for (auto m : *ModelManager::Resources())
		{
			const bool is_selected = (m.second == model);
			if (ImGui::Selectable(m.first.c_str(), is_selected))
			{
				model = ModelManager::GetModel(m.first);
				//modelName = m.first;
				AnnounceChange();
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if(model != nullptr && model->childNodes != nullptr)
		model->childNodes->DrawGUISimple();
}

Component* ComponentModel::Clone(Object* parent)
{
	ComponentModel* copy = new ComponentModel(parent);
	copy->model = model;
	copy->modelName = modelName;
	return copy;
}
