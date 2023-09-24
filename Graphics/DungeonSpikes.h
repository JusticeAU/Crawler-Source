#pragma once
#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonSpikes
	{
	public:
		~DungeonSpikes();
		ivec2 position = {0,0};
		
		bool disabled = false;

		void Disable();
		
		Dungeon* dungeon = nullptr;
		Object* object = nullptr;
		Object* placeHolderContainerObject = nullptr;

	private:
	};

	static void to_json(ordered_json& j, const DungeonSpikes& object)
	{
		j = { {"position", object.position } };
		if(object.disabled)
			j["disabled"] = object.disabled;
	}

	static void from_json(const ordered_json& j, DungeonSpikes& object)
	{
		j.at("position").get_to(object.position);
		if (j.contains("disabled"))
			j.at("disabled").get_to(object.disabled);
	}
}