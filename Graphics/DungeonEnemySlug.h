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
			DEAD
		};
		DungeonEnemySlug();
		~DungeonEnemySlug();
		ivec2 position;
		FACING_INDEX facing;

		void UpdateTransform();
		void Kill(FACING_INDEX direction);

		STATE state = IDLE;

		bool shouldDelete = false;
		int slugTurns[4] = { 0, -1, 1, 2 }; // Forward,  Left, Right, 180.
		ivec2 positionPrevious; // need to track previous position so we can test if the player tried to move through us.

		Dungeon* dungeon;
		Object* object;

		void Update();

		void UpdateVisuals(float delta);

		void PlayMoveSFX();

		// visuals
		float moveSpeed = 0.40f;
		float moveCurrent = 0.0f;

		float spinSpeed = 0.65f;

		const float deathSpeed = 0.3;
		float deathCurrent = -0.4f;
		const float deathDeleteTime = 3.0f;

		glm::vec3 oldPosition;
		glm::vec3 targetPosition;
		FACING_INDEX deathDirection = NORTH_INDEX;


		// audio
		string audioMove[6] =
		{
			"crawler/sound/load/murderina/move_6.wav",
			"crawler/sound/load/murderina/move_6.wav",
			"crawler/sound/load/murderina/move_6.wav",
			"crawler/sound/load/murderina/move_6.wav",
			"crawler/sound/load/murderina/move_6.wav",
			"crawler/sound/load/murderina/move_6.wav"
		};
		const string audioMoveSingle = "crawler/sound/load/murderina/move.wav";
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