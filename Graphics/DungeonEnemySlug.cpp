#include "DungeonEnemySlug.h"
#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Object.h"

#include "MathUtils.h"
#include "LogUtils.h"
#include "gtx/easing.hpp"

#include "DungeonPlayer.h"
#include "DungeonEnemyChase.h"

#include "AudioManager.h"

Crawl::DungeonEnemySlug::DungeonEnemySlug()
{
	AudioManager::SetAudioSourceAttentuation(audioMoveSingle, 2, 1);
	AudioManager::SetAudioSourceMinMaxDistance(audioMoveSingle, 1, 10);
}

Crawl::DungeonEnemySlug::~DungeonEnemySlug()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySlug::UpdateTransform()
{
	object->AddLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
}

void Crawl::DungeonEnemySlug::Kill(FACING_INDEX direction)
{
	deathDirection = direction;
	object->SetLocalPosition(targetPosition);
	state = DEAD;
}

void Crawl::DungeonEnemySlug::Update()
{

	object->SetLocalPosition(dungeonPosToObjectScale(position));
	oldPosition = dungeonPosToObjectScale(position);

	if (state == DEAD) return;

	// Slug Path system
	// Get path at current position
	DungeonEnemySlugPath* onPath = dungeon->GetSlugPath(position);
	if (onPath)
	{
		// forward, left, then right, otherwise turn around.
		bool validPath = false;
		for (int i = 0; i < 4; i++)
		{
			FACING_INDEX testDirection = dungeonRotate(facing, slugTurns[i]);
			if ((onPath->maskTraverse & orientationMasksIndex[testDirection]) == orientationMasksIndex[testDirection])
			{
				facing = testDirection;
				validPath = true;
				break;
			}
		}

		DungeonTile* fromTile = dungeon->GetTile(position);
		DungeonTile* toTile;
		if (validPath)
		{
			toTile = dungeon->GetTile(position + directions[facing]);
			positionPrevious = position;
			position += directions[facing];
			//PlayMoveSFX();
		}
		else toTile = fromTile;

		state = MOVING;
		targetPosition = dungeonPosToObjectScale(position);
	}
	else
		LogUtils::Log("Slug is not on a path");

	// Check if player moved through us
	if (dungeon->player->GetPositionPrevious() == position && dungeon->player->GetPosition() == positionPrevious)
		dungeon->player->TakeDamage();

	// Check if a chaser has moved through us
	for (auto& chaser : dungeon->chasers)
	{
		if (dungeon->player->GetPositionPrevious() == chaser->position && dungeon->player->GetPosition() == chaser->positionPrevious)
			chaser->Kill(Dungeon::DamageType::Murderina);
	}
	
	moveCurrent = 0.0f;
	// Damage has been moved to PostUpdate
}

void Crawl::DungeonEnemySlug::UpdateVisuals(float delta)
{
	float turnAmount = delta * spinSpeed * 360.0f;
	switch (state)
	{
	case IDLE:
	{
		object->children[0]->AddLocalRotation({ 0,0,turnAmount });

		break;
	}
	case MOVING:
	{
		turnAmount *= 2.0f;
		moveCurrent += delta;
		float t = moveCurrent / moveSpeed;
		float tEased = glm::sineEaseOut(t);
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			state = IDLE;
		}
		else
		{
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, tEased));
		}
		object->children[0]->AddLocalRotation({ 0,0,turnAmount });
		
		break;
	}
	case DEAD:
	{
		deathCurrent += delta;
		float t = deathCurrent / deathSpeed;
		t = glm::clamp(t, 0.0f, 1.0f);
		vec3 rotation = {0,0,0};
		switch (deathDirection)
		{
		case NORTH_INDEX:
		{
			rotation.x = MathUtils::Lerp(0, 90, t);
			break;
		}
		case EAST_INDEX:
		{
			rotation.y = MathUtils::Lerp(0, -90, t);
			break;
		}
		case SOUTH_INDEX:
		{
			rotation.x = MathUtils::Lerp(0, -90, t);
			break;
		}
		case WEST_INDEX:
		{
			rotation.y = MathUtils::Lerp(0, 90, t);
			break;
		}
		}

		object->SetLocalRotation(rotation);

		if (deathCurrent > deathDeleteTime) shouldDelete = true;
		break;
	}


	if (object->children[0]->localRotation.z > 180) object->children[0]->localRotation.z -= 360;
	}
}

void Crawl::DungeonEnemySlug::PlayMoveSFX()
{
	/*int sfxIndex = rand() % 6;
	AudioManager::PlaySound(audioMove[sfxIndex], object->GetWorldSpacePosition());*/

	AudioManager::PlaySound(audioMoveSingle, object->GetWorldSpacePosition());
}
