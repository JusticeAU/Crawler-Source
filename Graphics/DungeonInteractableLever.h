#pragma once
#include "DungeonInteractable.h"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonInteractableLever : public DungeonInteractable
	{
	public:
		~DungeonInteractableLever();
		void Toggle() override;
		void SetID(unsigned int newID);

		bool startStatus = false;
		bool status = false;

		Dungeon* dungeon = nullptr;
		Object* object = nullptr;

		unsigned int doorID = 0;
	};

	static void to_json(ordered_json& j, const DungeonInteractableLever& lever)
	{
		j = { {"position", lever.position }, {"id", lever.id }, {"orientation", lever.orientation }, {"status", lever.status}, {"doorID", lever.doorID }};
	}

	static void from_json(const ordered_json& j, DungeonInteractableLever& lever)
	{
		j.at("position").get_to(lever.position);
		j.at("id").get_to(lever.id);
		j.at("orientation").get_to(lever.orientation);
		j.at("status").get_to(lever.startStatus);
		j.at("doorID").get_to(lever.doorID);
	}
}