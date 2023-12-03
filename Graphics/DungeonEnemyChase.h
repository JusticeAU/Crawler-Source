#include "glm.hpp"
#include "DungeonHelpers.h"
#include "serialisation.h"
#include "Dungeon.h"

class Object;
class ComponentRenderer;
class ComponentAnimator;

namespace Crawl
{
	class Dungeon;

	class DungeonEnemyChase
	{
	public:
		enum STATE {
			INACTIVE,
			IDLE,
			MOVING,
			STUN,
			KILLING
		};
		enum class State
		{
			Inactive,
			Active,
			Stunned,
			Dead
		};
		enum class AnimationState
		{
			Inactive,
			Activating,
			Idle,
			Walking,
			Turning,
			Bonked,
			Stunned,
			Dying,
			Killing
		};

		const bool sightReflectsOffMirrors = true;
		~DungeonEnemyChase();
		void Initialise();
		glm::ivec2 position = {0,0};
		glm::ivec2 positionPrevious;
		FACING_INDEX facing = NORTH_INDEX;

		void UpdateTransform();
		
		bool canRemove = false;

		bool isDead = false;
		bool isDeadAnimationIncreased = false;
		Dungeon::DamageType diedTo = Dungeon::DamageType::Chaser;
		string deathAnimationToUse = "";
		
		
		STATE state = INACTIVE;
		STATE stateVisual = INACTIVE;
		
		State gameState = State::Inactive;
		AnimationState animationState = AnimationState::Inactive;
		float animationTime = 0.0f;
		float animationMinStunTime = 0.55f;
		string animationBaseName = "crawler/model/monster_chaser.fbx";
		
		string animationIdle = animationBaseName + "chaser.rig|chaser_idle";
		string animationActivate = animationBaseName + "chaser.rig|chaser_activate";
		
		string animationWalkForward = animationBaseName + "chaser.rig|chaser_walk";
		string animationBonk = animationBaseName + "chaser.rig|chaser_bonk";

		string animationTurnLeft = animationBaseName + "chaser.rig|chaser_turn_left";
		string animationTurnRight = animationBaseName + "chaser.rig|chaser_turn_right";
		bool animationTurnIsRight = false;

		string animationPushFront = animationBaseName + "chaser.rig|chaser_push_front";
		string animationPushLeft = animationBaseName + "chaser.rig|chaser_push_side_left";
		string animationPushBack = animationBaseName + "chaser.rig|chaser_push_back";
		string animationPushRight = animationBaseName + "chaser.rig|chaser_push_side_right";
		string stunAnimationToUse = "";

		string animationDeathBlocker = animationBaseName + "chaser.rig|chaser_death_blocker";
		string animationDeathLaser = animationBaseName + "chaser.rig|chaser_death_laser";
		string animationDeathSpike = animationBaseName + "chaser.rig|chaser_death_spike";

		string animationPlayerKill = animationBaseName + "chaser.rig|chaser_player_kill";

		// movement
		glm::ivec2 positionWant = { 0,0 };

		void Update();
		void ExecuteMove();
		void ExecuteDamage();

		void Activate();
		void Kick(FACING_INDEX inDirection);
		void Bonk();
		void Kill(Dungeon::DamageType damageType);

		void UpdateVisuals(float delta);
		void NewAnimationState(AnimationState state, bool blend = false);

		void PlaySFXActivate();

		// dependencies
		Dungeon* dungeon = nullptr;
		Object* object = nullptr;
		ComponentRenderer* renderer = nullptr;
		ComponentAnimator* animator = nullptr;

		// visuals
		float bounceSpeed = 0.5;
		float bounceCurrent = 0.0f;

		// Audio
		int audioActivateQuantity = 13;
		string audioActivateSFX[13] = {
			"crawler/sound/load/chaser/activate_1.wav",
			"crawler/sound/load/chaser/activate_2.wav",
			"crawler/sound/load/chaser/activate_3.wav",
			"crawler/sound/load/chaser/activate_4.wav",
			"crawler/sound/load/chaser/activate_5.wav",
			"crawler/sound/load/chaser/activate_6.wav",
			"crawler/sound/load/chaser/activate_7.wav",
			"crawler/sound/load/chaser/activate_8.wav",
			"crawler/sound/load/chaser/activate_9.wav",
			"crawler/sound/load/chaser/activate_10.wav",
			"crawler/sound/load/chaser/activate_11.wav",
			"crawler/sound/load/chaser/activate_12.wav",
			"crawler/sound/load/chaser/activate_13.wav"
		};
	};

	static void to_json(ordered_json& j, const DungeonEnemyChase& object)
	{
		j = { {"position", object.position }, {"facing", object.facing }, {"state", object.state } };
	}

	static void from_json(const ordered_json& j, DungeonEnemyChase& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		if(j.contains("state"))
			j.at("state").get_to(object.state);
	}
}