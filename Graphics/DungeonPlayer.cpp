#include "DungeonPlayer.h"
#include "Input.h"
#include "Scene.h"
#include "MathUtils.h"
#include "LogUtils.h"
#include "AudioManager.h"
#include "ComponentAnimator.h"

#include "DungeonEnemySwitcher.h"
#include "DungeonCheckpoint.h"

Crawl::DungeonPlayer::DungeonPlayer()
{
	// Initialise the player Scene Object;
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk("crawler/object/player.object"));
	object->children[2]->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/viewmodel_hands.object"));
	animator = (ComponentAnimator*)object->children[2]->children[0]->GetComponent(Component_Animator);
	
	// Hack the light in
	Scene::s_instance->m_pointLights.push_back(Light());
	Scene::s_instance->m_pointLights[0].position.z = 1.6;
	Scene::s_instance->m_pointLights[0].colour.x = 0.7009804248809814;
	Scene::s_instance->m_pointLights[0].colour.y = 0.36686262488365173;
	Scene::s_instance->m_pointLights[0].colour.z = 0.10308541357517242;
	Scene::s_instance->m_pointLights[0].intensity = 25.0f;
}

// Returns true if the player made a game-state changing action
bool Crawl::DungeonPlayer::Update(float deltaTime)
{
	if (state == IDLE)
	{
		if (Input::Keyboard(GLFW_KEY_R).Down())
			Respawn();

		// All these checks should move to a turn processors state machine.
		if (hp <= 0)
		{
			LogUtils::Log("Died. Resetting & Respawning");
			dungeon->RebuildDungeonFromSerialised(dungeon->serialised);
			Respawn();
			return false;
		}

		if (dungeon->GetCheckpointAt(position))
		{
			DungeonCheckpoint* newCheckpoint = dungeon->GetCheckpointAt(position);
			if (!newCheckpoint->activated)
			{
				LogUtils::Log("You activated a checkpoint");
				newCheckpoint->SetCheckpoint(this);
			}
		}

		if (shouldSwitchWith) // fairly hacky
		{
			AudioManager::PlaySound("crawler/sound/load/switcher.wav");
			shouldSwitchWith->SwapWithPlayer();
			shouldSwitchWith = nullptr;
		}

		// To here.

		glm::ivec2 coordinate = { 0, 0 };
		glm::ivec2 coordinateUnchanged = { 0, 0 }; // TO DO this sucks

		if (Input::Keyboard(GLFW_KEY_LEFT_ALT).Down())
			return true;

		if (Input::Keyboard(GLFW_KEY_SPACE).Down() && dungeon->playerCanKickBox)
		{
			if (dungeon->DoKick(position, facing))
			{
				animator->BlendToAnimation(animationNamePush, 0.1f);
				return true;
			}
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
				animator->BlendToAnimation(animationNamePush, 0.1f);
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

		// Rotators

		if (Input::Keyboard(GLFW_KEY_Z).Down())
		{
			DungeonMirror* mirror = dungeon->GetMirrorAt(position + directions[facing]);
			if (mirror)
			{
				dungeon->RotateMirror(mirror, 1);
				return true;
			}
		}
		if (Input::Keyboard(GLFW_KEY_C).Down())
		{
			DungeonMirror* mirror = dungeon->GetMirrorAt(position + directions[facing]);
			if (mirror)
			{
				dungeon->RotateMirror(mirror, -1);
				return true;
			}
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

	// Update the position of the light on the player after we have moved.
	object->RecalculateTransforms();
	Scene::s_instance->m_pointLights[0].position = object->GetWorldSpacePosition(); // hacky!!!
	Scene::s_instance->m_pointLights[0].position.z += 1.6f;
	Scene::s_instance->m_pointLights[0].position += dungeonPosToObjectScale(directions[facing]) * 0.2f;

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
	if (checkpointExists)
	{
		dungeon->RebuildDungeonFromSerialised(checkpointSerialised);
		Teleport(checkpointPosition);
		Orient(checkpointFacing);
	}
	else
	{
		dungeon->RebuildDungeonFromSerialised(dungeon->serialised);
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
	}

	hp = maxHp;
	shouldSwitchWith = nullptr;
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
