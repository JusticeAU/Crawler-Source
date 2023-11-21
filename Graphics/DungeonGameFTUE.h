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
		ResetHold,
		Wait,
		Look,
		Push,
		Door1,
		Door2,
		ChaserPush,
		Run,
		Key,
		FailedLevel
	};
	DungeonGameFTUE(Texture* tex, FTUEType t) { texture = tex, type = t; }
	Texture* texture;
	FTUEType type;
};