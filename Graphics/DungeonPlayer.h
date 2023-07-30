#pragma once
#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "glm.hpp"

class Object;

namespace Crawl
{
	class DungeonPlayer
	{
	public:

		enum STATE {
			IDLE,
			MOVING,
			TURNING
		};
		DungeonPlayer();

		void SetDungeon(Dungeon* dungeonPtr) { this->dungeon = dungeonPtr; }
		void SetPlayerObject(Object* objectPtr) { this->object = objectPtr; }

		bool Update(float deltaTime);

		ivec2 GetPosition() { return position; }
		FACING_INDEX GetOrientation() { return facing; }
		void Teleport(ivec2 position);
		void Orient(FACING_INDEX facing);
		void SetRespawn(ivec2 position, FACING_INDEX orientation);
		void Respawn();

		void TakeDamage();

		void SetShouldSwitchWith(DungeonEnemySwitcher* switcher) { shouldSwitchWith = switcher; }

		// combines our requested direction with our facing direction to return the actual direction.
		unsigned int GetMoveCardinalIndex(DIRECTION_INDEX dir);

	private:
		glm::ivec2 position;
		FACING_INDEX facing = EAST_INDEX;
		STATE state = IDLE;
		float moveSpeed = 0.25f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;
		float turnSpeed = 0.15f;
		float turnCurrent = 0.0f;
		float oldTurn;
		float targetTurn;
		Dungeon* dungeon;
		Object* object;

		bool didMove = false;

		int maxHp = 1;
		int hp = maxHp;

		bool hasRespawnLocation = false;
		glm::ivec2 respawnPosition = { 0,0 };
		FACING_INDEX respawnOrientation = EAST_INDEX;

		DungeonEnemySwitcher* shouldSwitchWith = nullptr;

		string stepSounds[4] = {
			"crawler/sound/load/step1.ogg",
			"crawler/sound/load/step2.ogg",
			"crawler/sound/load/step3.ogg",
			"crawler/sound/load/step4.ogg"
		};
	};
}
