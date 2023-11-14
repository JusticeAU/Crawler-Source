#pragma once
#include "glm.hpp"
#include "DungeonHelpers.h"
#include "serialisation.h"

using glm::ivec2;

class Object;
class ComponentRenderer;

namespace Crawl
{
	class Dungeon;

	class DungeonShootLaser
	{
	public:
		DungeonShootLaser();
		~DungeonShootLaser();
		ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;
		
		void UpdateTransform();

		unsigned int id = 0;
		bool firesProjectile = false;
		bool detectsLineOfSight = true;
		bool firesImmediately = false;
		
		unsigned int activateID = -1;

		Object* object;
		Object* jawObject;
		ComponentRenderer* renderer;
		const float jawOpenAngle = 45.0f;
		Dungeon* dungeon = nullptr;
		ivec2 targetPosition = { 0,0 };
		void* thingAtTargetPosition = nullptr;
		bool primed = false;
		unsigned int turnPrimed = 0;
		void Update(); // check if something is in line of sight or fire if primed
		void Activate();
		void Prime();
		void Fire();

		void* AcquireTarget(ivec2& positionOut);
		void SetInitialTarget();

		string audioPrime = "crawler/sound/load/laser_prime.wav";
		string audioShoot = "crawler/sound/load/laser_shoot.wav";
	};

	static void to_json(ordered_json& j, const DungeonShootLaser& object)
	{
		j = { {"id", object.id}, {"position", object.position }, {"facing", object.facing}, {"detectsLineOfSight", object.detectsLineOfSight }, {"firesProjectile", object.firesProjectile }, {"firesImmediately", object.firesImmediately }, {"activateID", object.activateID } };
	}

	static void from_json(const ordered_json& j, DungeonShootLaser& object)
	{
		j.at("id").get_to(object.id);
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		if (j.contains("detectsLineOfSight"))
			j.at("detectsLineOfSight").get_to(object.detectsLineOfSight);
		if (j.contains("firesProjectile"))
			j.at("firesProjectile").get_to(object.firesProjectile);
		if (j.contains("firesImmediately"))
			j.at("firesImmediately").get_to(object.firesImmediately);
		if(j.contains("activateID"))
			j.at("activateID").get_to(object.activateID);
	}
}