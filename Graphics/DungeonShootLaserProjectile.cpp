#include "DungeonShootLaserProjectile.h"
#include "Dungeon.h"
#include "Object.h"

Crawl::DungeonShootLaserProjectile::~DungeonShootLaserProjectile()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonShootLaserProjectile::Update()
{
	if (turnCreated == dungeon->turn)
		return;

	// this should move one unit in the facing direction, then perform damage on that space
	if (dungeon->CanSee(position, facing)) // this sould probably change to something like the canmove but without the player interaction
	{
		position += directions[facing];
		object->AddLocalPosition({ directions[facing].x * DUNGEON_GRID_SCALE, directions[facing].y * DUNGEON_GRID_SCALE, 0 });
		if (dungeon->DamageAtPosition(position, this, false, Dungeon::DamageType::Shooter))
			shouldDestroySelf = true;
	}
	else // if it collides, it should delete itself magically
	{
		shouldDestroySelf = true;
	}
}
