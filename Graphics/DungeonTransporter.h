#pragma once
#include "DungeonHelpers.h"
#include "DungeonGameManagerEvent.h"
#include "glm.hpp"
#include "serialisation.h"
#include <string>
#include <vector>

using std::string;
using glm::ivec2;

class Object;

namespace Crawl
{
	class DungeonTransporter
	{
	public:
		~DungeonTransporter();

		void ProcessGameManagerInteractions();
		string name = "";
		ivec2 position = { 0,0 };
		unsigned int fromOrientation = NORTH_INDEX;

		string toDungeon = "";
		string toTransporter = "";

		Object* object = nullptr;

		// Level lobby 2 hack stuff
		bool toLobby2 = false;

		// Game Manager Activations
		bool gameManagerInteraction = false;
		std::vector<DungeonGameManagerEvent> gameManagerEvents;
	};

	extern void to_json(ordered_json& j, const DungeonTransporter& object);
	extern void from_json(const ordered_json& j, DungeonTransporter& object);
}

