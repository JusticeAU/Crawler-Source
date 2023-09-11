#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;
class ComponentRenderer;

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

		bool castsShadows = true;

		void UpdateTransform();
		void LoadDecoration();

		void UpdateShadowCasting();

		Object* object = nullptr;
		ComponentRenderer* renderer = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonDecoration& object)
	{
		j = { {"position", object.position }, {"facing", object.facing}, {"localPosition", object.localPosition }, {"modelName", object.modelName } };
		if (!object.castsShadows) j["castsShadows"] = false;
	}

	static void from_json(const ordered_json& j, DungeonDecoration& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		j.at("localPosition").get_to(object.localPosition);
		j.at("modelName").get_to(object.modelName);
		if (j.contains("castsShadows")) j.at("castsShadows").get_to(object.castsShadows);
	}
}

