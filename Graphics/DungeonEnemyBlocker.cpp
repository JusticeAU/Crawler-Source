#include "DungeonEnemyBlocker.h"
#include "Dungeon.h"
#include "Object.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "LogUtils.h"
#include "AudioManager.h"

Crawl::DungeonEnemyBlocker::~DungeonEnemyBlocker()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemyBlocker::UpdateTransform()
{
	object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	object->SetLocalRotationZ(orientationEulersReversed[facing]);
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
		PlaySFX(audioSwing);
		bool didDamge = dungeon->DamageAtPosition(position + directions[facing], this, false, Dungeon::DamageType::Blocker);
		if (didDamge) PlaySFX(audioHit);
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/LambertRed.material");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[1] = MaterialManager::GetMaterial("engine/model/materials/LambertRed.material");
		state = State::DownSwing;
		animator->BlendToAnimation(animationDownSwing, 0.0f);
		animator->next->animationSpeedScale = 4.0f;
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
			PlaySFX(audioReturn);
			((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
			((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[1] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
			animator->BlendToAnimation(animationIdle, 0.0f);
			animator->next->animationSpeedScale = 4.0f;
		}
		break;
	}
	}

}

void Crawl::DungeonEnemyBlocker::CheckShouldPrime()
{
	// detect object in front
	// if yes, upswing
	if (!dungeon->CanSee(position, facing))
		return;

	DungeonTile* tile = dungeon->GetTile(position + directions[facing]);
	if (!tile)
		return;

	if (tile->occupied || dungeon->GetMurderinaAtPosition(tile->position))
	{
		LogUtils::Log("Object infront - Transition to Upswing");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/LambertAmber.material");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[1] = MaterialManager::GetMaterial("engine/model/materials/LambertAmber.material");

		state = State::UpSwing;
		PlaySFX(audioRaise);
		animator->BlendToAnimation(animationUpSwing, 0.0f);
		animator->next->animationSpeedScale = 4.0f;
	}
	else
	{
		LogUtils::Log("Nothing infront");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
		((ComponentRenderer*)object->children[0]->GetComponent(Component_Renderer))->materialArray[1] = MaterialManager::GetMaterial("engine/model/materials/LambertBlue.material");
		state = State::Idle;
	}
}

void Crawl::DungeonEnemyBlocker::PlaySFX(string sfx)
{
	AudioManager::PlaySound(sfx, object->GetWorldSpacePosition());
}
