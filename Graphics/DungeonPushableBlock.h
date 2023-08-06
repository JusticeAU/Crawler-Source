#pragma once
#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{
	class DungeonPushableBlock
	{
	public:
		enum STATE
		{
			IDLE,
			MOVING
		};
		~DungeonPushableBlock();
		ivec2 position;
		STATE state = IDLE;

		bool isDead = false;
		
		Object* object;

		// visuals
		float moveSpeed = 0.15f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;

		void UpdateVisuals(float delta);
	private:
	};

	static void to_json(ordered_json& j, const DungeonPushableBlock& object)
	{
		j = { {"position", object.position } };
	}

	static void from_json(const ordered_json& j, DungeonPushableBlock& object)
	{
		j.at("position").get_to(object.position);
	}
}