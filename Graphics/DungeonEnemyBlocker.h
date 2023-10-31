#pragma once
#include "glm.hpp"
#include "DungeonHelpers.h"
#include "serialisation.h"
#include "ComponentAnimator.h"

class Object;

using glm::ivec2;

namespace Crawl
{
	class Dungeon;
	class DungeonEnemyBlocker
	{
		enum class State
		{
			Idle,
			UpSwing,
			DownSwing
		};
	public:
		~DungeonEnemyBlocker();
		ivec2 position = { 0, 0 };
		FACING_INDEX facing = EAST_INDEX;

		void UpdateTransform();

		bool rapidAttack = false;
		
		State state = State::Idle;

		Dungeon* dungeon = nullptr;
		Object* object = nullptr;
		ComponentAnimator* animator = nullptr;

		string animationBaseName = "crawler/model/monster_blocker.fbx";
		string animationUpSwing = animationBaseName + "armature|ready.pose";
		string animationDownSwing = animationBaseName + "armature|attack.pose";
		string animationIdle = animationBaseName + "armature|idle.pose";


		void Update();
		void CheckShouldPrime();
	};

	static void to_json(ordered_json& j, const DungeonEnemyBlocker& object)
	{
		j = { {"position", object.position }, {"facing", object.facing }, {"rapidAttack", object.rapidAttack } };
	}

	static void from_json(const ordered_json& j, DungeonEnemyBlocker& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		j.at("rapidAttack").get_to(object.rapidAttack);
	}
}