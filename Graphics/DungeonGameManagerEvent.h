#pragma once
#include "DungeonGameManager.h"

namespace Crawl
{
	class DungeonGameManagerEvent
	{
	public:
		enum class Type
		{
			Door,
			Light,
			Lock
		};

		std::string typeString[3] = { "Door", "Light", "Lock" };
		std::string doorNames[8] = { "G  North 1","G  South 1","L2 North 1", "L2 North 2", "L2 North 3","L2 South 1", "L2 South 2", "L2 South 3" };
		std::string doorStateString[3] = { "Open", "Closed", "Barricaded" };
		bool DrawGUIInternal();

		Type type;
		int id;
		int status;
	};

	extern void to_json(nlohmann::ordered_json& j, const DungeonGameManagerEvent& gme);
	extern void from_json(const nlohmann::ordered_json& j, DungeonGameManagerEvent& gme);
}