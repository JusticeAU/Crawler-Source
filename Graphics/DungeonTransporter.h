#pragma once
#include "DungeonHelpers.h"
#include "glm.hpp"
#include "serialisation.h"
#include <string>

using std::string;
using glm::ivec2;

class Object;

namespace Crawl
{
	class DungeonTransporter
	{
	public:
		~DungeonTransporter();
		string name = "";
		ivec2 position = { 0,0 };
		unsigned int fromOrientation = NORTH_INDEX;

		string toDungeon = "";
		string toTransporter = "";

		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonTransporter& object)
	{
		j = { {"name", object.name }, {"position", object.position }, {"fromOrientation", object.fromOrientation }, {"toDungeon", object.toDungeon }, {"toTransporter", object.toTransporter } };
	}

	static void from_json(const ordered_json& j, DungeonTransporter& object)
	{
		j.at("name").get_to(object.name);
		j.at("position").get_to(object.position);
		j.at("fromOrientation").get_to(object.fromOrientation);
		j.at("toDungeon").get_to(object.toDungeon);
		j.at("toTransporter").get_to(object.toTransporter);
	}
}

