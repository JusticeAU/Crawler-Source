#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;

using glm::ivec2;

namespace Crawl
{
	class Dungeon;

	class DungeonEnemySlug
	{
	public:
		enum STATE {
			INACTIVE,
			IDLE,
			MOVING,
			TURNING,
			BOUNCING,
			STUN
		};

		enum class Command {
			TURN_LEFT,
			TURN_RIGHT,
			WALK_FORWARD,
		};
		const static std::string commandStrings[3];

		~DungeonEnemySlug();
		ivec2 position;
		FACING_INDEX facing;
		STATE state = INACTIVE;

		std::vector<Command> commands;
		int commandIndex = 0;

		Dungeon* dungeon;
		Object* object;

		void Update();
		void NextCommand();

		void UpdateVisuals(float delta);

		// visuals
		float moveSpeed = 0.25f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;
		float turnSpeed = 0.15f;
		float turnCurrent = 0.0f;
		float oldTurn;
		float targetTurn;
		float bounceSpeed = 0.5;
		float bounceCurrent = 0.0f;

	};

	static void to_json(ordered_json& j, const DungeonEnemySlug& object)
	{
		j = { {"position", object.position }, {"facing", object.facing }, {"commands", object.commands } };
	}

	static void from_json(const ordered_json& j, DungeonEnemySlug& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		j.at("commands").get_to(object.commands);
	}
}