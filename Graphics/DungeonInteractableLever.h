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

		//void Prime();

		static const float wallHeightOff;
		static const float wallHeightOn;

		bool status = false;

		Dungeon* dungeon = nullptr;
		Object* object = nullptr;

		unsigned int activateID = 0;
	};

	static void to_json(ordered_json& j, const DungeonInteractableLever& lever)
	{
		j = { {"position", lever.position }, {"id", lever.id }, {"orientation", lever.orientation }, {"status", lever.status}, {"activateID", lever.activateID }};
	}

	static void from_json(const ordered_json& j, DungeonInteractableLever& lever)
	{
		j.at("position").get_to(lever.position);
		j.at("id").get_to(lever.id);
		j.at("orientation").get_to(lever.orientation);
		
		if(j.contains("startStatus"))
			j.at("startStatus").get_to(lever.status);
		if (j.contains("status"))
			j.at("status").get_to(lever.status);

		j.at("activateID").get_to(lever.activateID);
	}
}