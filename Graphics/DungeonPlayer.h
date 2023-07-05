#pragma once
#include "Dungeon.h"
#include "glm.hpp"

class Object;

namespace Crawl
{
	class DungeonPlayer
	{
	public:
		enum FACING
		{
			NORTH,
			EAST,
			SOUTH,
			WEST
		};

		enum DIRECTION
		{
			FORWARD,
			RIGHT,
			BACK,
			LEFT
		};

		enum STATE {
			IDLE,
			MOVING,
			TURNING
		};
		DungeonPlayer();

		void SetDungeon(Dungeon* dungeonPtr) { this->dungeon = dungeonPtr; }
		void SetPlayerObject(Object* objectPtr) { this->object = objectPtr; }

		void Update(float deltaTime);

		// combines our requested direction with our facing direction to return a x/y co-ordinate to move on.
		glm::ivec2 GetMoveCoordinate(DIRECTION dir);

	private:
		Position position;
		FACING facing = EAST;
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

		// this is indexed in to by GetMoveCoordinate;
		glm::ivec2 directions[4] = {
			{0, 1},	// north
			{1, 0},	// east
			{0,-1},	// south
			{-1,0}	// west
		};
	};
}
