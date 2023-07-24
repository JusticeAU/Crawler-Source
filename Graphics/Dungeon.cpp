#include "Dungeon.h"
#include "DungeonHelpers.h"
#include "DungeonInteractableLever.h"
#include "DungeonDoor.h"
#include "DungeonPlayer.h"
#include "Object.h"
#include "DungeonActivatorPlate.h"
#include "DungeonTransporter.h"
#include "DungeonSpikes.h"
#include "DungeonPushableBlock.h"
#include "DungeonShootLaser.h"
#include "DungeonDamageVisual.h"
#include "DungeonShootLaserProjectile.h"
#include "DungeonEnemyBlocker.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include "Scene.h"
#include "ComponentFactory.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "serialisation.h"

Crawl::Dungeon::Dungeon()
{
	wallVariantPaths.push_back("crawler/model/tile_wall1_blockout/tile_wall1_blockout.object");
	wallVariantPaths.push_back("crawler/model/tile_wall2_blockout/tile_wall2_blockout.object");
	wallVariantPaths.push_back("crawler/model/tile_wall3_blockout/tile_wall3_blockout.object");

	InitialiseTileMap();
}

/// <summary>
/// Adds a dungeonTile to the grid.
/// </summary>
/// <param name="column"></param>
/// <param name="row"></param>
/// <returns>A reference to the newly added dungeonTile. If a dungeonTile already existing, then a nullptr is returned.</returns>
Crawl::DungeonTile* Crawl::Dungeon::AddTile(ivec2 position)
{
	Column& col = tiles[position.x];

	auto existingTile = col.row.find(position.y);
	if (existingTile != col.row.end())
		return nullptr; // dungeonTile existed already, no duplicating or overwriting please!

	DungeonTile newTile;
	newTile.position = position;
	return &col.row.emplace(position.y, newTile).first->second;
}

void Crawl::Dungeon::AddTile(DungeonTile& dungeonTile)
{
	Column& col = tiles[dungeonTile.position.x];

	auto existingTile = col.row.find(dungeonTile.position.y);
	if (existingTile != col.row.end())
		return; // dungeonTile existed already, no duplicating or overwriting please!

	col.row.emplace(dungeonTile.position.y, dungeonTile).first->second;
}

bool Crawl::Dungeon::SetTileMask(ivec2 position, int mask)
{
	DungeonTile* tile = GetTile(position);
	if (!tile)
		return false;

	tile->mask = mask;
	return true;
}

Crawl::DungeonTile* Crawl::Dungeon::GetTile(ivec2 pos)
{
	Column& col = tiles[pos.x];

	auto existingTile = col.row.find(pos.y);
	if (existingTile == col.row.end())
		return nullptr;

	return &existingTile->second;
}

bool Crawl::Dungeon::DeleteTile(ivec2 position)
{
	auto col = tiles.find(position.x);
	if (col == tiles.end())
		return false;

	auto tile = col->second.row.find(position.y);
	if (tile == col->second.row.end())
		return false;


	tile->second.object->markedForDeletion = true;
	col->second.row.erase(tile);
	return true;
}

void Crawl::Dungeon::CreateTileObject(DungeonTile* tile)
{
	if (tile->object != nullptr)
		tile->object->markedForDeletion = true;

	Object* obj = Scene::s_instance->DuplicateObject(tile_template, Scene::s_instance->objects[2]);
	
	// Set up wall variants
	for (int i = 0; i < 4; i++)
	{
		int variant = tile->wallVariants[i];
		if (variant > 0)
		{
			ordered_json wall = ReadJSONFromDisk(wallVariantPaths[variant-1]);
			obj->children[i + 1]->children[0]->LoadFromJSON(wall); // i+1 because this object has the floor tile in index 0;
		}
	}
	obj->SetLocalPosition({ tile->position.x * DUNGEON_GRID_SCALE, tile->position.y * DUNGEON_GRID_SCALE , 0 });

	tile->object = obj;
}

bool Crawl::Dungeon::IsOpenTile(ivec2 position)
{
	auto col = tiles.find(position.x);
	if (col == tiles.end())
		return false;

	auto tile = col->second.row.find(position.y);
	if (tile == col->second.row.end())
		return false;

	return true;
}

bool Crawl::Dungeon::HasLineOfSight(ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];

	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;
	unsigned int reverseDirectionIndex = directionIndex + 2;
	if (reverseDirectionIndex > 3) reverseDirectionIndex -= 4;

	bool canMove = true;
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// Check if the tile we're on allows us to move in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	canMove = (currentTile->mask & directionMask) == directionMask;

	if (!canMove)
		return canMove;

	// check for edge blocked - Doors!
	// Check tile we're on
	DungeonDoor* doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == fromPos && activatable[i]->orientation == directionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

	// check tile we want to move to.
	doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == toPos && activatable[i]->orientation == reverseDirectionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

return canMove;
}

bool Crawl::Dungeon::CanMove(glm::ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];

	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;
	unsigned int reverseDirectionIndex = directionIndex + 2;
	if (reverseDirectionIndex > 3) reverseDirectionIndex -= 4;

	bool canMove = true;
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// Check if the tile we're on allows us to move in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	canMove = (currentTile->mask & directionMask) == directionMask;

	if (!canMove)
		return canMove;

	// check for edge blocked - Doors!
	// Check tile we're on
	DungeonDoor* doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == fromPos && activatable[i]->orientation == directionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

	// check tile we want to move to.
	doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == toPos && activatable[i]->orientation == reverseDirectionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open)
		return false;

	// check destination blocked by pushable box
	DungeonTile* toTile = GetTile(toPos);

	// check if we can push boxes
	if (playerCanPushBox)
	{
		// needs to be a 'if block can be pushed in direction check'
		for (int i = 0; i < pushableBlocks.size(); i++)
		{
			if (pushableBlocks[i]->position == toPos)
			{
				LogUtils::Log("There is a block where we are trying to move");
				// there is a block where we want to go to.
				// can it be pushed?
				DungeonTile* blockTo = GetTile(toPos + directions[directionIndex]);
				if (blockTo)
				{
					LogUtils::Log("block has a tile behind it");
					// check line of sight
					if (!HasLineOfSight(toPos, directionIndex))
						return false; // A door or something is blocking.

					if (blockTo->occupied)
					{
						LogUtils::Log("the tile is occupied");
						return false;
					}
					else
					{
						LogUtils::Log("There tile is not occupied, kicking it");
						DoKick(fromPos, (FACING_INDEX)directionIndex);
						return true;
					}
				}
			}
		}
	}
	canMove = !toTile->occupied;

	return canMove;
}

bool Crawl::Dungeon::DoInteractable(unsigned int id)
{
	bool didInteract = false;
	for (int i = 0; i < interactables.size(); i++)
	{
		if (id == interactables[i]->id)
		{
			if (interactables[i]->position == player->GetPosition())
			{
				interactables[i]->Toggle();
				didInteract = true;
			}
		}
	}
	return didInteract;
}

Crawl::DungeonInteractableLever* Crawl::Dungeon::CreateLever(ivec2 position, unsigned int directionIndex, unsigned int id, unsigned int doorID, bool startStatus)
{
	DungeonInteractableLever* lever = new DungeonInteractableLever();
	lever->position = position;
	lever->orientation = directionIndex;
	lever->dungeon = this;
	lever->startStatus = startStatus;
	lever->status = startStatus;

	// Load lever Objects from JSON
	ordered_json lever_objectJSON = ReadJSONFromDisk("crawler/object/interactable_lever.object");
	ordered_json lever_modelJSON = ReadJSONFromDisk("crawler/model/interactable_lever_blockout/interactable_lever_blockout.object");

	// load Model object in to Model child object
	Object* lever_object = Scene::CreateObject();
	lever_object->LoadFromJSON(lever_objectJSON);
	lever->object = lever_object;
	lever_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 2.5 });
	Object* lever_model = Scene::CreateObject(lever_object->children[0]);
	lever_model->LoadFromJSON(lever_modelJSON);
	lever_object->SetLocalRotationZ(orientationEulers[directionIndex]);
	lever->SetID(id);
	lever->activateID = doorID;
	interactables.push_back(lever);
	return lever;
}

void Crawl::Dungeon::DoActivate(unsigned int id)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->id == id)
			activatable[i]->Toggle();
	}
}

void Crawl::Dungeon::DoActivate(unsigned int id, bool on)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->id == id)
			activatable[i]->Toggle(on);
	}

	if (on)
	{
		for (int i = 0; i < shootLasers.size(); i++)
		{
			if (shootLasers[i]->activateID == id)
				shootLasers[i]->Activate();
		}
	}
}

bool Crawl::Dungeon::DamageAtPosition(ivec2 position, bool fromPlayer)
{
	bool didDamage = false;
	CreateDamageVisual(position, fromPlayer);
	// For now it's just the player, but this might need to turn in to "damagables"
	// For prototype, lets just go with player, and vectors of things that can take damage (enemies)
	if (player->GetPosition() == position)
	{
		didDamage = true;
		player->TakeDamage();
	}

	// check boxes
	// check for pushable boxes at spikes
	for (int i = 0; i < pushableBlocks.size(); i++)
	{
		if (pushableBlocks[i]->position == position)
		{
			GetTile(pushableBlocks[i]->position)->occupied = false;
			delete pushableBlocks[i];
			pushableBlocks.erase(pushableBlocks.begin() + i);
			i--;
		}
	}

	// check blockers
	for (int i = 0; i < blockers.size(); i++)
	{
		if (blockers[i]->position == position)
		{
			didDamage = true;
			RemoveEnemyBlocker(position);
		}
	}
	return didDamage;
}

bool Crawl::Dungeon::DoKick(ivec2 fromPosition, FACING_INDEX direction)
{
	// translate kick in direction
	ivec2 targetPosition = fromPosition + directions[direction];

	// check you have line of sight to position
	if (!HasLineOfSight(fromPosition, direction))
		return false;

	// see if theres anything kickable in that position
	DungeonPushableBlock* pushable = nullptr;
	for (auto& block : pushableBlocks)
	{
		if (block->position == targetPosition)
		{
			pushable = block;
			break;
		}
	}
	if (!pushable)
		return false;

	DungeonTile* kickTile = GetTile(targetPosition);

	// see if the thing can move
	ivec2 moveToPos = targetPosition + directions[direction];

	if (!HasLineOfSight(targetPosition, direction))
		return false;

	// is there a tile?
	DungeonTile* toTile = GetTile(moveToPos);
	if (!toTile)
		return false;
	// is it occupied?
	if (toTile->occupied)
		return false; // We coudl do damage and shit here.

	// kick it in that direction
	kickTile->occupied = false;
	toTile->occupied = true;
	pushable->position = moveToPos;
	pushable->object->AddLocalPosition({ directions[direction].x * DUNGEON_GRID_SCALE, directions[direction].y * DUNGEON_GRID_SCALE, 0 });
	return true;
}

Crawl::DungeonDoor* Crawl::Dungeon::CreateDoor(ivec2 position, unsigned int directionIndex, unsigned int id, bool startOpen)
{
	DungeonDoor* door = new DungeonDoor();
	door->position = position;
	door->orientation = directionIndex;
	door->id = id;
	door->startOpen = startOpen;
	door->open = startOpen;

	ordered_json door_objectJSON = ReadJSONFromDisk("crawler/object/interactable_door.object");
	Object* door_object = Scene::CreateObject();
	door_object->LoadFromJSON(door_objectJSON);
	door->object = door_object;
	door_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	ordered_json door_modelJSON = ReadJSONFromDisk("crawler/model/door_jail_blockout/door_jail_blockout.object");
	Object* door_model = Scene::CreateObject(door_object->children[0]);
	door_model->LoadFromJSON(door_modelJSON);
	door_object->SetLocalRotationZ(orientationEulers[directionIndex]);
	door->UpdateTransforms();
	activatable.push_back(door);
	return door;
}

Crawl::DungeonActivatorPlate* Crawl::Dungeon::CreatePlate(ivec2 position, unsigned int activateID)
{
	DungeonActivatorPlate* plate = new DungeonActivatorPlate();
	plate->position = position;
	plate->activateID = activateID;
	plate->dungeon = this;
	plate->object = Scene::CreateObject();
	plate->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/interactable_floortile.object"));
	plate->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_floorplate/interactable_floorplate.object"));
	plate->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	activatorPlates.push_back(plate);
	return plate;
}

Crawl::DungeonTransporter* Crawl::Dungeon::CreateTransporter(ivec2 position)
{
	DungeonTransporter* transporter = new DungeonTransporter();
	transporter->position = position;
	transporter->object = Scene::CreateObject();
	transporter->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/exit.object"));
	transporter->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1 });
	transporterPlates.push_back(transporter);
	return transporter;
}

Crawl::DungeonSpikes* Crawl::Dungeon::CreateSpikes(ivec2 position)
{
	DungeonSpikes* spikes = new DungeonSpikes();
	spikes->position = position;
	spikes->object = Scene::CreateObject();
	spikes->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/spikes.object"));
	spikes->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	spikesPlates.push_back(spikes);
	return spikes;
}

void Crawl::Dungeon::RemoveSpikes(ivec2 position)
{
	for (int i = 0; i < spikesPlates.size(); i++)
	{
		if (spikesPlates[i]->position == position)
		{
			delete spikesPlates[i];
			spikesPlates.erase(spikesPlates.begin() + i);
			break;
		}
	}
}

Crawl::DungeonPushableBlock* Crawl::Dungeon::CreatePushableBlock(ivec2 position)
{
	DungeonPushableBlock* pushable = new DungeonPushableBlock();
	pushable->position = position;
	pushable->object = Scene::CreateObject();
	pushable->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/pushable.object"));
	pushable->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	pushableBlocks.push_back(pushable);

	// set tile its in to be occupied.
	DungeonTile* tile = GetTile(position);
	if (tile)
		tile->occupied = true;
	return pushable;
}

void Crawl::Dungeon::RemovePushableBlock(ivec2 position)
{
	for (int i = 0; i < pushableBlocks.size(); i++)
	{
		if (pushableBlocks[i]->position == position)
		{
			delete pushableBlocks[i];
			pushableBlocks.erase(pushableBlocks.begin() + i);
			
			// set tile its in to be unoccupied
			// This is super dodgy and will cause bugs probably.
			DungeonTile* tile = GetTile(position);
			if (tile)
				tile->occupied = false;
			break;
		}
	}
}

Crawl::DungeonTransporter* Crawl::Dungeon::GetTransporter(string transporterName)
{
	for (auto& transporter : transporterPlates)
		if (transporter->name == transporterName) return transporter;

	return nullptr;
}

Crawl::DungeonShootLaser* Crawl::Dungeon::CreateShootLaser(ivec2 position, FACING_INDEX facing, unsigned int id)
{
	DungeonShootLaser* shootLaser = new DungeonShootLaser();
	shootLaser->dungeon = this;
	shootLaser->id = id;
	shootLaser->position = position;
	shootLaser->facing = facing;
	shootLaser->object = Scene::CreateObject();
	shootLaser->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/shoot_laser.object"));
	shootLaser->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	shootLaser->object->SetLocalRotationZ(orientationEulersReversed[facing]);
	shootLasers.emplace_back(shootLaser);
	return shootLaser;
}

void Crawl::Dungeon::RemoveDungeonShootLaser(ivec2 position)
{
	for (int i = 0; i < shootLasers.size(); i++)
	{
		if (shootLasers[i]->position == position)
		{
			delete shootLasers[i];
			shootLasers.erase(shootLasers.begin() + i);
		}
	}
}

void Crawl::Dungeon::CreateDamageVisual(ivec2 position, bool fromPlayer)
{
	DungeonDamageVisual* visual = new DungeonDamageVisual();
	visual->turnCreated = turn;
	if (fromPlayer)
		visual->turnCreated += 1;
	visual->position = position;
	visual->object = Scene::CreateObject();
	visual->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/damage_visual.object"));
	visual->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	damageVisuals.emplace_back(visual);
}

void Crawl::Dungeon::CreateShootLaserProjectile(ivec2 position, FACING_INDEX direction)
{
	DamageAtPosition(position);
	
	DungeonShootLaserProjectile* projectile = new DungeonShootLaserProjectile();
	projectile->dungeon = this;
	projectile->turnCreated = turn;
	projectile->position = position;
	projectile->facing = direction;
	projectile->object = Scene::CreateObject();
	projectile->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/shoot_laser_visual.object"));
	projectile->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	shootLaserProjectiles.emplace_back(projectile);
}

Crawl::DungeonEnemyBlocker* Crawl::Dungeon::CreateEnemyBlocker(ivec2 position, FACING_INDEX direction)
{
	DungeonEnemyBlocker* blocker = new DungeonEnemyBlocker();
	blocker->position = position;
	blocker->facing = direction;
	blocker->dungeon = this;
	blocker->object = Scene::CreateObject();
	blocker->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/monster_blocker.object"));
	blocker->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	blockers.emplace_back(blocker);

	DungeonTile* tile = GetTile(position);
	if (tile)
	{
		tile->occupied = true;
	}
	else
		LogUtils::Log("WARNING - ATTEMPTING TO ADD BLOCKER TO TILE THAT DOESN'T EXIST");

	return blocker;
}

void Crawl::Dungeon::RemoveEnemyBlocker(ivec2 position)
{
	for (int i = 0; i < blockers.size(); i++)
	{
		if (position == blockers[i]->position)
		{
			delete blockers[i];
			blockers.erase(blockers.begin() + i);

			DungeonTile* tile = GetTile(position);
			if (tile)
				tile->occupied = false;
		}
	}
}

void Crawl::Dungeon::Save(std::string filename)
{
	BuildSerialised();
	WriteJSONToDisk(filename, serialised);
}

void Crawl::Dungeon::Load(std::string filename)
{
	int lastSlash = filename.find_last_of('/');
	int extensionStart = filename.find_last_of('.');
	string name = filename.substr(lastSlash + 1, extensionStart - (lastSlash + 1));
	dungeonFileName = name;
	dungeonFilePath = filename;
	serialised = ReadJSONFromDisk(filename);
	RebuildFromSerialised();
}

bool Crawl::Dungeon::TestDungeonExists(std::string filename)
{
	return FileUtils::CheckFileExists(filename);
}

void Crawl::Dungeon::BuildSerialised()
{
	serialised.clear();

	ordered_json tiles_json;

	serialised["type"] = "dungeon";
	serialised["version"] = 1;
	serialised["defaultPosition"] = defaultPlayerStartPosition;
	serialised["defaultOrientation"] = defaultPlayerStartOrientation;
	serialised["playerTurnIsFree"] = playerTurnIsFree;
	serialised["playerHasKnife"] = playerHasKnife;
	serialised["playerCanKickBox"] = playerCanKickBox;
	serialised["playerCanPushBox"] = playerCanPushBox;

	for (auto& x : tiles)
	{
		for (auto& y : x.second.row)
			tiles_json.push_back(y.second);
	}
	serialised["tiles"] = tiles_json;

	ordered_json levers_json;
	for (auto& interactable : interactables)
		levers_json.push_back(*interactable);
	serialised["levers"] = levers_json;

	ordered_json doors_json;
	for (auto& door : activatable)
		doors_json.push_back(*door);
	serialised["doors"] = doors_json;

	ordered_json plates_json;
	for (auto& plate : activatorPlates)
		plates_json.push_back(*plate);
	serialised["plates"] = plates_json;

	ordered_json transporters_json;
	for (auto& plate : transporterPlates)
		transporters_json.push_back(*plate);
	serialised["transporters"] = transporters_json;

	ordered_json spikes_json;
	for (auto& spikes : spikesPlates)
		spikes_json.push_back(*spikes);
	serialised["spikes"] = spikes_json;

	ordered_json blocks_json;
	for (auto& block : pushableBlocks)
		blocks_json.push_back(*block);
	serialised["blocks"] = blocks_json;

	ordered_json shootLaser_json;
	for (auto& shootLaser : shootLasers)
		shootLaser_json.push_back(*shootLaser);
	serialised["shootLasers"] = shootLaser_json;

	ordered_json blockers_json;
	for (auto& blocker : blockers)
		blockers_json.push_back(*blocker);
	serialised["blockers"] = blockers_json;
}

void Crawl::Dungeon::RebuildFromSerialised()
{
	DestroySceneFromDungeonLayout();
	tiles.clear();

	if (serialised.contains("defaultPosition"))
		serialised.at("defaultPosition").get_to(defaultPlayerStartPosition);
	else
		defaultPlayerStartPosition = { 0,0 };

	if (serialised.contains("defaultOrientation"))
		serialised.at("defaultOrientation").get_to(defaultPlayerStartOrientation);
	else
		defaultPlayerStartOrientation = EAST_INDEX;

	if (serialised.contains("playerTurnIsFree"))
		serialised.at("playerTurnIsFree").get_to(playerTurnIsFree);
	else
		playerTurnIsFree = true;

	if (serialised.contains("playerHasKnife"))
		serialised.at("playerHasKnife").get_to(playerHasKnife);
	else
		playerHasKnife = true;

	if (serialised.contains("playerCanKickBox"))
		serialised.at("playerCanKickBox").get_to(playerCanKickBox);
	else
		playerTurnIsFree = true;

	if (serialised.contains("playerCanPushBox"))
		serialised.at("playerCanPushBox").get_to(playerCanPushBox);
	else
		playerHasKnife = true;


	auto& tiles_json = serialised["tiles"];
	for (auto it = tiles_json.begin(); it != tiles_json.end(); it++)
	{
		DungeonTile tile = it.value().get<Crawl::DungeonTile>();
		AddTile(tile);
	}

	auto& doors_json = serialised["doors"];
	for (auto it = doors_json.begin(); it != doors_json.end(); it++)
	{
		DungeonDoor door = it.value().get<Crawl::DungeonDoor>();
		CreateDoor(door.position, door.orientation, door.id, door.startOpen);
	}

	auto& levers_json = serialised["levers"];
	for (auto it = levers_json.begin(); it != levers_json.end(); it++)
	{
		DungeonInteractableLever lever = it.value().get<Crawl::DungeonInteractableLever>();
		DungeonInteractableLever* newLever = CreateLever(lever.position, lever.orientation, lever.id, lever.activateID, lever.startStatus);
		if (newLever->status)
		{
			newLever->Prime();
		}
	}

	auto& plates_json = serialised["plates"];
	for (auto it = plates_json.begin(); it != plates_json.end(); it++)
	{
		DungeonActivatorPlate plate = it.value().get<Crawl::DungeonActivatorPlate>();
		CreatePlate(plate.position, plate.activateID);
	}

	auto& transporters_json = serialised["transporters"];
	for (auto it = transporters_json.begin(); it != transporters_json.end(); it++)
	{
		DungeonTransporter transporter = it.value().get<Crawl::DungeonTransporter>();
		DungeonTransporter* newTransporter = CreateTransporter(transporter.position);
		newTransporter->name = transporter.name;
		newTransporter->toDungeon = transporter.toDungeon;
		newTransporter->toTransporter = transporter.toTransporter;
		newTransporter->fromOrientation = transporter.fromOrientation;
	}

	auto& spikes_json = serialised["spikes"];
	for (auto it = spikes_json.begin(); it != spikes_json.end(); it++)
	{
		DungeonSpikes spikes = it.value().get<Crawl::DungeonSpikes>();
		CreateSpikes(spikes.position);
	}

	auto& blocks_json = serialised["blocks"];
	for (auto it = blocks_json.begin(); it != blocks_json.end(); it++)
	{
		DungeonPushableBlock block = it.value().get<Crawl::DungeonPushableBlock>();
		CreatePushableBlock(block.position);
	}

	auto& shootLasers_json = serialised["shootLasers"];
	for (auto it = shootLasers_json.begin(); it != shootLasers_json.end(); it++)
	{
		DungeonShootLaser shootLaser = it.value().get<Crawl::DungeonShootLaser>();
		DungeonShootLaser* newLaser = CreateShootLaser(shootLaser.position, shootLaser.facing, shootLaser.id);
		newLaser->detectsLineOfSight = shootLaser.detectsLineOfSight;
		newLaser->firesProjectile = shootLaser.firesProjectile;
		newLaser->firesImmediately = shootLaser.firesImmediately;
		newLaser->activateID = shootLaser.activateID;
	}

	auto& blockers_json = serialised["blockers"];
	for (auto it = blockers_json.begin(); it != blockers_json.end(); it++)
	{
		DungeonEnemyBlocker blocker = it.value().get<Crawl::DungeonEnemyBlocker>();
		DungeonEnemyBlocker* newBlocker = CreateEnemyBlocker(blocker.position, blocker.facing);
		newBlocker->rapidAttack = blocker.rapidAttack;
	}

	BuildSceneFromDungeonLayout();
}

void Crawl::Dungeon::InitialiseTileMap()
{
	// Load the JSON template
	ordered_json tile_layout = ReadJSONFromDisk("crawler/object/tile_layout.object");
	ordered_json tile_ground1 = ReadJSONFromDisk("crawler/model/tile_ground1_blockout/tile_ground1_blockout.object");

	tile_template = new Object(0, "Tile Template");
	tile_template->LoadFromJSON(tile_layout);
	tile_template->children[0]->children[0]->LoadFromJSON(tile_ground1);
}

void Crawl::Dungeon::DestroySceneFromDungeonLayout()
{
	turn = 0;

	for (auto& x : tiles)
	{
		for (auto& y : x.second.row)
		{
			Crawl::DungeonTile* tile = &y.second;
			tile->object->markedForDeletion = true;
		}
	}

	for (int i = 0; i < interactables.size(); i++)
		delete interactables[i];
	interactables.clear();

	for (int i = 0; i < activatable.size(); i++)
		delete activatable[i];
	activatable.clear();

	for (int i = 0; i < activatorPlates.size(); i++)
		delete activatorPlates[i];
	activatorPlates.clear();

	for (int i = 0; i < transporterPlates.size(); i++)
		delete transporterPlates[i];
	transporterPlates.clear();

	for (int i = 0; i < spikesPlates.size(); i++)
		delete spikesPlates[i];
	spikesPlates.clear();

	for (int i = 0; i < pushableBlocks.size(); i++)
		delete pushableBlocks[i];
	pushableBlocks.clear();

	for (int i = 0; i < shootLasers.size(); i++)
		delete shootLasers[i];
	shootLasers.clear();

	// clean up - do not need to be rebuild, just stuff created from game play
	for (int i = 0; i < damageVisuals.size(); i++)
		delete damageVisuals[i];
	damageVisuals.clear();

	for (int i = 0; i < shootLaserProjectiles.size(); i++)
		delete shootLaserProjectiles[i];
	shootLaserProjectiles.clear();

	for (int i = 0; i < blockers.size(); i++)
		delete blockers[i];
	blockers.clear();

}

Object* Crawl::Dungeon::GetTileTemplate(int mask)
{
	return tile_template;
}

void Crawl::Dungeon::SetParentTileObject(Object* object)
{
	tile_template->parent = object;
}

unsigned int Crawl::Dungeon::GetReverseDirectionMask(unsigned int direction)
{
	switch (direction)
	{
	case NORTH_MASK:
		return SOUTH_MASK;
	case SOUTH_MASK:
		return NORTH_MASK;
	case EAST_MASK:
		return WEST_MASK;
	case WEST_MASK:
		return EAST_MASK;
	default:
		return 0; // Shouldn't get here.
	}
}

void Crawl::Dungeon::Update()
{
	turn++;
	string turnMessage = "Turn: " + std::to_string(turn);
	LogUtils::Log(turnMessage.c_str());

	// UPDATE PHASE! - Order of precedence is very important here.
	// remove any expired shoot visual indicators
	for (int i = 0; i < damageVisuals.size(); i++)
	{
		if (damageVisuals[i]->turnCreated < turn)
		{
			delete damageVisuals[i];
			damageVisuals.erase(damageVisuals.begin() + i);
			i--;
		}
	}

	// test all activator plates
	for (auto& tileTest : activatorPlates)
		tileTest->TestPosition();

	// update all doors
	for (auto& door : activatable)
		door->Update();

	// make all projectiles update and destroy them if they have collided
	for (int i = 0; i < shootLaserProjectiles.size(); i++)
	{
		shootLaserProjectiles[i]->Update();
		if (shootLaserProjectiles[i]->shouldDestroySelf)
		{
			delete shootLaserProjectiles[i];
			shootLaserProjectiles.erase(shootLaserProjectiles.begin() + i);
			i--;
		}
	}

	// have all shooters update
	for (auto& shooters : shootLasers)
		shooters->Update();

	// All spikes perform damage
	for (auto& spikes : spikesPlates)
	{
		// check for pushable boxes at spikes
		for (int i = 0; i < pushableBlocks.size(); i++)
		{
			if (pushableBlocks[i]->position == spikes->position)
			{
				GetTile(pushableBlocks[i]->position)->occupied = false;
				spikes->disabled = true;
				((ComponentRenderer*)spikes->object->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/spikes_covered.material");
				delete pushableBlocks[i];
				pushableBlocks.erase(pushableBlocks.begin() + i);
				break;
			}
		}

		if(!spikes->disabled)
			DamageAtPosition(spikes->position);
	}

	// All Sword Blocker Enemies Update
	for (auto& blocker : blockers)
		blocker->Update();

	// Test all transporters - This should probably be in the player controller rather than the dungeon.
	DungeonTransporter* activateTransporter = nullptr;
	for (auto& transporter : transporterPlates)
	{
		if (transporter->position == player->GetPosition())
		{
			activateTransporter = transporter;
			break;
		}
	}
	if (activateTransporter)
	{
		string dungeonToLoad = activateTransporter->toDungeon;
		if (!TestDungeonExists(dungeonToLoad + ".dungeon"))
		{
			LogUtils::Log("Dungeon does not exist, bailing on loading:");
			LogUtils::Log(dungeonToLoad.c_str());
			return;
		}

		string TransporterToGoTo = activateTransporter->toTransporter;
		// Load dungeonName
		Load(dungeonToLoad + ".dungeon");

		// Get Transporter By Name
		DungeonTransporter* gotoTransporter = GetTransporter(TransporterToGoTo);

		// Set player Position
		if (gotoTransporter)
		{
			player->SetRespawn(gotoTransporter->position, (FACING_INDEX)gotoTransporter->fromOrientation);
			player->Respawn();
		}
		else
		{
			LogUtils::Log("Unable to find transporter in new dungeon:");
			LogUtils::Log(TransporterToGoTo.c_str());
			LogUtils::Log("Spawning at default dungeon position.");
			player->SetRespawn(defaultPlayerStartPosition, defaultPlayerStartOrientation);
			player->Respawn();
		}
	}
}

unsigned int Crawl::Dungeon::GetAutoTileMask(ivec2 position)
{
	unsigned int tile = 0;
	// test north
	if (IsOpenTile(position + NORTH_COORDINATE))
		tile += NORTH_MASK;
	// test west
	if (IsOpenTile(position + WEST_COORDINATE))
		tile += WEST_MASK;
	// test east
	if (IsOpenTile(position + EAST_COORDINATE))
		tile += EAST_MASK;
	// test south
	if (IsOpenTile(position + SOUTH_COORDINATE))
		tile += SOUTH_MASK;
	return tile;
}

void Crawl::Dungeon::BuildSceneFromDungeonLayout()
{
	// for each tile, create an object.
	for (auto& column : tiles)
	{
		for (auto& row : column.second.row)
		{
			Crawl::DungeonTile* tile = &row.second;
			CreateTileObject(tile);
		}
	}
}
