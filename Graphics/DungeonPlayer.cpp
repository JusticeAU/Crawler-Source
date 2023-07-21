#include "DungeonPlayer.h"
#include "Input.h"
#include "Scene.h"
#include "MathUtils.h"
#include "LogUtils.h"

Crawl::DungeonPlayer::DungeonPlayer()
{
	// TODO better inject / handle this better.
	object = Scene::s_instance->objects[1];
}

void Crawl::DungeonPlayer::Update(float deltaTime)
{
	// damage player for testing
	if (Input::Keyboard(GLFW_KEY_SPACE).Down())
		hp -= 1;

	if (state == IDLE)
	{
		if (hp == 0)
		{
			LogUtils::Log("Died. Resetting & Respawning");
			dungeon->RebuildFromSerialised();
			Respawn();
			return;
		}

		glm::ivec2 coordinate = { 0, 0 };
		glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

		// Test Object Picking stuffo
		if (Input::Mouse(0).Down())
			Scene::RequestObjectSelection();

		if (Scene::s_instance->objectPickedID != 0)
		{
			dungeon->DoInteractable(Scene::s_instance->objectPickedID);
			Scene::s_instance->objectPickedID = 0;
			dungeon->Update(); // this should move in to a big order of events process somewhere.
		}

		int index = -1;
		if (Input::Keyboard(GLFW_KEY_W).Pressed())
			index = GetMoveCardinalIndex(FORWARD_INDEX);
		if (Input::Keyboard(GLFW_KEY_S).Pressed())
			index = GetMoveCardinalIndex(BACK_INDEX);
		if (Input::Keyboard(GLFW_KEY_A).Pressed())
			index = GetMoveCardinalIndex(LEFT_INDEX);
		if (Input::Keyboard(GLFW_KEY_D).Pressed())
			index = GetMoveCardinalIndex(RIGHT_INDEX);

		ivec2 oldPlayerCoordinate = position;
		if (index != -1)
		{
			if (dungeon->CanMove(position, index))
			{
				position += directions[index];
				dungeon->GetTile(position)->occupied = true;
				didMove = true;
			}
		}

		if (didMove)
		{
			dungeon->GetTile(oldPlayerCoordinate)->occupied = false;
			state = MOVING;
			oldPosition = object->localPosition;
			targetPosition = { position.x * Crawl::DUNGEON_GRID_SCALE, position.y * Crawl::DUNGEON_GRID_SCALE , 0 };
			moveCurrent = 0.0f;
			didMove = false;
			return;
		}

		// Turning
		if (Input::Keyboard(GLFW_KEY_E).Pressed())
		{
			int faceInt = (int)facing;
			faceInt++;
			if (faceInt == 4)
				faceInt = 0;
			facing = (FACING_INDEX)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z - 90;
		}
		if (Input::Keyboard(GLFW_KEY_Q).Pressed())
		{
			int faceInt = (int)facing;
			faceInt--;
			if (faceInt == -1)
				faceInt = 3;
			facing = (FACING_INDEX)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z + 90;
		}
	}
	else if (state == MOVING)
	{
		moveCurrent += deltaTime;
		float t = MathUtils::InverseLerp(0, moveSpeed, moveCurrent);
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			state = IDLE;
			dungeon->Update();  // this should move in to a big order of events process somewhere.
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
	}
	else if (state == TURNING)
	{
		turnCurrent += deltaTime;
		float t = MathUtils::InverseLerp(0, turnSpeed, turnCurrent);
		if (turnCurrent > turnSpeed)
		{
			object->SetLocalRotationZ(targetTurn);
			state = IDLE;
		}
		else
			object->SetLocalRotationZ(MathUtils::Lerp(oldTurn, targetTurn, t));
	}
}

void Crawl::DungeonPlayer::Teleport(ivec2 position)
{
	state = IDLE;
	this->position = position;
	targetPosition = { position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 };
	object->SetLocalPosition(targetPosition);
}

void Crawl::DungeonPlayer::Orient(FACING_INDEX facing)
{
	this->facing = facing;
	object->SetLocalRotationZ(orientationEulers[facing]);
}

void Crawl::DungeonPlayer::SetRespawn(ivec2 position, FACING_INDEX orientation)
{
	hasRespawnLocation = true;
	this->respawnPosition = position;
	respawnOrientation = orientation;
}

void Crawl::DungeonPlayer::Respawn()
{
	dungeon->RebuildFromSerialised();
	if (hasRespawnLocation)
	{
		Teleport(respawnPosition);
		Orient(respawnOrientation);
	}
	else
	{
		Teleport(dungeon->defaultPlayerStartPosition);
		Orient(dungeon->defaultPlayerStartOrientation);
	}
	hp = maxHp;
}

void Crawl::DungeonPlayer::TakeDamage()
{
	hp -= 1;
}

// Take the requested direction and offset by the direction we're facing, check for overflow, then index in to the directions array.
unsigned int Crawl::DungeonPlayer::GetMoveCardinalIndex(DIRECTION_INDEX dir)
{
	unsigned int index = (int)dir;
	index += (int)facing;
	if (index >= 4)
		index -= 4;

	return index;
}
