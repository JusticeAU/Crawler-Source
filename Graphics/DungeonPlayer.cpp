#include "DungeonPlayer.h"
#include "Input.h"
#include "Scene.h"
#include "MathUtils.h"
#include "LogUtils.h"
#include "AudioManager.h"

Crawl::DungeonPlayer::DungeonPlayer()
{
	// TODO better inject / handle this better.
	object = Scene::s_instance->objects[1];
}

// Returns true if the player made a game-state changing action
bool Crawl::DungeonPlayer::Update(float deltaTime)
{
	// damage player for testing
	/*if (Input::Keyboard(GLFW_KEY_SPACE).Down())
		hp -= 1;*/

	if (state == IDLE)
	{
		if (hp <= 0)
		{
			LogUtils::Log("Died. Resetting & Respawning");
			dungeon->RebuildFromSerialised();
			Respawn();
			return false;
		}

		glm::ivec2 coordinate = { 0, 0 };
		glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

		if (Input::Keyboard(GLFW_KEY_LEFT_ALT).Down())
			return true;

		if (Input::Keyboard(GLFW_KEY_SPACE).Down() && dungeon->playerCanKickBox)
		{
			if (dungeon->DoKick(position, facing))
				return true;
		}

		if ((Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Down() || Input::Mouse(1).Down()) && dungeon->playerHasKnife)
		{
			if (dungeon->HasLineOfSight(position, facing))
			{
				LogUtils::Log("Stab!");
				dungeon->DamageAtPosition(position + directions[facing], this, true);
				return true;
			}
		}

		// Test Object Picking stuffo
		if (Input::Mouse(0).Down())
			Scene::RequestObjectSelection();

		if (Scene::s_instance->objectPickedID != 0)
		{
			unsigned int picked = Scene::s_instance->objectPickedID;
			Scene::s_instance->objectPickedID = 0;
			if (dungeon->DoInteractable(picked))
			{
				if (!dungeon->playerInteractIsFree)
					return true;
			}
		}

		int index = -1;
		if (Input::Keyboard(GLFW_KEY_W).Down())
			index = GetMoveCardinalIndex(FORWARD_INDEX);
		if (Input::Keyboard(GLFW_KEY_S).Down())
			index = GetMoveCardinalIndex(BACK_INDEX);
		if (Input::Keyboard(GLFW_KEY_A).Down())
			index = GetMoveCardinalIndex(LEFT_INDEX);
		if (Input::Keyboard(GLFW_KEY_D).Down())
			index = GetMoveCardinalIndex(RIGHT_INDEX);

		ivec2 oldPlayerCoordinate = position;
		if (index != -1)
		{
			if (dungeon->PlayerCanMove(position, index))
			{
				AudioManager::PlaySound(stepSounds[rand() % 4]);
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
			return true;
		}

		// Turning
		if (Input::Keyboard(GLFW_KEY_E).Down())
		{
			AudioManager::PlaySound("crawler/sound/load/turn.wav");
			int faceInt = (int)facing;
			faceInt++;
			if (faceInt == 4)
				faceInt = 0;
			facing = (FACING_INDEX)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z - 90;
			if (!dungeon->playerTurnIsFree)
				return true;
		}
		if (Input::Keyboard(GLFW_KEY_Q).Down())
		{
			AudioManager::PlaySound("crawler/sound/load/turn.wav");
			int faceInt = (int)facing;
			faceInt--;
			if (faceInt == -1)
				faceInt = 3;
			facing = (FACING_INDEX)faceInt;
			state = TURNING;
			turnCurrent = 0.0f;
			oldTurn = object->localRotation.z;
			targetTurn = object->localRotation.z + 90;
			if (!dungeon->playerTurnIsFree)
				return true;
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

	return false;
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
	AudioManager::PlaySound("crawler/sound/load/start.wav");
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
