#pragma once
#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "glm.hpp"

class Object;
class ComponentAnimator;

namespace Crawl
{
	class DungeonPlayer
	{
	public:

		enum STATE {
			IDLE,
			MOVING,
			TURNING,
			STAIRBEARS
		};
		DungeonPlayer();

		void SetDungeon(Dungeon* dungeonPtr);
		void SetPlayerObject(Object* objectPtr) { this->object = objectPtr; }

		bool Update(float deltaTime);
		bool UpdateStateIdle(float delta);
		bool UpdateStateMoving(float delta);
		bool UpdateStateTurning(float delta);
		bool UpdateStateStairs(float delta);

		ivec2 GetPosition() { return position; }
		FACING_INDEX GetOrientation() { return facing; }
		void Teleport(ivec2 position);
		void Orient(FACING_INDEX facing);
		void SetRespawn(ivec2 position, FACING_INDEX orientation);
		void ClearRespawn();
		void Respawn();

		void MakeCheckpoint();
		void ClearCheckpoint();

		void TakeDamage();

		void SetShouldSwitchWith(DungeonEnemySwitcher* switcher) { shouldSwitchWith = switcher; }
		void SetShouldActivateStairs(DungeonStairs* stairs);

		// combines our requested direction with our facing direction to return the actual direction.
		unsigned int GetMoveCardinalIndex(DIRECTION_INDEX dir);

	private:
		glm::ivec2 position;
		FACING_INDEX facing = EAST_INDEX;
		
		STATE state = IDLE;
		float lookSpeed = 0.1f;
		float lookMaxY = 45.0f;
		float lookMaxZ = 170.0f;
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
		Object* objectView;
		ComponentAnimator* animator;

		bool didMove = false;

		int maxHp = 1;
		int hp = maxHp;

		bool hasRespawnLocation = false;
		glm::ivec2 respawnPosition = { 0,0 };
		FACING_INDEX respawnOrientation = EAST_INDEX;

		DungeonEnemySwitcher* shouldSwitchWith = nullptr;
		DungeonStairs* activateStairs = nullptr;

		// Checkpointing
	public:
		bool checkpointExists = false;
		ordered_json checkpointSerialised;
		glm::ivec2 checkpointPosition = { 0,0 };
		FACING_INDEX checkpointFacing = NORTH_INDEX;
	
	protected:
		string stepSounds[4] = {
			"crawler/sound/load/step1.ogg",
			"crawler/sound/load/step2.ogg",
			"crawler/sound/load/step3.ogg",
			"crawler/sound/load/step4.ogg"
		};

		string animationNamePush = "crawler/model/viewmodel_hands.fbxarmature|armatureaction";

		// Lobby Scene Stuff
		bool isOnLobbyLevel2 = false;
		Dungeon* lobbyLevel2Dungeon = nullptr;
		Dungeon* currentDungeon = nullptr;
		
		float lobbyLevel2Floor = 3.0f;
		float stairTimeTotal = 3.0f;
		float stairTimeCurrent = 0.0f;
		float playerZPosition = 0.0f;
		bool facingStairs = false;
	};
}
