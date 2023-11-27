#include "DungeonEnemyBlocker.h"
#include "Dungeon.h"
#include "Object.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "LogUtils.h"
#include "AudioManager.h"

Crawl::DungeonEnemyBlocker::DungeonEnemyBlocker()
{
	// set the audio source configurations
	AudioManager::SetAudioSourceAttentuation(audioRaise, 2, 1);
	AudioManager::SetAudioSourceAttentuation(audioReturn, 2, 1);
	AudioManager::SetAudioSourceAttentuation(audioSwing, 2, 1);
	AudioManager::SetAudioSourceAttentuation(audioHit, 2, 1);

	AudioManager::SetAudioSourceMinMaxDistance(audioRaise, 1, 16);
	AudioManager::SetAudioSourceMinMaxDistance(audioReturn, 1, 16);
	AudioManager::SetAudioSourceMinMaxDistance(audioSwing, 1, 16);
	AudioManager::SetAudioSourceMinMaxDistance(audioHit, 1, 16);
}

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
		state = State::DownSwing;
		animator->StartAnimation(animationDownSwing);
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
			animator->StartAnimation(animationReturnToIdle);
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

	if (tile->occupied || dungeon->GetMurderinaAtPosition(tile->position, false))
	{
		LogUtils::Log("Object infront - Transition to Upswing");
		state = State::UpSwing;
		PlaySFX(audioRaise);
		animator->BlendToAnimation(animationUpSwing, 0.0f);
	}
	else
	{
		LogUtils::Log("Nothing infront");
		state = State::Idle;
	}
}

void Crawl::DungeonEnemyBlocker::PlaySFX(string sfx)
{
	AudioManager::PlaySound(sfx, object->GetWorldSpacePosition());
}
