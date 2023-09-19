#pragma once
#include <glm.hpp>
#include "serialisation.h"
#include "ComponentFactory.h"

class Object;

namespace Crawl
{
	class DungeonLight
	{
	public:
		~DungeonLight();
		glm::ivec2 position = { 0,0 }; // grid coordinate
		glm::vec3 localPosition = { 0,0,0 }; // scene scale position on grid square;
		
		glm::vec3 colour = { 1,1,1 };
		float intensity = 5.0f;

		bool isLobbyLight = false;

		Object* object = nullptr;
		ComponentLightPoint* light = nullptr;

		void Init();

		void UpdateTransform();
		void UpdateLight();

	};

	static void to_json(ordered_json& j, const DungeonLight& object)
	{
		j = { {"position", object.position}, {"localPosition", object.localPosition}, {"colour", object.colour}, {"intensity", object.intensity} };
		if (object.isLobbyLight) j["isLobbyLight"] = true;
	}

	static void from_json(const ordered_json& j, DungeonLight& object)
	{
		j.at("position").get_to(object.position);
		j.at("localPosition").get_to(object.localPosition);
		j.at("colour").get_to(object.colour);
		j.at("intensity").get_to(object.intensity);
		if(j.contains("isLobbyLight")) j.at("isLobbyLight").get_to(object.isLobbyLight);
	}
}