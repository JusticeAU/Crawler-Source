#include "DungeonEnemyBlocker.h"
#include "Dungeon.h"
#include "Object.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "LogUtils.h"

Crawl::DungeonEnemyBlocker::~DungeonEnemyBlocker()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemyBlocker::Update()
{
	LogUtils::Log("Updating Blocker");
	switch (state)
	{
	case(State::Idle):
	{
		CheckShouldPrime();
		break;
	}
	case(State::UpSwing):
	{
		LogUtils::Log("Down Swinging, attacking infront");
		dungeon->DamageAtPosition(position + directions[facing], this);
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/monster_blocker_downSwing.material");
		state = State::DownSwing;
		break;
	}
	case(State::DownSwing):
	{
		if (rapidAttack)
		{
			LogUtils::Log("Rapid Attack - Checking if should raise sword again.");
			CheckShouldPrime();
		}
		else
		{
			LogUtils::Log("No Rapid Attack - Moving to Idle");
			state = State::Idle;
			((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/monster_blocker_idle.material");
		}
		break;
	}
	}

}

void Crawl::DungeonEnemyBlocker::CheckShouldPrime()
{
	// detect object in front
	// if yes, upswing
	if (!dungeon->HasLineOfSight(position, facing))
		return;

	DungeonTile* tile = dungeon->GetTile(position + directions[facing]);
	if (!tile)
		return;

	if (tile->occupied)
	{
		LogUtils::Log("Object infront - Transition to Upswing");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/monster_blocker_upSwing.material");
		state = State::UpSwing;
	}
	else
	{
		LogUtils::Log("Nothing infront");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/monster_blocker_idle.material");
		state = State::Idle;
	}
}
