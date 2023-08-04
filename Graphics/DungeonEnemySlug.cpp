#include "DungeonEnemySlug.h"
#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Object.h"

#include "MathUtils.h"
#include "LogUtils.h"

Crawl::DungeonEnemySlug::~DungeonEnemySlug()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySlug::Update()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	oldPosition = dungeonPosToObjectScale(position);
	object->SetLocalRotationZ(orientationEulers[facing]);
	oldTurn = orientationEulers[facing];

	// Slug Path system
	// Get path at current position
	DungeonEnemySlugPath* onPath = dungeon->GetSlugPath(position);
	if (onPath)
	{
		// Check there is a path at current position + facing
		DungeonEnemySlugPath* toPath = dungeon->GetSlugPath(position + directions[facing]);
		if (toPath)
		{
			// if yes, got for it
			DungeonTile* fromTile = dungeon->GetTile(position);
			DungeonTile* toTile = dungeon->GetTile(position + directions[facing]);
			fromTile->occupied = false;
			toTile->occupied = true;
			position += directions[facing];
			state = MOVING;
			targetPosition = dungeonPosToObjectScale(position);
			moveCurrent = -0.0f;
			dungeon->DamageAtPosition(position, this);
		}
		else
		{
			// if no, check left/right and then turn
			// check left
			DungeonEnemySlugPath* toTurn = dungeon->GetSlugPath(position + directions[dungeonRotate(facing, -1)]);
			if (toTurn)
			{
				facing = dungeonRotate(facing, -1);
				state = TURNING;
				targetTurn = orientationEulers[facing];
				turnCurrent = 0.0f;
			}
			else
			{
				facing = dungeonRotate(facing, 1);
				state = TURNING;
				targetTurn = orientationEulers[facing];
				turnCurrent = 0.0f;
			}

		}

	}
	else
		LogUtils::Log("Slug is not on a path");



	// Command system
	/*Command nextCommand = commands[commandIndex];  
	switch (nextCommand)
	{
	case Command::TURN_LEFT:
	{
		facing = dungeonRotate(facing, -1);
		state = TURNING;
		targetTurn = orientationEulers[facing];
		turnCurrent = -0.0f;
		NextCommand();
		break;
	}
	case Command::TURN_RIGHT:
	{
		facing = dungeonRotate(facing, 1);
		state = TURNING;
		targetTurn = orientationEulers[facing];
		turnCurrent = -0.0f;
		NextCommand();
		break;
	}
	case Command::WALK_FORWARD:
	{
		if (dungeon->HasLineOfSight(position, facing))
		{
			DungeonTile* tile = dungeon->GetTile(position + directions[facing]);
			if (tile && !tile->occupied)
			{
				dungeon->GetTile(position)->occupied = false;
				tile->occupied = true;
				position += directions[facing];
				NextCommand();
				tile->occupied = true;
				state = MOVING;
				targetPosition = dungeonPosToObjectScale(position);
				moveCurrent = -0.0f;
			}
		}
		break;
	}
	}*/
}

void Crawl::DungeonEnemySlug::NextCommand()
{
	commandIndex += 1;
	if (commandIndex > commands.size() - 1)
		commandIndex = 0;
}

void Crawl::DungeonEnemySlug::UpdateVisuals(float delta)
{
	switch (state)
	{
	case IDLE:
		break;
	case TURNING:
	{
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeed, glm::max(0.0f, turnCurrent));
		if (turnCurrent > turnSpeed)
		{
			object->SetLocalRotationZ(targetTurn);
			state = IDLE;
		}
		else
			object->SetLocalRotationZ(MathUtils::LerpDegrees(oldTurn, targetTurn, t));
		break;
	}
	case MOVING:
	{
		moveCurrent += delta;
		float t = MathUtils::InverseLerp(0, moveSpeed, glm::max(0.0f, moveCurrent));
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			state = IDLE;
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		break;
	}
	case BOUNCING:
	{
		bounceCurrent += delta;
		float t = MathUtils::InverseLerp(0, bounceSpeed, glm::max(0.0f, bounceCurrent));
		if (bounceCurrent > bounceSpeed)
		{
			object->SetLocalPosition(oldPosition);
			state = IDLE;
		}
		else
		{
			if (t > 0.5f)
				t -= (t - 0.5) * 2;

			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		}
		break;
	}
	}
}

const std::string Crawl::DungeonEnemySlug::commandStrings[3] = { "Turn Left", "Turn Right", "Walk Forward" };