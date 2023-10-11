#include "DungeonGameManagerEvent.h"
#include "imgui.h"

bool Crawl::DungeonGameManagerEvent::DrawGUIInternal()
{
	bool madeChange = false;
	ImGui::PushItemWidth(100);
	if (ImGui::BeginCombo("Type", typeString[(int)type].c_str()))
	{
		/*for (int i = 0; i < 3; i++)
		{
			bool selected = ((int)type == i);
			if (ImGui::Selectable(typeString[i].c_str(), selected)) type = (Type)i;
		}*/
		// Just hard code the two options for now, doesn't need to be complicated and doors and lights are combined for time being.
		if (ImGui::Selectable(typeString[0].c_str(), (int)type == 0))
		{
			type = (Type)0;
			madeChange = true;
		}
		if (ImGui::Selectable(typeString[2].c_str(), (int)type == 2))
		{
			type = (Type)2;
			madeChange = true;
		}
		ImGui::EndCombo();
	}
	
	switch (type)
	{
	case Type::Door:
	{
		ImGui::SameLine();
		if (ImGui::BeginCombo("Door", doorNames[id].c_str()))
		{
			for (int i = 0; i < 8; i++)
			{
				bool selected = ((int)id == i);
				if (ImGui::Selectable(doorNames[i].c_str(), selected))
				{
					id = i;
					madeChange = true;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::BeginCombo("State", doorStateString[status].c_str()))
		{
			for (int i = 0; i < 3; i++)
			{
				bool selected = ((int)status == i);
				if (ImGui::Selectable(doorStateString[i].c_str(), selected))
				{
					status = i;
					madeChange = true;
				}
			}
			ImGui::EndCombo();
		}
		break;
	}
	case Type::Lock:
	{
		ImGui::SameLine();
		if (ImGui::BeginCombo("Lock", std::to_string(id+1).c_str()))
		{
			for (int i = 0; i < 4; i++)
			{
				if (ImGui::Selectable(std::to_string(i+1).c_str(), id == i))
				{
					id = i;
					madeChange = true;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (status > 1) status = 0;
		if (ImGui::BeginCombo("State", status == 0 ? "Off" : "On"))
		{
			if (ImGui::Selectable("On", status == 1))
			{
				status = 1;
				madeChange = true;
			}
			if (ImGui::Selectable("Off", status == 0))
			{
				status = 0;
				madeChange = true;
			}

			ImGui::EndCombo();
		}
		break;
	}
	}
	ImGui::PopItemWidth();

	return madeChange;
}

void Crawl::to_json(nlohmann::ordered_json& j, const DungeonGameManagerEvent& gme)
{
	j = { {"type", gme.type}, {"id", gme.id}, {"status", gme.status} };
}

void Crawl::from_json(const nlohmann::ordered_json& j, DungeonGameManagerEvent& gme)
{
	j.at("type").get_to(gme.type);
	j.at("id").get_to(gme.id);
	j.at("status").get_to(gme.status);
}