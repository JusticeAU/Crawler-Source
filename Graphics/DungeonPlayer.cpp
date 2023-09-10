#include "DungeonPlayer.h"
#include "Input.h"
#include "Window.h"
#include "Scene.h"
#include "MathUtils.h"
#include "LogUtils.h"
#include "AudioManager.h"
#include "ComponentAnimator.h"

#include "DungeonEnemySwitcher.h"
#include "DungeonCheckpoint.h"
#include "DungeonStairs.h"

#include "gtx/spline.hpp"
#include "gtx/easing.hpp"

Crawl::DungeonPlayer::DungeonPlayer()
{
	// Initialise the player Scene Object;
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk("crawler/object/player.object"));
	object->children[0]->children[0]->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/viewmodel_hands.object"));
	animator = (ComponentAnimator*)object->children[0]->children[0]->children[0]->GetComponent(Component_Animator);
	objectView = object->children[0];

	// load the lobby second level
	lobbyLevel2Dungeon = new Dungeon(true);
	lobbyLevel2Dungeon->Load("crawler/dungeon/lobby2.dungeon");
	lobbyLevel2Dungeon->player = this;
}

void Crawl::DungeonPlayer::SetDungeon(Dungeon* dungeonPtr)
{
	dungeon = dungeonPtr;
	currentDungeon = dungeon;
}

// Returns true if the player made a game-state changing action
bool Crawl::DungeonPlayer::Update(float deltaTime)
{
	if (state == IDLE)
	{
		if (UpdateStateIdle(deltaTime))
			return true;
	}
	else if (state == MOVING)
	{
		UpdateStateMoving(deltaTime);
	}
	else if (state == TURNING)
	{
		UpdateStateTurning(deltaTime);
	}
	else if (state == STAIRBEARS)
	{
		UpdateStateStairs(deltaTime);
	}

	return false;
}

bool Crawl::DungeonPlayer::UpdateStateIdle(float delta)
{
	if (Input::Mouse(1).Down()) Window::GetWindow()->SetMouseCursorHidden(true);
	if (Input::Mouse(1).Pressed())
	{
		vec2 mouseDelta = -Input::GetMouseDelta() * lookSpeed;
		objectView->AddLocalRotation({ mouseDelta.y, 0, mouseDelta.x });
		objectView->localRotation.x = glm::clamp(objectView->localRotation.x, -lookMaxY, lookMaxY);
		objectView->localRotation.z = glm::clamp(objectView->localRotation.z, -lookMaxZ, lookMaxZ);
		return false;
	}

	if (Input::Mouse(1).Up())
	{
		Window::GetWindow()->SetMouseCursorHidden(false);
		lookReturnFrom = objectView->localRotation;
		lookReturnTimeCurrent = 0.0f;
	}
	if (lookReturnTimeCurrent < lookReturnTimeTotal)
	{
		float t = glm::clamp(lookReturnTimeCurrent / lookReturnTimeTotal, 0.0f, 1.0f);
		
		vec3 newRotation(0, 0, 0);
		newRotation.x = MathUtils::Lerp(lookReturnFrom.x, 0.0f, MathUtils::EaseOutBounceSubtle(t));
		newRotation.z = MathUtils::Lerp(lookReturnFrom.z, 0.0f, MathUtils::EaseOutBounceSubtle(t));
		objectView->SetLocalRotation(newRotation);

		lookReturnTimeCurrent += delta;

		return false; // disallow movement whilst this is resetting
	}

	if (Input::Keyboard(GLFW_KEY_SPACE).Down() && currentDungeon->playerCanKickBox)
	{
		if (currentDungeon->DoKick(position, facing))
		{
			animator->BlendToAnimation(animationNamePush, 0.1f);
			return true;
		}
	}


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

	if (currentDungeon->GetCheckpointAt(position))
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

	// Old Stab mechanic
	/*if ((Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Down() || Input::Mouse(1).Down()) && dungeon->playerHasKnife)
	{
		if (dungeon->CanSee(position, facing))
		{
			LogUtils::Log("Stab!");
			dungeon->DamageAtPosition(position + directions[facing], this, true);
			return true;
		}
	}*/

	// Activating Objects
	// Via Mouse
	if (Input::Mouse(0).Down())
		Scene::RequestObjectSelection();

	if (Scene::s_instance->objectPickedID != 0)
	{
		unsigned int picked = Scene::s_instance->objectPickedID;
		Scene::s_instance->objectPickedID = 0;
		if (currentDungeon->DoInteractable(picked))
		{
			animator->BlendToAnimation(animationNamePush, 0.1f);
			if (!currentDungeon->playerInteractIsFree)
				return true;
		}
	}
	
	// Via Interact Key
	if (Input::Keyboard(GLFW_KEY_SPACE).Down())
	{
		if (currentDungeon->DoInteractable(position, facing))
		{
			animator->BlendToAnimation(animationNamePush, 0.1f);
			if (!currentDungeon->playerInteractIsFree)
				return true;
		}
	}

	ivec2 oldPlayerCoordinate = position;
	if (IsMoveDown() || IsMovePressedLongEnough(delta))
	{
		int index = GetMoveIndex();

		if (index != -1)
		{
			// Check stairs in this direction
			if (currentDungeon->ShouldActivateStairs(position, (FACING_INDEX)index))
			{
				DungeonTile* oldTile = currentDungeon->GetTile(position);
				if (oldTile) oldTile->occupied = false;
				state = STAIRBEARS;
				position = activateStairs->endPosition;
				stairTimeCurrent = 0.0f;
				if (activateStairs->up)
				{
					isOnLobbyLevel2 = true;
					currentDungeon = lobbyLevel2Dungeon;
					playerZPosition = lobbyLevel2Floor;
				}
				else
				{
					isOnLobbyLevel2 = false;
					currentDungeon = dungeon;
					playerZPosition = 0.0f;
				}
				return false;
			}
			else if (currentDungeon->PlayerCanMove(position, index))
			{
				AudioManager::PlaySound(stepSounds[rand() % 4]);
				position += directions[index];
				currentDungeon->GetTile(position)->occupied = true;
				didMove = true;
			}
		}
	}

	if (didMove)
	{
		currentDungeon->GetTile(oldPlayerCoordinate)->occupied = false;
		state = MOVING;
		oldPosition = object->localPosition;
		targetPosition = { position.x * Crawl::DUNGEON_GRID_SCALE, position.y * Crawl::DUNGEON_GRID_SCALE , playerZPosition };
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

	// Rotators - not in use
	/*if (Input::Keyboard(GLFW_KEY_Z).Down())
	{
		DungeonMirror* mirror = currentDungeon->GetMirrorAt(position + directions[facing]);
		if (mirror)
		{
			dungeon->RotateMirror(mirror, 1);
			return true;
		}
	}
	if (Input::Keyboard(GLFW_KEY_C).Down())
	{
		DungeonMirror* mirror = currentDungeon->GetMirrorAt(position + directions[facing]);
		if (mirror)
		{
			currentDungeon->RotateMirror(mirror, -1);
			return true;
		}
	}*/

	return false;
}

bool Crawl::DungeonPlayer::IsMoveDown()
{
	return
		Input::Keyboard(GLFW_KEY_W).Down() ||
		Input::Keyboard(GLFW_KEY_S).Down() ||
		Input::Keyboard(GLFW_KEY_A).Down() ||
		Input::Keyboard(GLFW_KEY_D).Down();
}

bool Crawl::DungeonPlayer::IsMovePressedLongEnough(float delta)
{
	if (Input::Keyboard(GLFW_KEY_W).Pressed() ||
		Input::Keyboard(GLFW_KEY_S).Pressed() ||
		Input::Keyboard(GLFW_KEY_A).Pressed() ||
		Input::Keyboard(GLFW_KEY_D).Pressed())
	{
		moveDelayCurrent += delta;
		if (moveDelayCurrent > moveDelay)
		{
			moveDelayCurrent = 0.0f;
			return true;
		}
	}
	else moveDelayCurrent = 0.0f;

	return false;
}

int Crawl::DungeonPlayer::GetMoveIndex()
{
	int index = -1;
	if (Input::Keyboard(GLFW_KEY_W).Pressed())
		index = GetMoveCardinalIndex(FORWARD_INDEX);
	if (Input::Keyboard(GLFW_KEY_S).Pressed())
		index = GetMoveCardinalIndex(BACK_INDEX);
	if (Input::Keyboard(GLFW_KEY_A).Pressed())
		index = GetMoveCardinalIndex(LEFT_INDEX);
	if (Input::Keyboard(GLFW_KEY_D).Pressed())
		index = GetMoveCardinalIndex(RIGHT_INDEX);
	return index;
}

bool Crawl::DungeonPlayer::UpdateStateMoving(float delta)
{
	moveCurrent += delta;
	float t = MathUtils::InverseLerp(0, moveSpeed, moveCurrent);
	if (moveCurrent > moveSpeed)
	{
		object->SetLocalPosition(targetPosition);
		state = IDLE;
	}
	else
		object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, glm::sineEaseOut(t)));

	return false;
}

bool Crawl::DungeonPlayer::UpdateStateTurning(float delta)
{
	float t = MathUtils::InverseLerp(0, turnSpeed, turnCurrent);
	if (turnCurrent > turnSpeed)
	{
		object->SetLocalRotationZ(targetTurn);
		state = IDLE;
	}
	else
	{
		object->SetLocalRotationZ(MathUtils::Lerp(oldTurn, targetTurn, MathUtils::EaseOutBounceSubtle(t)));
	}

	turnCurrent += delta;
	return false;
}

bool Crawl::DungeonPlayer::UpdateStateStairs(float delta)
{
	// If we're not facing the stairs, do that first.
	if (!facingStairs)
	{
		// We need to turn towards the enter direction
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeed * 2, turnCurrent);
		if (turnCurrent > turnSpeed * 2)
		{
			object->SetLocalRotationZ(targetTurn);
			facingStairs = true;
		}
		else
			object->SetLocalRotationZ(MathUtils::LerpDegrees(oldTurn, targetTurn, MathUtils::EaseOutBounceSubtle(t)));

	}
	else
	{
		stairTimeCurrent += delta;
		if (stairTimeCurrent > stairTimeTotal)
		{
			state = IDLE;
			facing = activateStairs->directionEnd;
			object->SetLocalPosition(activateStairs->endWorldPosition);
			object->SetLocalRotationZ(orientationEulers[activateStairs->directionEnd]);
			activateStairs = nullptr;
		}
		else
		{
			float t = stairTimeCurrent / stairTimeTotal;
			float moveEase = t;
			float lookEase = glm::sineEaseInOut(t);
			object->SetLocalPosition(glm::hermite(activateStairs->startWorldPosition, activateStairs->startOffset, activateStairs->endWorldPosition, activateStairs->endOffset, moveEase));
			object->SetLocalRotationZ(MathUtils::LerpDegrees(orientationEulers[activateStairs->directionStart], orientationEulers[activateStairs->directionEnd], lookEase));
		}
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

void Crawl::DungeonPlayer::ClearRespawn()
{
	hasRespawnLocation = false;
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
		dungeon->ResetDungeon();
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
	currentDungeon = dungeon;
	isOnLobbyLevel2 = false;
	playerZPosition = 0.0f;
}

void Crawl::DungeonPlayer::MakeCheckpoint()
{
	checkpointSerialised = dungeon->GetDungeonSerialised();
	checkpointPosition = position;
	checkpointFacing = facing;
	checkpointExists = true;
}

void Crawl::DungeonPlayer::ClearCheckpoint()
{
	checkpointSerialised.clear();
	checkpointExists = false;
}

void Crawl::DungeonPlayer::TakeDamage()
{
	hp -= 1;
}

void Crawl::DungeonPlayer::SetShouldActivateStairs(DungeonStairs* stairs)
{
	activateStairs = stairs;
	if (facing != activateStairs->directionStart)
	{
		facingStairs = false;
		turnCurrent = 0.0f;
		oldTurn = object->localRotation.z;
		targetTurn = orientationEulers[activateStairs->directionStart];
	}
	else
		facingStairs = true;
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