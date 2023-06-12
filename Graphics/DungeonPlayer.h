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
		DungeonPlayer();

		void SetDungeon(Dungeon* dungeonPtr) { this->dungeon = dungeonPtr; }
		void SetPlayerObject(Object* objectPtr) { this->object = objectPtr; }

		void Update();

		// combines our requested direction with our facing direction to return a x/y co-ordinate to move on.
		glm::ivec2 GetMoveCoordinate(DIRECTION dir);

	private:
		Position position;
		FACING facing = EAST;
		Dungeon* dungeon;
		Object* object;

		bool didMove = false;

		// this is indexed in to by GetMoveCoordinate;
		glm::ivec2 directions[4] = {
			{0, -1},
			{1, 0},
			{0,1},
			{-1,0}
		};
	};
}
