#include "glm.hpp"
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;

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
			TURNING,
			BOUNCING,
			STUN,
			KICKED
		};

		const bool sightReflectsOffMirrors = true;
		~DungeonEnemyChase();
		glm::ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;
		
		bool isDead = false;
		
		STATE state = INACTIVE;
		STATE stateVisual = IDLE;

		// movement
		glm::ivec2 positionWant = { 0,0 };

		void Update();
		void ExecuteMove();
		void ExecuteDamage();
		void UpdateVisuals(float delta);


		// dependencies
		Object* object = nullptr;
		Dungeon* dungeon = nullptr;

		// visuals
		float moveSpeed = 0.25f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;
		float turnSpeed = 0.15f;
		float turnCurrent = 0.0f;
		float oldTurn;
		float targetTurn;
		float bounceSpeed = 0.5;
		float bounceCurrent = 0.0f;
		float kickedSpeed = 0.15f;
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