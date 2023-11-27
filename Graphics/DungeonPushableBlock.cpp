#include "DungeonPushableBlock.h"
#include "DungeonHelpers.h"
#include "Object.h"

#include "MathUtils.h"

#include "Dungeon.h"
#include "DungeonSpikes.h"

#include "AudioManager.h"

Crawl::DungeonPushableBlock::~DungeonPushableBlock()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonPushableBlock::UpdateTransform()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
}

void Crawl::DungeonPushableBlock::MoveTo(ivec2 toPosition, bool snap)
{
	oldPosition = dungeonPosToObjectScale(position);
	position = toPosition;
	state = STATE::MOVING;
	targetPosition = dungeonPosToObjectScale(toPosition);
	moveCurrent = 0.0f;
	DungeonSpikes* spikes = dungeon->GetSpikesAtPosition(toPosition);
	if (spikes && !spikes->disabled) // Check if the box is on top of some spikes that havent been disabled.
	{
		isOnSpikes = true;
		spikes->Disable();
	}

	if (snap)
	{
		if (isOnSpikes)
			object->SetLocalPosition({ targetPosition.x, targetPosition.y, fallPosition });
		else
			object->SetLocalPosition(targetPosition);
		state = STATE::IDLE;
	}
	else
		AudioManager::PlaySound(sfxSliding, targetPosition);
}

void Crawl::DungeonPushableBlock::UpdateVisuals(float delta)
{
	switch (state)
	{
	case STATE::IDLE:
		break;
	case STATE::MOVING:
	{
		moveCurrent += delta;
		if (moveCurrent > moveSpeed)
		{
			if (isOnSpikes) state = STATE::FALLING;
			else
			{
				object->SetLocalPosition(targetPosition);
				state = STATE::IDLE;
			}
		}
		else
		{
			float t = moveCurrent / moveSpeed;
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		}
		break;
	}
	case STATE::FALLING:
	{
		fallCurrent += delta;
		if (fallCurrent > fallSpeed)
		{
			object->SetLocalPosition({ targetPosition.x, targetPosition.y, fallPosition });
			state = STATE::IDLE;
		}
		else
		{
			float t = fallCurrent / fallSpeed;
			object->SetLocalPosition({ targetPosition.x, targetPosition.y, glm::sineEaseOut(t) * fallPosition });
		}
		break;
	}
	case STATE::EXPLODING:
	{
		fuse -= delta;
		break;
	}
	}
}

void Crawl::DungeonPushableBlock::Explode(float fuse)
{
	isDead = true;
	state = STATE::EXPLODING;
	this->fuse = fuse;
	DungeonTile* tile = dungeon->GetTile(position);
	if (tile) tile->occupied = false;
}

void Crawl::DungeonPushableBlock::Explode(float fuse, FACING_INDEX fromDirection)
{
	Explode(fuse);
	explodeDirection = fromDirection;

}
