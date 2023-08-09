#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonDecoration
	{
	public:
		~DungeonDecoration();
		glm::ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;
		
		glm::vec3 localPosition = {0,0,0};
		std::string modelName = "";

		void UpdateTransform();
		void LoadDecoration();

		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonDecoration& object)
	{
		j = { {"position", object.position }, {"facing", object.facing}, {"localPosition", object.localPosition }, {"modelName", object.modelName } };
	}

	static void from_json(const ordered_json& j, DungeonDecoration& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		j.at("localPosition").get_to(object.localPosition);
		j.at("modelName").get_to(object.modelName);
	}
}

