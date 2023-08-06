#include "DungeonPushableBlock.h"
#include "Object.h"

#include "MathUtils.h"

Crawl::DungeonPushableBlock::~DungeonPushableBlock()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonPushableBlock::UpdateVisuals(float delta)
{
	switch (state)
	{
	case IDLE:
		break;
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
	}
}
