#pragma once

#include <string>

class Texture;

using std::string;

struct DungeonGameFTUE
{
	enum class FTUEType
	{
		Turn,
		Move,
		Interact,
		Reset,
		Wait,
		Look,
		Push
	};
	DungeonGameFTUE(Texture* tex, FTUEType t) { texture = tex, type = t; }
	Texture* texture;
	FTUEType type;
};