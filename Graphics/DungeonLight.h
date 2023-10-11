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
		glm::vec3 localPosition = { 0,0,2.5f }; // scene scale position on grid square;

		bool isEnabled = false;
		glm::vec3 colour = { 1,1,1 };
		float intensity = 5.0f;

		bool isLobbyLight = false;
		bool startDisabled = false;

		Object* object = nullptr;
		ComponentLightPoint* light = nullptr;

		void Enable();
		void Disable();

		void UpdateTransform();
		void UpdateLight();

		void Flicker();
		void ConfigureFlickerState();
		void ResetRandomFlickerTime();

		void UpdateVisual(float delta);
		
		int id = 1;
		bool flickerIgnoreGlobal = false;

		float flickerBaseIntensity = 0.0f;
		float flickerOffset = 1.0f;
		float flickerCurrent = 0.0f;;
		float flickerTime = 1.0f;

		bool flickerEnabled = false;
		bool flickerRepeat = false;
		float flickerRepeatMin = 5.0f;
		float flickerRepeatMax = 20.0f;
	};

	static void to_json(ordered_json& j, const DungeonLight& object)
	{
		j = { {"position", object.position}, {"localPosition", object.localPosition}, {"colour", object.colour}, {"intensity", object.intensity}, {"id", object.id} };
		if (object.isLobbyLight) j["isLobbyLight"] = true;
		if (object.flickerIgnoreGlobal) j["flickerIgnoreGlobal"] = true;
		if (object.flickerRepeat)
		{
			j["flickerRepeat"] = true;
			j["flickerRepeatMin"] = object.flickerRepeatMin;
			j["flickerRepeatMax"] = object.flickerRepeatMax;
		}
		if (object.startDisabled) j["startDisabled"] = true;
	}

	static void from_json(const ordered_json& j, DungeonLight& object)
	{
		j.at("position").get_to(object.position);
		j.at("localPosition").get_to(object.localPosition);
		j.at("colour").get_to(object.colour);
		j.at("intensity").get_to(object.intensity);
		if (j.contains("id")) j.at("id").get_to(object.id);
		if (j.contains("isLobbyLight")) object.isLobbyLight = true;
		if (j.contains("flickerIgnoreGlobal")) object.flickerIgnoreGlobal = true;
		if (j.contains("flickerRepeat"))
		{
			j.at("flickerRepeat").get_to(object.flickerRepeat);
			j.at("flickerRepeatMin").get_to(object.flickerRepeatMin);
			j.at("flickerRepeatMax").get_to(object.flickerRepeatMax);
		}
		if (j.contains("startDisabled")) object.startDisabled = true;
	}
}