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

		~DungeonEnemySlug();
		ivec2 position;
		FACING_INDEX facing;
		STATE state = INACTIVE;

		bool isDead = false;

		int slugTurns[4] = { 0, -1, 1, 2 }; // Forward,  Left, Right, 180.

		Dungeon* dungeon;
		Object* object;

		void Update();

		void UpdateVisuals(float delta);

		// visuals
		float moveSpeed = 1.0f;
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
		j = { {"position", object.position }, {"facing", object.facing } };
	}

	static void from_json(const ordered_json& j, DungeonEnemySlug& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
	}
}