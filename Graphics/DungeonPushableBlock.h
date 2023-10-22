#pragma once
#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{
	class Dungeon;
	class DungeonPushableBlock
	{
	public:
		enum class STATE
		{
			IDLE,
			MOVING,
			FALLING
		};
		~DungeonPushableBlock();
		ivec2 position;
		void UpdateTransform();
		STATE state = STATE::IDLE;

		bool isOnSpikes = false;
		bool isDead = false;
		
		Object* object;
		Dungeon* dungeon;

		// visuals
		float moveSpeed = 0.15f;
		float moveCurrent = 0.0f;
		glm::vec3 oldPosition;
		glm::vec3 targetPosition;

		// Audio
		string sfxName = "crawler/sound/load/sliding_box.wav";

		// falling on spikes
		float fallSpeed = 0.25f;
		float fallCurrent = 0.0f;
		const float fallPosition = -1.33f;

		void MoveTo(ivec2 position, bool snap = false);
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