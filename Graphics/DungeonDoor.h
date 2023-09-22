#pragma once
#include <glm.hpp>
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonDoor
	{
	public:
		~DungeonDoor();
		void Toggle();
		void Toggle(bool on);

		void Update();
		void UpdateVisuals(float delta);
		void UpdateTransforms(bool instant = false);

		void PlayRattleSound();

		const float openEulerAngle = -120.0f;
		
		const float swingTime = 0.5f;
		float swingTimeCurrent = 0.0f;
		bool shouldSwing = false;

		bool shouldWobble = false;
		const float wobbleTime = 0.4f;
		float wobbleTimeCurrent = 0.0f;
		string wobbleSound = "crawler/sound/load/door_rattle.wav";

		glm::ivec2 position = { 0, 0 };
		int orientation = 0; // Use DIRECTION_MASK in DungeonHelpers.h
		unsigned int id = 0;
		

		unsigned int power = 0;
		bool open = false;
		
		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonDoor& door)
	{
		j = { {"position", door.position}, {"id", door.id}, {"orientation", door.orientation}, {"open", door.open} };
	}

	static void from_json(const ordered_json& j, DungeonDoor& door)
	{
		j.at("position").get_to(door.position);
		j.at("id").get_to(door.id);
		j.at("orientation").get_to(door.orientation);
		j.at("open").get_to(door.open);
	}
}