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
#include "DungeonEnemyChase.h"
#include "DungeonEnemySwitcher.h"
#include "DungeonCheckpoint.h"
#include "DungeonMirror.h"
#include "DungeonEnemySlug.h"
#include "DungeonEnemySlugPath.h"
#include "DungeonDecoration.h"
#include "DungeonStairs.h"
#include "DungeonLight.h"
#include "DungeonEventTrigger.h"

#include "FileUtils.h"
#include "LogUtils.h"

#include "Scene.h"
#include "ComponentFactory.h"
#include "ModelManager.h"
#include "MaterialManager.h"
#include "serialisation.h"
#include <algorithm>

Crawl::Dungeon::Dungeon(bool fakeDungeon) : fakeDungeon(fakeDungeon)
{
	if (fakeDungeon)
		return;

	wallVariantPaths.push_back("crawler/model/tile_wall_1.object");
	wallVariantPaths.push_back("crawler/model/tile_wall_seethrough.object");

	tilesParentObject = Scene::CreateObject("Tiles");

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
	DungeonTile newTile;
	newTile.position = position;
	return AddTile(newTile);
}

Crawl::DungeonTile* Crawl::Dungeon::AddTile(DungeonTile& dungeonTile)
{
	Column& col = tiles[dungeonTile.position.x];

	auto existingTile = col.row.find(dungeonTile.position.y);
	if (existingTile != col.row.end())
		return nullptr; // dungeonTile existed already, no duplicating or overwriting please!

	// Create the tile
	DungeonTile* tile = &col.row.emplace(dungeonTile.position.y, dungeonTile).first->second;
	
	UpdateTileNeighbors(tile->position);
	return tile;
}

void Crawl::Dungeon::UpdateTileNeighbors(ivec2 atPosition)
{
	DungeonTile* centreTile = GetTile(atPosition);

	// This is done a little weirdly, because this function is called on both creation and deletion of a tile
	// So it can run on a co-ordinate where there isn't a tile, and it should update neighours of the deleted tile accurately with a nullptr.
	for (int directionIndex = 0; directionIndex < 4; directionIndex++)
	{
		DungeonTile* neighbor = GetTile(atPosition + directions[directionIndex]);
		if (centreTile)
			centreTile->neighbors[directionIndex] = neighbor;

		if(neighbor)
			neighbor->neighbors[facingIndexesReversed[directionIndex]] = centreTile;
	}
}

bool Crawl::Dungeon::FindPath(ivec2 from, ivec2 to, int facing)
{
	bool turningIsFree = true;
	if (facing != -1)
		turningIsFree = false;

	/*LogUtils::Log("FindPath: GO!");
	LogUtils::Log(std::to_string(glfwGetTime()));*/
	goodPath.clear();

	// Clean up the data in the dungeon before we being.
	for (auto& column : tiles)
	{
		for (auto& tile : column.second.row)
		{
			tile.second.cost = 0;
			tile.second.openListed = false;
			tile.second.closedListed = false;
			tile.second.fromDestination = nullptr;
			tile.second.toDestination = nullptr;
			tile.second.enterDirection = -1;
		}
	}

	DungeonTile* fromTile = GetTile(from);
	if (fromTile == nullptr)
	{
		LogUtils::Log("FindPath: unable to find start tile");
		return false;
	}

	DungeonTile* toTile = GetTile(to);
	if (toTile == nullptr)
	{
		LogUtils::Log("FindPath: destination not a valid tile");
		return false;
	}

	vector<DungeonTile*> openList;
	vector<DungeonTile*> closedList;
	openList.push_back(fromTile);

	DungeonTile* currentTile = openList.front();
	while (true)
	{
		// We've arrived. This will need to be tweaked based on whos doing what.
		if (currentTile == toTile)
			break;

		currentTile->closedListed = true;
		closedList.push_back(currentTile);

		for (int directionIndex = 0; directionIndex < 4; directionIndex++) // check all directions
		{
			if (!CanTraverse(currentTile->position, directionIndex))
				continue;

			DungeonTile* connectingTile = currentTile->neighbors[directionIndex];
			if (connectingTile) // if there is a connectingTile
			{

				if (!connectingTile->closedListed)
				{
					int cost = currentTile->cost + 1;
					if (!turningIsFree)
					{
						int turnCost = glm::abs(facing - directionIndex);
						if (turnCost == 3) turnCost = 1; // lets not turn 3 times one way when we could turn once the other
						cost += turnCost;
					}

					if (!connectingTile->openListed)
					{
						// Check if the tile is occupied by something that isnt the player or another chaser/enemy
						bool shouldSkip = false;
						if (IsEnemyBlockerAtPosition(connectingTile->position)) shouldSkip = true;
						if (IsPushableBlockAtPosition(connectingTile->position)) shouldSkip = true;
						if (IsEnemySwitcherAtPosition(connectingTile->position)) shouldSkip = true;
						if (IsSpikesAtPosition(connectingTile->position)) shouldSkip = true;
						if (IsSlugPath(connectingTile->position)) shouldSkip = true;
						if (IsMirrorAtPosition(connectingTile->position)) shouldSkip = true;

						if (shouldSkip)
						{
							connectingTile->openListed = true;
							connectingTile->closedListed = true;
						}
						else
						{
							connectingTile->cost = cost;
							connectingTile->openListed = true;
							connectingTile->fromDestination = currentTile;
							connectingTile->enterDirection = directionIndex;
							openList.push_back(connectingTile);
							std::push_heap(openList.begin(), openList.end(), Dungeon::CostToPlayerGreaterThan);
						}
					}
					else if(cost < connectingTile->cost)
					{
						connectingTile->cost = cost;
						connectingTile->fromDestination = currentTile;
						connectingTile->enterDirection = directionIndex;
						std::make_heap(openList.begin(), openList.end(), Dungeon::CostToPlayerGreaterThan);
					}
				}
			}
		}
		if (openList.empty())
			break;
		
		std::pop_heap(openList.begin(), openList.end(), Dungeon::CostToPlayerGreaterThan);
		currentTile = openList.back();
		openList.pop_back();
		facing = currentTile->enterDirection;
	}
	currentTile = toTile;

	if (currentTile->fromDestination == nullptr)
	{
		LogUtils::Log("FindPath: Unable to find path to destination");
		return false;
	}

	goodPath.push_back(currentTile);
	// Trace the path backwards
	while (currentTile != fromTile)
	{
		currentTile->fromDestination->toDestination = currentTile;
		goodPath.push_back(currentTile->fromDestination);
		currentTile = currentTile->fromDestination;
	}
	/*LogUtils::Log(std::to_string(glfwGetTime()));
	LogUtils::Log("FindPath: Done.");*/
	return true;

}

bool Crawl::Dungeon::SetTileMask(ivec2 position, int maskTraverse)
{
	DungeonTile* tile = GetTile(position);
	if (!tile)
		return false;

	tile->maskTraverse = maskTraverse;
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
	UpdateTileNeighbors(position);
	UpdatePillarsForTileCoordinate(position);

	return true;
}

void Crawl::Dungeon::CreateTileObject(DungeonTile* tile)
{
	if (tile->object != nullptr)
		tile->object->markedForDeletion = true;

	Object* obj = Scene::s_instance->DuplicateObject(tile_template, tilesParentObject);
	
	// Set up wall variants
	for (int i = 0; i < 4; i++)
	{
		int variant = tile->wallVariants[i];
		if (variant >= 0)
		{
			ordered_json wall = ReadJSONFromDisk(wallVariantPaths[variant]);
			obj->children[i + 1]->children[0]->LoadFromJSON(wall); // i+1 because this object has the floor tile in index 0;
		}
	}
	obj->SetLocalPosition({ tile->position.x * DUNGEON_GRID_SCALE, tile->position.y * DUNGEON_GRID_SCALE , 0 });

	tile->object = obj;

	// Set up Pillars
	if (pillarsParentObject == nullptr) pillarsParentObject = Scene::CreateObject("Pillars");
	UpdatePillarsForTileCoordinate(tile->position);

	// ceiling
	if (!noRoof)
	{
		Object* ceiling = Scene::CreateObject(obj);
		ceiling->LoadFromJSON(ReadJSONFromDisk("crawler/model/tile_ceiling1.object"));
	}
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

bool Crawl::Dungeon::CanTraverse(ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];

	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;

	bool canMove = true;
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// Check if the tile we're on allows us to move in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	canMove = (currentTile->maskTraverse & directionMask) == directionMask;

	if (!canMove)
		return canMove;

	// check for edge blocked - Doors!

	canMove = !IsDoorBlocking(currentTile, directionIndex);
	if (!canMove)
		return canMove;

	return canMove;
}

bool Crawl::Dungeon::CanSee(ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];

	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;

	bool canMove = true;
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// Check if the tile we're on allows us to see in the requested direction - Maybe I could just create some Masks for each cardinal direction and pass those around instead.
	canMove = (currentTile->maskSee & directionMask) == directionMask;

	if (!canMove)
		return canMove;

	// check for edge blocked - Doors!
	
	canMove = !IsDoorBlocking(currentTile, directionIndex);
	if (!canMove)
		return canMove;

	return canMove;
}

bool Crawl::Dungeon::PlayerCanMove(glm::ivec2 fromPos, int directionIndex)
{
	glm::ivec2 toPos = fromPos + directions[directionIndex];

	unsigned int directionMask;
	if (directionIndex == NORTH_INDEX) directionMask = NORTH_MASK;
	else if (directionIndex == EAST_INDEX) directionMask = EAST_MASK;
	else if (directionIndex == SOUTH_INDEX) directionMask = SOUTH_MASK;
	else directionMask = WEST_MASK;

	bool canMove = true;

	// Check our current location for..
	// for a Tile..
	DungeonTile* currentTile = GetTile(fromPos);
	if (!currentTile) return false; // early out if we're not a tile. We goofed up design wise and cant really resolve this, and shouldn't. Fix the level!

	// for a Wall..
	canMove = (currentTile->maskTraverse & directionMask) == directionMask;
	if (!canMove) return canMove;

	// for a Door (on both tiles, from and to)..
	canMove = !IsDoorBlocking(currentTile, directionIndex);
	if (!canMove) return canMove;

	// Check the location we want to move to...
	// for a Tile
	DungeonTile* toTile = GetTile(toPos);
	if (!toTile) return false; // if there is no tile, we cant move there. (hole in wall?)


	// for a Box..
	// .. that we can push
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
					if (!CanSee(toPos, directionIndex))
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

	// for a Mirror..
	if (playerCanPushMirror)
	{
		// needs to be a 'if mirror/block/thing can be pushed in direction check'
		for (int i = 0; i < mirrors.size(); i++)
		{
			if (mirrors[i]->position == toPos)
			{
				LogUtils::Log("There is a mirror where we are trying to move");
				// there is a mirror where we want to go to.
				// can it be pushed?
				DungeonTile* blockTo = GetTile(toPos + directions[directionIndex]);
				if (blockTo)
				{
					LogUtils::Log("mirror has a tile behind it");
					// check line of sight
					if (!CanSee(toPos, directionIndex))
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

	// ...for general occupance (unpushable box, enemy, or other..)
	if (toTile->occupied) return false;

	return true;
}

bool Crawl::Dungeon::IsPlayerPointOfInterest(ivec2 position)
{
	if (IsEnemyChaserAtPosition(position))return true;
	if (IsSpikesAtPosition(position)) return true;
	if (IsPlateAtPosition(position)) return true;
	
	return false;
}

bool Crawl::Dungeon::IsDoorBlocking(DungeonTile* fromTile, int directionIndex)
{
	ivec2 fromPos = fromTile->position;
	DungeonDoor* doorCheck = nullptr;
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == fromPos && activatable[i]->orientation == directionIndex)
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open) return true; // There is a closed door blocking us from moving off this tile..

	// check for a door on the tile we want to move on to.
	doorCheck = nullptr;
	ivec2 toPos = fromPos + directions[directionIndex];
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->position == toPos && activatable[i]->orientation == facingIndexesReversed[directionIndex])
		{
			doorCheck = activatable[i];
			break;
		}
	}
	if (doorCheck && !doorCheck->open) return true; // There is a closed door blocking us from moving on to this tile..

	return false;
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
				if (interactables[i]->orientation == player->GetOrientation())
				{
					interactables[i]->Toggle();
					didInteract = true;
				}
			}
		}
	}
	return didInteract;
}

bool Crawl::Dungeon::DoInteractable(ivec2 position, FACING_INDEX direction)
{
	bool didInteract = false;
	for (int i = 0; i < interactables.size(); i++)
	{
		if (position == interactables[i]->position && direction == interactables[i]->orientation)
		{
			interactables[i]->Toggle();
			didInteract = true;
		}
	}

	// Search for doors to wobble
	for (int i = 0; i < activatable.size(); i++)
	{
		if (position == activatable[i]->position && direction == activatable[i]->orientation && !activatable[i]->open)
		{
			activatable[i]->PlayRattleSound();
			activatable[i]->shouldWobble = true;
			activatable[i]->wobbleTimeCurrent = 0.0f;
		}
		// doors on shared tile edge (move position in facing direction, and then reverse the checking direction)
		if (position + directions[direction] == activatable[i]->position && facingIndexesReversed[direction] == activatable[i]->orientation && !activatable[i]->open)
		{
			activatable[i]->PlayRattleSound();
			activatable[i]->shouldWobble = true;
			activatable[i]->wobbleTimeCurrent = 0.0f;
		}
	}


	return didInteract;
}

Crawl::DungeonInteractableLever* Crawl::Dungeon::CreateLever(ivec2 position, unsigned int directionIndex, unsigned int id, unsigned int doorID, bool status)
{
	DungeonInteractableLever* lever = new DungeonInteractableLever();
	lever->position = position;
	lever->orientation = directionIndex;
	lever->dungeon = this;
	lever->status = status;

	// Load lever Objects from JSON
	ordered_json lever_objectJSON = ReadJSONFromDisk("crawler/object/interactable_lever.object");
	ordered_json lever_modelJSON = ReadJSONFromDisk("crawler/model/interactable_lever.object");
	ordered_json lever_modelBracketJSON = ReadJSONFromDisk("crawler/model/interactable_lever_bracket.object");


	// load Model object in to Model child object
	Object* lever_object = Scene::CreateObject();
	lever_object->LoadFromJSON(lever_objectJSON);
	lever->object = lever_object;
	lever_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1.5 });
	Object* lever_model = Scene::CreateObject(lever_object->children[0]);
	lever_model->LoadFromJSON(lever_modelJSON);
	Object* lever_modelBracket = Scene::CreateObject(lever_object->children[1]);
	lever_modelBracket->LoadFromJSON(lever_modelBracketJSON);

	lever_object->SetLocalRotationZ(orientationEulers[directionIndex]);
	lever->SetID(id);
	lever->activateID = doorID;
	lever->UpdateTransform(true);
	
	interactables.push_back(lever);
	return lever;
}

void Crawl::Dungeon::RemoveLever(DungeonInteractableLever* lever)
{
	for (int i = 0; i < interactables.size(); i++)
	{
		if (interactables[i] == lever)
		{
			delete interactables[i];
			interactables.erase(interactables.begin() + i);
			return;
		}
	}
}

void Crawl::Dungeon::DoActivate(unsigned int id, bool on)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i]->id == id)
			activatable[i]->Toggle();
	}

	// We only activate lasers on an 'on' signal, they dont fire if something walks off the plate.
	if (on)
	{
		for (int i = 0; i < shootLasers.size(); i++)
		{
			if (shootLasers[i]->activateID == id)
				shootLasers[i]->Activate();
		}
	}
}

bool Crawl::Dungeon::ShouldActivateStairs(ivec2 position, FACING_INDEX direction)
{
	for (int i = 0; i < stairs.size(); i++)
	{
		// check if the direction we want to move would put as on a stair object, and if that stair object starts from that direction.
		if (stairs[i]->startPosition == position + directions[direction] && stairs[i]->directionStart == direction)
		{
			// We should activate this staircase
			player->SetShouldActivateStairs(stairs[i]);
			return true;
		}
	}
	return false;
}

bool Crawl::Dungeon::ShouldActivateTransporter(ivec2 position, FACING_INDEX direction)
{
	DungeonTransporter* activateTransporter = nullptr;
	for (auto& transporter : transporterPlates)
	{
		if (transporter->position == player->GetPosition() + directions[direction])
		{
			activateTransporter = transporter;
			break;
		}
	}

	if (player->isOnLobbyLevel2)
	{
		for (auto& transporter : player->lobbyLevel2Dungeon->transporterPlates)
		{
			if (transporter->position == player->GetPosition() + directions[direction])
			{
				activateTransporter = transporter;
				break;
			}
		}
	}

	if (activateTransporter)
	{
		// check its a valid dungeoowwnnn
		if (TestDungeonExists(activateTransporter->toDungeon + dungeonFileExtension))
		{
			player->SetShouldActivateTransporter(activateTransporter);
			return true;
		}
	}

	return false;
}

bool Crawl::Dungeon::DamageAtPosition(ivec2 position, void* dealer, bool fromPlayer, DamageType damageType)
{
	bool didDamage = false;
	//CreateDamageVisual(position, fromPlayer);
	// For now it's just the player, but this might need to turn in to "damagables"
	// For prototype, lets just go with player, and vectors of things that can take damage (enemies)
	if (player->GetPosition() == position)
	{
		didDamage = true;
		player->TakeDamage();
	}

	// check boxes - We dont do this any more
	if (damageType == DamageType::Physical)
	{
		for (int i = 0; i < pushableBlocks.size(); i++)
		{
			if (pushableBlocks[i]->position == position)
			{
				if(!pushableBlocks[i]->isOnSpikes) pushableBlocks[i]->isDead = true;
				break;
			}
		}
	}

	// check blockers
	if (damageType == DamageType::Physical)
	{
		for (int i = 0; i < blockers.size(); i++)
		{
			if (blockers[i]->position == position)
			{
				didDamage = true;
				RemoveEnemyBlocker(blockers[i]);
				break;
			}
		}
	}

	// check chasers
	for (int i = 0; i < chasers.size(); i++)
	{
		if (chasers[i]->position == position)
		{
			if (dealer == chasers[i])
				continue;
			didDamage = true;
			chasers[i]->isDead = true;
			break;
		}
	}

	// Check for slugs
	if (damageType == DamageType::Physical)
	{
		for (int i = 0; i < slugs.size(); i++)
		{
			if (slugs[i]->position == position)
			{
				if (dealer == slugs[i])
					continue;
				didDamage = true;
				slugs[i]->isDead = true;
				break;
			}
		}
	}

	// Check for Switchers
	for (int i = 0; i < switchers.size(); i++)
	{
		if (switchers[i]->position == position)
		{
			didDamage = true;
			RemoveEnemySwitcher(switchers[i]);
			break;
		}
	}
	return didDamage;
}

bool Crawl::Dungeon::DoKick(ivec2 fromPosition, FACING_INDEX direction)
{
	// translate kick in direction
	ivec2 targetPosition = fromPosition + directions[direction];

	// check you reach into the tile you're interacting with.
	if (!CanTraverse(fromPosition, direction))
		return false;

	// see if theres anything kickable in that position

	// push(kick)able blocks.
	DungeonPushableBlock* pushable = GetPushableBlockAtPosition(targetPosition, false);
	if (pushable)
	{
		DungeonTile* kickTile = GetTile(targetPosition);

		// see if the thing can move
		ivec2 moveToPos = targetPosition + directions[direction];

		if (!CanTraverse(targetPosition, direction))
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
		pushable->MoveTo(moveToPos);
		return true;
	}

	// Kickable blockers - Blockers are no longer kickable
	//DungeonEnemyBlocker* kickableBlocker = GetEnemyBlockerAtPosition(targetPosition);
	//if (kickableBlocker)
	//{
	//	DungeonTile* kickTile = GetTile(targetPosition);

	//	// see if the thing can move
	//	ivec2 moveToPos = targetPosition + directions[direction];

	//	if (!CanTraverse(targetPosition, direction))
	//		return false;

	//	// is there a tile?
	//	DungeonTile* toTile = GetTile(moveToPos);
	//	if (!toTile)
	//		return false;
	//	// is it occupied?
	//	if (toTile->occupied)
	//		return false; // We coudl do damage and shit here.

	//	// kick it in that direction
	//	kickTile->occupied = false;
	//	toTile->occupied = true;
	//	kickableBlocker->position = moveToPos;
	//	kickableBlocker->object->SetLocalPosition(dungeonPosToObjectScale(kickableBlocker->position));

	//	return true;
	//}

	// Kickable chasers
	DungeonEnemyChase* kickableChaser = GetEnemyChaseAtPosition(targetPosition);
	if (kickableChaser)
	{
		DungeonTile* kickTile = GetTile(targetPosition);

		// see if the thing can move
		ivec2 moveToPos = targetPosition + directions[direction];

		if (!CanTraverse(targetPosition, direction))
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
		kickableChaser->state = DungeonEnemyChase::STUN;
		kickableChaser->stateVisual = DungeonEnemyChase::KICKED;
		kickableChaser->moveCurrent = 0.0f;
		kickableChaser->oldPosition = dungeonPosToObjectScale(kickableChaser->position);
		kickableChaser->targetPosition = dungeonPosToObjectScale(moveToPos);
		kickableChaser->position = moveToPos;
		kickableChaser->positionWant = moveToPos;
		kickableChaser->animator->StartAnimation(kickableChaser->animationKickBack);
		return true;
	}

	// Kickable Mirror
	DungeonMirror* kickableMirror = GetMirrorAt(targetPosition);
	if (kickableMirror)
	{
		DungeonTile* kickTile = GetTile(targetPosition);

		// see if the thing can move
		ivec2 moveToPos = targetPosition + directions[direction];

		if (!CanTraverse(targetPosition, direction))
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
		kickableMirror->position = moveToPos;
		kickableMirror->object->SetLocalPosition(dungeonPosToObjectScale(kickableMirror->position));

		return true;
	}

	// Kickable Switcher
	DungeonEnemySwitcher* kickableSwitcher = GetEnemySwitcherAtPosition(targetPosition);
	if (kickableSwitcher)
	{
		DungeonTile* kickTile = GetTile(targetPosition);

		// see if the thing can move
		ivec2 moveToPos = targetPosition + directions[direction];

		if (!CanTraverse(targetPosition, direction))
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
		kickableSwitcher->position = moveToPos;
		kickableSwitcher->object->SetLocalPosition(dungeonPosToObjectScale(kickableSwitcher->position));

		return true;
	}
	
	return false;
}

Crawl::DungeonDoor* Crawl::Dungeon::CreateDoor(ivec2 position, unsigned int directionIndex, unsigned int id, bool open)
{
	DungeonDoor* door = new DungeonDoor();
	door->position = position;
	door->orientation = directionIndex;
	door->id = id;
	door->open = open;

	ordered_json door_objectJSON = ReadJSONFromDisk("crawler/object/interactable_door.object");
	Object* door_object = Scene::CreateObject();
	door_object->LoadFromJSON(door_objectJSON);
	door->object = door_object;
	door_object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	
	ordered_json door_modelJSON = ReadJSONFromDisk("crawler/model/door_door_frame.object");
	Object* door_model = door_object->children[0]->children[0];
	door_model->LoadFromJSON(door_modelJSON);
	door_object->SetLocalRotationZ(orientationEulers[directionIndex]);

	// Left Panel
	ordered_json door_leftModelJSON = ReadJSONFromDisk("crawler/model/door_door_left.object");
	Object* door_leftPanelJSON = Scene::CreateObject(door_object->children[0]->children[1]->children[0]);
	door_leftPanelJSON->LoadFromJSON(door_leftModelJSON);

	// Right panel
	ordered_json door_rightModelJSON = ReadJSONFromDisk("crawler/model/door_door_right.object");
	Object* door_rightPanelJSON = Scene::CreateObject(door_object->children[0]->children[2]->children[0]);
	door_rightPanelJSON->LoadFromJSON(door_rightModelJSON);
		
	
	door->UpdateTransforms(true);
	door->UpdateVisuals(0.0f);
	activatable.push_back(door);
	return door;
}

void Crawl::Dungeon::RemoveDoor(DungeonDoor* door)
{
	for (int i = 0; i < activatable.size(); i++)
	{
		if (activatable[i] == door)
		{
			delete activatable[i];
			activatable.erase(activatable.begin() + i);
			return;
		}
	}
}

Crawl::DungeonActivatorPlate* Crawl::Dungeon::CreatePlate(ivec2 position, unsigned int activateID)
{
	DungeonActivatorPlate* plate = new DungeonActivatorPlate();
	plate->position = position;
	plate->activateID = activateID;
	plate->dungeon = this;
	plate->object = Scene::CreateObject();
	plate->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/interactable_floortile.object"));
	plate->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_pressureplate.object"));
	plate->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	activatorPlates.push_back(plate);
	return plate;
}

void Crawl::Dungeon::RemovePlate(DungeonActivatorPlate* plate)
{
	for (int i = 0; i < activatorPlates.size(); i++)
	{
		if (activatorPlates[i] == plate)
		{
			delete activatorPlates[i];
			activatorPlates.erase(activatorPlates.begin() + i);
			return;
		}
	}
}

bool Crawl::Dungeon::IsPlateAtPosition(ivec2 position)
{
	for (int i = 0; i < activatorPlates.size(); i++)
	{
		if (activatorPlates[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonTransporter* Crawl::Dungeon::CreateTransporter(ivec2 position)
{
	DungeonTransporter* transporter = new DungeonTransporter();
	transporter->position = position;
	if (!fakeDungeon) // We dont need an object for this if its the fake lobby dungeon boiii
	{
		/*transporter->object = Scene::CreateObject();
		transporter->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/exit.object"));
		transporter->object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1 });*/
	}
	transporterPlates.push_back(transporter);
	return transporter;
}

void Crawl::Dungeon::RemoveTransporter(DungeonTransporter* transporter)
{
	for (int i = 0; i < transporterPlates.size(); i++)
	{
		if (transporterPlates[i] == transporter)
		{
			delete transporterPlates[i];
			transporterPlates.erase(transporterPlates.begin() + i);
			return;
		}
	}
}

Crawl::DungeonSpikes* Crawl::Dungeon::CreateSpikes(ivec2 position, bool disabled)
{
	DungeonSpikes* spikes = new DungeonSpikes();
	spikes->position = position;
	spikes->disabled = disabled;
	spikes->dungeon = this;
	spikes->object = Scene::CreateObject();
	spikes->object->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_trap_spikes.object"));
	spikes->UpdateTransform();


	// Should be able to remove this when new spike asset goes in. (and delete call in destructor)
	// some hack to remove the floor from the tile we're on
	DungeonTile* tile = GetTile(position);
	if (tile && tile->object->children[0]->children[0])
	{
		tile->object->children[0]->children[0]->markedForDeletion = true;
	}

	// more hack for placeholder black box
	spikes->placeHolderContainerObject = Scene::CreateObject();
	spikes->placeHolderContainerObject->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_trap_spike_tempblackcontainer.object"));
	spikes->placeHolderContainerObject->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 }); // until new asset is in

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

Crawl::DungeonSpikes* Crawl::Dungeon::GetSpikesAtPosition(ivec2 position)
{
	for (int i = 0; i < spikesPlates.size(); i++)
	{
		if (spikesPlates[i]->position == position) return spikesPlates[i];
	}

	return nullptr;
}

bool Crawl::Dungeon::IsSpikesAtPosition(ivec2 position)
{
	for (int i = 0; i < spikesPlates.size(); i++)
	{
		if (spikesPlates[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonPushableBlock* Crawl::Dungeon::CreatePushableBlock(ivec2 position)
{
	DungeonPushableBlock* pushable = new DungeonPushableBlock();
	pushable->dungeon = this;
	pushable->position = position;
	pushable->object = Scene::CreateObject();
	pushable->object->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_crate.object"));
	pushable->UpdateTransform();
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

bool Crawl::Dungeon::IsPushableBlockAtPosition(ivec2 position)
{
	for (int i = 0; i < pushableBlocks.size(); i++)
	{
		if (pushableBlocks[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonPushableBlock* Crawl::Dungeon::GetPushableBlockAtPosition(ivec2 position, bool includeDisabled)
{
	for (int i = 0; i < pushableBlocks.size(); i++)
	{
		if (pushableBlocks[i]->position == position)
		{
			if (includeDisabled)						return pushableBlocks[i];
			else if (!pushableBlocks[i]->isOnSpikes)	return pushableBlocks[i];
		}
	}

	return nullptr;
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
	shootLaser->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/monster_shootlaser.object"));
	shootLaser->UpdateTransform();

	Object* gargoyle = Scene::CreateObject(shootLaser->object->children[0]);
	gargoyle->LoadFromJSON(ReadJSONFromDisk("crawler/model/monster_gargoyle.object"));

	shootLasers.emplace_back(shootLaser);
	return shootLaser;
}

void Crawl::Dungeon::RemoveDungeonShootLaser(DungeonShootLaser* laser)
{
	for (int i = 0; i < shootLasers.size(); i++)
	{
		if (shootLasers[i] == laser)
		{
			delete shootLasers[i];
			shootLasers.erase(shootLasers.begin() + i);
			return;
		}
	}
}

void Crawl::Dungeon::CreateDamageVisual(ivec2 position, FACING_INDEX direction)
{
	DungeonDamageVisual* visual = new DungeonDamageVisual();
	visual->turnCreated = turn;
	visual->position = position;
	visual->object = Scene::CreateObject();
	//visual->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/damage_visual.object"));
	visual->object->LoadFromJSON(ReadJSONFromDisk("crawler/model/vfx_shoot_laser_shot.object"));
	visual->object->AddLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 1.2 });
	visual->object->SetLocalRotationZ(orientationEulersReversed[direction]);
	damageVisuals.emplace_back(visual);
}

void Crawl::Dungeon::CreateShootLaserProjectile(void* dealer, ivec2 position, FACING_INDEX direction)
{
	DamageAtPosition(position, dealer);
	
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
	blocker->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/monster_blocker.object"));
	blocker->UpdateTransform();
	blocker->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/monster_blocker_prototype.object"));
	blocker->animator = (ComponentAnimator*)blocker->object->children[0]->GetComponent(Component_Animator);
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

void Crawl::Dungeon::RemoveEnemyBlocker(DungeonEnemyBlocker* blocker)
{
	for (int i = 0; i < blockers.size(); i++)
	{
		if (blocker == blockers[i])
		{
			DungeonTile* tile = GetTile(blocker->position);
			if (tile)
				tile->occupied = false;

			delete blockers[i];
			blockers.erase(blockers.begin() + i);
			return;
		}
	}
}

bool Crawl::Dungeon::IsEnemyBlockerAtPosition(ivec2 position)
{
	for (int i = 0; i < blockers.size(); i++)
	{
		if (blockers[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonEnemyBlocker* Crawl::Dungeon::GetEnemyBlockerAtPosition(ivec2 position)
{
	for (int i = 0; i < blockers.size(); i++)
	{
		if (blockers[i]->position == position) return blockers[i];
	}

	return nullptr;
}

Crawl::DungeonEnemyChase* Crawl::Dungeon::CreateEnemyChase(ivec2 position, FACING_INDEX facing)
{
	DungeonEnemyChase* chaser = new DungeonEnemyChase();
	chaser->position = position;
	chaser->positionWant = position;
	chaser->facing = facing;
	chaser->dungeon = this;
	chaser->object = Scene::CreateObject();
	chaser->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/monster_chaser.object"));
	chaser->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/monster_chaser.object"));
	chaser->object->children[0]->SetLocalRotationZ(180);
	if (chaser->useAnimator)
	{
		chaser->animator = (ComponentAnimator*)chaser->object->children[0]->GetComponent(Component_Animator);
		chaser->animator->SetPose(chaser->animationActivate);
	}
	chasers.emplace_back(chaser);

	DungeonTile* tile = GetTile(position);
	if (tile)
	{
		tile->occupied = true;
	}
	else
		LogUtils::Log("WARNING - ATTEMPTING TO ADD CHASER TO TILE THAT DOESN'T EXIST");

	chaser->UpdateTransform();
	return chaser;
}

void Crawl::Dungeon::RemoveEnemyChase(DungeonEnemyChase* chaser)
{
	for (int i = 0; i < chasers.size(); i++)
	{
		if (chaser == chasers[i])
		{
			DungeonTile* tile = GetTile(chaser->position);
			if (tile)
				tile->occupied = false;

			delete chasers[i];
			chasers.erase(chasers.begin() + i);
			return;
		}
	}
}

bool Crawl::Dungeon::IsEnemyChaserAtPosition(ivec2 position)
{
	for (int i = 0; i < chasers.size(); i++)
	{
		if (chasers[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonEnemyChase* Crawl::Dungeon::GetEnemyChaseAtPosition(ivec2 position)
{
	for (int i = 0; i < chasers.size(); i++)
	{
		if (chasers[i]->position == position) return chasers[i];
	}

	return nullptr;
}

Crawl::DungeonEnemySwitcher* Crawl::Dungeon::CreateEnemySwitcher(ivec2 position, FACING_INDEX facing)
{
	DungeonEnemySwitcher* switcher = new DungeonEnemySwitcher();
	switcher->position = position;
	switcher->facing = facing;
	switcher->dungeon = this;
	switcher->object = Scene::CreateObject();
	switcher->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/monster_switcher.object"));
	switcher->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/monster_switchbitch.object"));
	switcher->object->children[0]->SetLocalRotationZ(180);
	switcher->UpdateTransform();
	switchers.emplace_back(switcher);

	DungeonTile* tile = GetTile(position);
	if (tile)
	{
		tile->occupied = true;
	}
	else
		LogUtils::Log("WARNING - ATTEMPTING TO ADD SWITCHER TO TILE THAT DOESN'T EXIST");

	return switcher;
}

void Crawl::Dungeon::RemoveEnemySwitcher(DungeonEnemySwitcher* switcher)
{
	for (int i = 0; i < switchers.size(); i++)
	{
		if (switcher == switchers[i])
		{
			DungeonTile* tile = GetTile(switcher->position);
			if (tile)
				tile->occupied = false;

			delete switchers[i];
			switchers.erase(switchers.begin() + i);
		}
	}
}

bool Crawl::Dungeon::IsEnemySwitcherAtPosition(ivec2 position)
{
	for (int i = 0; i < switchers.size(); i++)
	{
		if (switchers[i]->position == position) return true;
	}

	return false;
}

Crawl::DungeonEnemySwitcher* Crawl::Dungeon::GetEnemySwitcherAtPosition(ivec2 position)
{
	for (int i = 0; i < switchers.size(); i++)
	{
		if (switchers[i]->position == position) return switchers[i];
	}

	return nullptr;
}

Crawl::DungeonCheckpoint* Crawl::Dungeon::CreateCheckpoint(ivec2 position, FACING_INDEX facing, bool activated)
{
	DungeonCheckpoint* checkpoint = new DungeonCheckpoint();
	checkpoint->position = position;
	checkpoint->facing = facing;
	checkpoint->activated = activated;
	checkpoint->dungeon = this;
	/*checkpoint->object = Scene::CreateObject();
	checkpoint->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/checkpoint.object"));
	checkpoint->object->AddLocalPosition(dungeonPosToObjectScale(position));
	if (checkpoint->activated)
		checkpoint->SetActivatedMaterial();*/
	checkpoints.push_back(checkpoint);
	return checkpoint;
}

void Crawl::Dungeon::RemoveCheckpoint(DungeonCheckpoint* checkpoint)
{
	if (checkpoint == nullptr)
		return;

	for (int i = 0; i < checkpoints.size(); i++)
	{
		if (checkpoint == checkpoints[i])
		{
			delete checkpoint;
			checkpoints.erase(checkpoints.begin() + i);
		}
	}
}

Crawl::DungeonCheckpoint* Crawl::Dungeon::GetCheckpointAt(ivec2 position)
{
	for (int i = 0; i < checkpoints.size(); i++)
	{
		if (position == checkpoints[i]->position)
			return checkpoints[i];
	}

	return nullptr;
}

Crawl::DungeonMirror* Crawl::Dungeon::CreateMirror(ivec2 position, FACING_INDEX direction)
{
	DungeonMirror* mirror = new DungeonMirror();
	mirror->position = position;
	mirror->facing = direction;
	//mirror->dungeon = this;
	mirror->object = Scene::CreateObject();
	mirror->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/mirror.object"));
	mirror->UpdateTransform();
	mirrors.push_back(mirror);

	DungeonTile* tile = GetTile(position);
	if (tile)
	{
		tile->occupied = true;
	}
	else
		LogUtils::Log("WARNING - ATTEMPTING TO ADD MIRROR TO TILE THAT DOESN'T EXIST");

	return mirror;
}

void Crawl::Dungeon::RemoveMirror(DungeonMirror* mirror)
{
	for (int i = 0; i < mirrors.size(); i++)
	{
		if (mirrors[i] == mirror)
		{
			delete mirror;
			mirrors.erase(mirrors.begin() + i);

			DungeonTile* tile = GetTile(mirror->position);
			if (tile)
			{
				tile->occupied = true;
			}
			else
				LogUtils::Log("WARNING - ATTEMPTING TO REMOVE MIRROR TO TILE THAT DOESN'T EXIST");
			return;
		}
	}
}

Crawl::DungeonMirror* Crawl::Dungeon::GetMirrorAt(ivec2 position)
{
	for (int i = 0; i < mirrors.size(); i++)
	{
		if (mirrors[i]->position == position)
			return mirrors[i];
	}

	return nullptr;
}

bool Crawl::Dungeon::IsMirrorAtPosition(ivec2 position)
{
	DungeonMirror* mirror = GetMirrorAt(position);
	return mirror;
}

// direction should be a sign of a direction e.g. -1 or 1 to upadte the mirrors FACING_INDEX
void Crawl::Dungeon::RotateMirror(DungeonMirror* mirror, int direction)
{
	mirror->facing = (FACING_INDEX)(mirror->facing + direction);
	if (mirror->facing > 3)
		mirror->facing = (FACING_INDEX)0;
	else if (mirror->facing < 0)
		mirror->facing = (FACING_INDEX)3;

	mirror->object->SetLocalRotationZ(orientationEulers[(int)mirror->facing]);
}

Crawl::DungeonEnemySlug* Crawl::Dungeon::CreateMurderina(ivec2 position, FACING_INDEX direction)
{
	DungeonEnemySlug* slug = new DungeonEnemySlug();
	slug->position = position;
	slug->facing = direction;
	slug->dungeon = this;
	slug->object = Scene::CreateObject();
	slug->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/slug.object"));
	slug->object->children[0]->LoadFromJSON(ReadJSONFromDisk("crawler/model/monster_sawarina.object"));
	slug->object->children[0]->SetLocalRotationZ(180);
	slug->UpdateTransform();
	slugs.push_back(slug);

	DungeonTile* tile = GetTile(position);
	if (tile)
	{
		tile->occupied = true;
	}
	else
		LogUtils::Log("WARNING - ATTEMPTING TO ADD SLUG TO TILE THAT DOESN'T EXIST");

	return slug;
}

void Crawl::Dungeon::RemoveSlug(DungeonEnemySlug* slug)
{
	for (int i = 0; i < slugs.size(); i++)
	{
		if (slugs[i] == slug)
		{
			DungeonTile* tile = GetTile(slug->position);
			if (tile)
			{
				tile->occupied = false;
			}
			else
				LogUtils::Log("WARNING - ATTEMPTING TO REMOVE SLUG TO TILE THAT DOESN'T EXIST");

			delete slug;
			slugs.erase(slugs.begin() + i);
			return;
		}
	}
}

bool Crawl::Dungeon::IsSlugPath(ivec2 position)
{
	for (auto* path : slugPaths)
	{
		if (path->position == position)
			return true;
	}
	return false;
}

Crawl::DungeonEnemySlugPath* Crawl::Dungeon::GetSlugPath(ivec2 position)
{
	for (auto* path : slugPaths)
	{
		if (path->position == position)
			return path;
	}
	return nullptr;
}

Crawl::DungeonEnemySlugPath* Crawl::Dungeon::CreateSlugPath(ivec2 position)
{	
	if (IsSlugPath(position))
		return nullptr;

	DungeonEnemySlugPath* slugPath = new DungeonEnemySlugPath();
	slugPath->position = position;
	slugPath->dungeon = this;
	slugPaths.push_back(slugPath);
	return slugPath;
}

void Crawl::Dungeon::RemoveSlugPath(DungeonEnemySlugPath* slugPath)
{
	for (int i = 0; i < slugPaths.size(); i++)
	{
		if (slugPaths[i] == slugPath)
		{
			DungeonEnemySlugPath* toDelete = slugPaths[i];
			slugPaths.erase(slugPaths.begin() + i); // remove from list
			for (int j = 0; j < 4; j++)
			{
				if (toDelete->neighbors[j])
				{
					toDelete->neighbors[j]->neighbors[facingIndexesReversed[j]] = nullptr;
					toDelete->neighbors[j]->RefreshObject();
				}
			}
			delete toDelete;
			return;
		}
	}
	return;
}

Crawl::DungeonDecoration* Crawl::Dungeon::CreateDecoration(ivec2 position, FACING_INDEX facing)
{
	DungeonDecoration* decoration = new DungeonDecoration();
	decoration->position = position;
	decoration->facing = facing;
	decoration->object = Scene::CreateObject();
	decoration->object->LoadFromJSON(ReadJSONFromDisk("crawler/object/decoration.object"));
	decoration->UpdateTransform();
	decorations.push_back(decoration);
	return decoration;
}

void Crawl::Dungeon::RemoveDecoration(DungeonDecoration* decoration)
{
	for (int i = 0; i < decorations.size(); i++)
	{
		if (decorations[i] == decoration)
		{
			delete decorations[i];
			decorations.erase(decorations.begin() + i);
			break;
		}
	}
}

Crawl::DungeonStairs* Crawl::Dungeon::CreateStairs(ivec2 position)
{
	DungeonStairs* stair = new DungeonStairs();
	stair->startPosition = position;
	stairs.push_back(stair);
	return stair;
}

void Crawl::Dungeon::RemoveStairs(DungeonStairs* stair)
{
	for (int i = 0; i < stairs.size(); i++)
	{
		if (stairs[i] == stair)
		{
			delete stairs[i];
			stairs.erase(stairs.begin() + i);
			break;
		}
	}
}

Crawl::DungeonLight* Crawl::Dungeon::CreateLight(ivec2 position)
{
	DungeonLight* light = new DungeonLight();
	light->position = position;
	pointLights.push_back(light);
	light->Init();
	light->UpdateTransform();
	return light;
}

void Crawl::Dungeon::RemoveLight(DungeonLight* light)
{
	for (int i = 0; i < pointLights.size(); i++)
	{
		if (pointLights[i] == light)
		{
			delete pointLights[i];
			pointLights.erase(pointLights.begin() + i);
			break;
		}
	}
}
// ID defaults to 0, if 0 is used, it will flicker all lights. Otherwise, just flicker lights with matching ID.
void Crawl::Dungeon::FlickerLights(int id)
{
	for (auto& light : pointLights)
	{
		if(id == 0 ||light->id == id)
			light->Flicker();
	}
}

Crawl::DungeonEventTrigger* Crawl::Dungeon::CreateEventTrigger(ivec2 position)
{
	DungeonEventTrigger* event = new DungeonEventTrigger();
	event->dungeon = this;
	event->position = position;
	events.emplace_back(event);
	return event;
}

void Crawl::Dungeon::RemoveEventTrigger(DungeonEventTrigger* event)
{
	for (int i = 0; i < events.size(); i++)
	{
		if (events[i] == event)
		{
			delete events[i];
			events.erase(events.begin() + i);
			break;
		}
	}
}

void Crawl::Dungeon::DoEventTriggerFacing(ivec2 position, FACING_INDEX facing)
{
	for (auto& event : events)
	{
		if (event->mustBeFacing && event->position == position && event->facing == player->GetOrientation()) event->Activate();
	}
}

void Crawl::Dungeon::Save(std::string filename)
{
	SetDungeonNameFromFileName(filename);
	serialised = GetDungeonSerialised();
	WriteJSONToDisk(filename, serialised);
}

void Crawl::Dungeon::ClearDungeon()
{
	dungeonFileName = "";
	dungeonFilePath = "";
	dungeonSubFolder = "";
	defaultPlayerStartPosition = { 0,0 };
	defaultPlayerStartOrientation = EAST_INDEX;
	
	serialised.clear();

	DestroySceneFromDungeonLayout();
}

void Crawl::Dungeon::ResetDungeon()
{
	RebuildDungeonFromSerialised(serialised);
	player->ClearCheckpoint();
}

void Crawl::Dungeon::Load(std::string filename)
{
	ClearDungeon();

	SetDungeonNameFromFileName(filename);

	serialised = ReadJSONFromDisk(filename);
	RebuildDungeonFromSerialised(serialised);
}

bool Crawl::Dungeon::TestDungeonExists(std::string filename)
{
	return FileUtils::CheckFileExists(filename);
}

void Crawl::Dungeon::SetDungeonNameFromFileName(string filename)
{
	int lastSlash = filename.find_last_of('/');
	int extensionStart = filename.find_last_of('.');
	string name = filename.substr(lastSlash + 1, extensionStart - (lastSlash + 1));
	dungeonFileName = name;
	int subFolderSize = lastSlash - dungeonFileLocation.size();
	if (subFolderSize < 0) subFolderSize = 0;
	dungeonSubFolder = filename.substr(dungeonFileLocation.size(), subFolderSize);
	dungeonFilePath = filename;
}

ordered_json Crawl::Dungeon::GetDungeonSerialised()
{
	ordered_json dungeon_serialised;

	ordered_json tiles_json;

	dungeon_serialised["type"] = "dungeon";
	dungeon_serialised["version"] = version;
	dungeon_serialised["defaultPosition"] = defaultPlayerStartPosition;
	dungeon_serialised["defaultOrientation"] = defaultPlayerStartOrientation;
	if (noRoof) dungeon_serialised["noRoof"] = true;


	// These settings are now just stored on the player and locked in.
	/*dungeon_serialised["playerTurnIsFree"] = playerTurnIsFree;
	dungeon_serialised["playerHasKnife"] = playerHasKnife;
	dungeon_serialised["playerCanKickBox"] = playerCanKickBox;
	dungeon_serialised["playerCanPushBox"] = playerCanPushBox;
	dungeon_serialised["playerInteractIsFree"] = playerInteractIsFree;
	dungeon_serialised["switchersMustBeLookedAt"] = switchersMustBeLookedAt;
	dungeon_serialised["playerCanPushMirror"] = playerCanPushMirror;*/


	for (auto& x : tiles)
	{
		for (auto& y : x.second.row)
			tiles_json.push_back(y.second);
	}
	dungeon_serialised["tiles"] = tiles_json;

	ordered_json levers_json;
	for (auto& interactable : interactables)
		levers_json.push_back(*interactable);
	dungeon_serialised["levers"] = levers_json;

	ordered_json doors_json;
	for (auto& door : activatable)
		doors_json.push_back(*door);
	dungeon_serialised["doors"] = doors_json;

	ordered_json plates_json;
	for (auto& plate : activatorPlates)
		plates_json.push_back(*plate);
	dungeon_serialised["plates"] = plates_json;

	ordered_json transporters_json;
	for (auto& plate : transporterPlates)
		transporters_json.push_back(*plate);
	dungeon_serialised["transporters"] = transporters_json;

	ordered_json checkpoints_json;
	for (auto& checkpoint : checkpoints)
		checkpoints_json.push_back(*checkpoint);
	dungeon_serialised["checkpoints"] = checkpoints_json;

	ordered_json spikes_json;
	for (auto& spikes : spikesPlates)
		spikes_json.push_back(*spikes);
	dungeon_serialised["spikes"] = spikes_json;

	ordered_json blocks_json;
	for (auto& block : pushableBlocks)
		blocks_json.push_back(*block);
	dungeon_serialised["blocks"] = blocks_json;

	ordered_json shootLaser_json;
	for (auto& shootLaser : shootLasers)
		shootLaser_json.push_back(*shootLaser);
	dungeon_serialised["shootLasers"] = shootLaser_json;

	ordered_json blockers_json;
	for (auto& blocker : blockers)
		blockers_json.push_back(*blocker);
	dungeon_serialised["blockers"] = blockers_json;

	ordered_json chasers_json;
	for (auto& chaser : chasers)
		chasers_json.push_back(*chaser);
	dungeon_serialised["chasers"] = chasers_json;

	ordered_json slugs_json;
	for (auto& slug : slugs)
		slugs_json.push_back(*slug);
	dungeon_serialised["slugs"] = slugs_json;

	ordered_json slugPaths_json;
	for (auto& slugPath : slugPaths)
		slugPaths_json.push_back(*slugPath);
	dungeon_serialised["slugPaths"] = slugPaths_json;

	ordered_json switchers_json;
	for (auto& switcher : switchers)
		switchers_json.push_back(*switcher);
	dungeon_serialised["switchers"] = switchers_json;

	ordered_json mirrors_json;
	for (auto& mirror : mirrors)
		mirrors_json.push_back(*mirror);
	dungeon_serialised["mirrors"] = mirrors_json;

	ordered_json decorations_json;
	for (auto& decoration : decorations)
		decorations_json.push_back(*decoration);
	dungeon_serialised["decorations"] = decorations_json;

	ordered_json pointLights_json;
	for (auto& pointLight : pointLights)
		pointLights_json.push_back(*pointLight);
	dungeon_serialised["pointLights"] = pointLights_json;

	ordered_json events_json;
	for (auto& event : events)
		events_json.push_back(*event);
	dungeon_serialised["events"] = events_json;

	if (stairs.size() > 0)
	{
		ordered_json stairs_json;
		for (auto& stair : stairs)
			stairs_json.push_back(*stair);
		dungeon_serialised["stairs"] = stairs_json;
	}

	return dungeon_serialised;
}

void Crawl::Dungeon::RebuildDungeonFromSerialised(ordered_json& serialised)
{
	if(!fakeDungeon)
		DestroySceneFromDungeonLayout();

	int dungeonVersion = serialised["version"];

	if (serialised.contains("defaultPosition"))
		serialised.at("defaultPosition").get_to(defaultPlayerStartPosition);
	else
		defaultPlayerStartPosition = { 0,0 };

	if (serialised.contains("defaultOrientation"))
		serialised.at("defaultOrientation").get_to(defaultPlayerStartOrientation);
	else
		defaultPlayerStartOrientation = EAST_INDEX;

	if (serialised.contains("noRoof")) noRoof = true;
	else noRoof = false;

	// Player settings are no longer stored in dungeon files.
	/*if (serialised.contains("playerTurnIsFree"))
		serialised.at("playerTurnIsFree").get_to(playerTurnIsFree);
	else
		playerTurnIsFree = false;

	if (serialised.contains("playerInteractIsFree"))
		serialised.at("playerInteractIsFree").get_to(playerInteractIsFree);
	else
		playerInteractIsFree = false;

	if (serialised.contains("playerHasKnife"))
		serialised.at("playerHasKnife").get_to(playerHasKnife);
	else
		playerHasKnife = false;

	if (serialised.contains("playerCanKickBox"))
		serialised.at("playerCanKickBox").get_to(playerCanKickBox);
	else
		playerTurnIsFree = true;

	if (serialised.contains("playerCanPushBox"))
		serialised.at("playerCanPushBox").get_to(playerCanPushBox);
	else
		playerCanPushBox = false;

	if (serialised.contains("switchersMustBeLookedAt"))
		serialised.at("switchersMustBeLookedAt").get_to(switchersMustBeLookedAt);
	else
		switchersMustBeLookedAt = false;

	if (serialised.contains("playerCanPushMirror"))
		serialised.at("playerCanPushMirror").get_to(playerCanPushMirror);
	else
		playerCanPushMirror = false;*/

	// Static Objects
	auto& tiles_json = serialised["tiles"];
	for (auto it = tiles_json.begin(); it != tiles_json.end(); it++)
	{
		DungeonTile tile = it.value().get<Crawl::DungeonTile>();
		if (dungeonVersion == 1) // wallVariants were initially implemented kinda broken but are now being used to assist with untraversable but see through-able walls.
		{
			tile.wallVariants[0] = (tile.maskTraverse & NORTH_MASK) == NORTH_MASK ? -1 : 0;
			tile.wallVariants[1] = (tile.maskTraverse & EAST_MASK) == EAST_MASK ? -1 : 0;
			tile.wallVariants[2] = (tile.maskTraverse & SOUTH_MASK) == SOUTH_MASK ? -1 : 0;
			tile.wallVariants[3] = (tile.maskTraverse & WEST_MASK) == WEST_MASK ? -1 : 0;
		}
		AddTile(tile);
	}
	if(!fakeDungeon) BuildSceneFromDungeonLayout(); // this creates all the 3D visuals for the tiles, based on their neighhbors etc.

	auto& transporters_json = serialised["transporters"];
	for (auto it = transporters_json.begin(); it != transporters_json.end(); it++)
	{
		DungeonTransporter transporter = it.value().get<Crawl::DungeonTransporter>();
		DungeonTransporter* newTransporter = CreateTransporter(transporter.position);
		newTransporter->name = transporter.name;
		newTransporter->toDungeon = transporter.toDungeon;
		newTransporter->toTransporter = transporter.toTransporter;
		newTransporter->fromOrientation = transporter.fromOrientation;
		newTransporter->toLobby2 = transporter.toLobby2;
	}

	auto& checkpoints_json = serialised["checkpoints"];
	for (auto it = checkpoints_json.begin(); it != checkpoints_json.end(); it++)
	{
		DungeonCheckpoint checkpoint = it.value().get<Crawl::DungeonCheckpoint>();
		DungeonCheckpoint* newCheckpoint = CreateCheckpoint(checkpoint.position, checkpoint.facing);
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

	auto& decorations_json = serialised["decorations"];
	for (auto it = decorations_json.begin(); it != decorations_json.end(); it++)
	{
		DungeonDecoration decoration = it.value().get<Crawl::DungeonDecoration>();
		DungeonDecoration* newDecoration = CreateDecoration(decoration.position, decoration.facing);
		newDecoration->localPosition = decoration.localPosition;
		newDecoration->localRotation = decoration.localRotation;
		newDecoration->modelName = decoration.modelName;
		newDecoration->castsShadows = decoration.castsShadows;
		newDecoration->LoadDecoration();
		newDecoration->UpdateShadowCasting();
	}

	auto& slugPaths_json = serialised["slugPaths"];
	for (auto it = slugPaths_json.begin(); it != slugPaths_json.end(); it++)
	{
		DungeonEnemySlugPath slug = it.value().get<Crawl::DungeonEnemySlugPath>();
		DungeonEnemySlugPath* newSlug = CreateSlugPath(slug.position);
		newSlug->maskTraverse = slug.maskTraverse;
		if (newSlug->maskTraverse != -1) newSlug->RefreshObject();
	}

	if (dungeonVersion < 3)
	{
		for (auto& slugPath : slugPaths)
		{
			if (slugPath->maskTraverse == -1)
			{
				slugPath->AutoGenerateMask();
				slugPath->RefreshObject();
			}
		}
	}

	auto& spikes_json = serialised["spikes"];
	for (auto it = spikes_json.begin(); it != spikes_json.end(); it++)
	{
		DungeonSpikes spikes = it.value().get<Crawl::DungeonSpikes>();
		CreateSpikes(spikes.position);
	}

	if (!fakeDungeon) Scene::s_instance->SetAllObjectsStatic(); // None of this stuff moves, so can be marked as static.

	// Dynamic Objects
	auto& doors_json = serialised["doors"]; // Technically door frame are static, only the doors are dynamic - but the optimisation is not worth it
	for (auto it = doors_json.begin(); it != doors_json.end(); it++)
	{
		DungeonDoor door = it.value().get<Crawl::DungeonDoor>();
		CreateDoor(door.position, door.orientation, door.id, door.open);
	}

	auto& levers_json = serialised["levers"];
	for (auto it = levers_json.begin(); it != levers_json.end(); it++)
	{
		DungeonInteractableLever lever = it.value().get<Crawl::DungeonInteractableLever>();
		DungeonInteractableLever* newLever = CreateLever(lever.position, lever.orientation, lever.id, lever.activateID, lever.status);
	}

	auto& plates_json = serialised["plates"];
	for (auto it = plates_json.begin(); it != plates_json.end(); it++)
	{
		DungeonActivatorPlate plate = it.value().get<Crawl::DungeonActivatorPlate>();
		CreatePlate(plate.position, plate.activateID);
	}

	auto& blocks_json = serialised["blocks"];
	for (auto it = blocks_json.begin(); it != blocks_json.end(); it++)
	{
		DungeonPushableBlock block = it.value().get<Crawl::DungeonPushableBlock>();
		DungeonPushableBlock* newBlock = CreatePushableBlock(block.position);
		newBlock->MoveTo(newBlock->position, true); // We call move here just to have it check if its on top of spikes and handle that.
	}

	auto& blockers_json = serialised["blockers"];
	for (auto it = blockers_json.begin(); it != blockers_json.end(); it++)
	{
		DungeonEnemyBlocker blocker = it.value().get<Crawl::DungeonEnemyBlocker>();
		DungeonEnemyBlocker* newBlocker = CreateEnemyBlocker(blocker.position, blocker.facing);
		newBlocker->rapidAttack = blocker.rapidAttack;
	}

	auto& chasers_json = serialised["chasers"];
	for (auto it = chasers_json.begin(); it != chasers_json.end(); it++)
	{
		DungeonEnemyChase chaser = it.value().get<Crawl::DungeonEnemyChase>();
		DungeonEnemyChase* newChaser = CreateEnemyChase(chaser.position, chaser.facing);
		newChaser->state = chaser.state;
		newChaser->object->SetStatic(false);
	}

	auto& slugs_json = serialised["slugs"];
	for (auto it = slugs_json.begin(); it != slugs_json.end(); it++)
	{
		DungeonEnemySlug slug = it.value().get<Crawl::DungeonEnemySlug>();
		DungeonEnemySlug* newSlug = CreateMurderina(slug.position, slug.facing);
	}

	auto& switchers_json = serialised["switchers"];
	for (auto it = switchers_json.begin(); it != switchers_json.end(); it++)
	{
		DungeonEnemySwitcher switcher = it.value().get<Crawl::DungeonEnemySwitcher>();
		DungeonEnemySwitcher* newSwitcher = CreateEnemySwitcher(switcher.position, switcher.facing);
	}

	auto& mirrors_json = serialised["mirrors"];
	for (auto it = mirrors_json.begin(); it != mirrors_json.end(); it++)
	{
		DungeonMirror mirror = it.value().get<Crawl::DungeonMirror>();
		DungeonMirror* newMirror = CreateMirror(mirror.position, mirror.facing);
	}

	// Invisible objects
	auto& pointLights_json = serialised["pointLights"];
	for (auto it = pointLights_json.begin(); it != pointLights_json.end(); it++)
	{
		DungeonLight pointLight = it.value().get<Crawl::DungeonLight>();
		DungeonLight* newPointLight = CreateLight(pointLight.position);
		newPointLight->position = pointLight.position;
		newPointLight->localPosition = pointLight.localPosition;
		newPointLight->colour = pointLight.colour;
		newPointLight->intensity = pointLight.intensity;
		newPointLight->id = pointLight.id;
		newPointLight->isLobbyLight = pointLight.isLobbyLight;
		newPointLight->flickerRepeat = pointLight.flickerRepeat;
		newPointLight->flickerRepeatMin = pointLight.flickerRepeatMin;
		newPointLight->flickerRepeatMax = pointLight.flickerRepeatMax;
		newPointLight->flickerEnabled = pointLight.flickerEnabled;
		newPointLight->UpdateLight();
		newPointLight->UpdateTransform();
		newPointLight->ConfigureFlickerState();
	}

	auto& events_json = serialised["events"];
	for (auto it = events_json.begin(); it != events_json.end(); it++)
	{
		DungeonEventTrigger event = it.value().get<Crawl::DungeonEventTrigger>();
		DungeonEventTrigger* newEvent = CreateEventTrigger(event.position);
		newEvent->eventID = event.eventID;
		newEvent->repeats = event.repeats;
		newEvent->type = event.type;
		if (event.mustBeFacing)
		{
			newEvent->mustBeFacing = event.mustBeFacing;
			newEvent->facing = event.facing;

		}
	}

	auto& stairs_json = serialised["stairs"];
	for (auto it = stairs_json.begin(); it != stairs_json.end(); it++)
	{
		DungeonStairs stair = it.value().get<Crawl::DungeonStairs>();
		DungeonStairs* newStair = CreateStairs(stair.startPosition);
		newStair->endPosition = stair.endPosition;
		newStair->directionStart = stair.directionStart;
		newStair->directionEnd = stair.directionEnd;
		newStair->up = stair.up;
		newStair->startWorldPosition = stair.startWorldPosition;
		newStair->startOffset = stair.startOffset;
		newStair->endWorldPosition = stair.endWorldPosition;
		newStair->endOffset = stair.endOffset;
	}

	if(!fakeDungeon) Scene::s_instance->SetStaticObjectsDirty();
}

void Crawl::Dungeon::InitialiseTileMap()
{
	// Load the JSON template
	ordered_json tile_layout = ReadJSONFromDisk("crawler/object/tile_layout.object");
	ordered_json tile_ground1 = ReadJSONFromDisk("crawler/model/tile_wood.object");

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
	tiles.clear();

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

	for (int i = 0; i < checkpoints.size(); i++)
		delete checkpoints[i];
	checkpoints.clear();

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

	for (int i = 0; i < chasers.size(); i++)
		delete chasers[i];
	chasers.clear();

	for (int i = 0; i < slugs.size(); i++)
		delete slugs[i];
	slugs.clear();

	for (int i = 0; i < slugPaths.size(); i++)
		delete slugPaths[i];
	slugPaths.clear();

	for (int i = 0; i < switchers.size(); i++)
		delete switchers[i];
	switchers.clear();

	for (int i = 0; i < mirrors.size(); i++)
		delete mirrors[i];
	mirrors.clear();

	for (auto& pillar : pillars)
		pillar.second->markedForDeletion = true;
	pillars.clear();

	for (int i = 0; i < decorations.size(); i++)
		delete decorations[i];
	decorations.clear();
	
	for (int i = 0; i < pointLights.size(); i++)
		delete pointLights[i];
	pointLights.clear();

	for (int i = 0; i < events.size(); i++)
		delete events[i];
	events.clear();

	for (int i = 0; i < stairs.size(); i++)
		delete stairs[i];
	stairs.clear();
}

Object* Crawl::Dungeon::GetTileTemplate(int maskTraverse)
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

	// Events
	for (auto& event : events)
	{
		if (event->position == player->GetPosition())
		{
			if(!event->mustBeFacing) event->Activate();
			else if(event->facing == player->GetOrientation()) event->Activate();
		}
	}

	// slug logic
	for (auto& slug : slugs)
		slug->Update();

	// Chaser Logic
	for (auto& chaser : chasers)
	{

		if(!chaser->isDead) chaser->Update();
	}

	// Check for chaser clashes
	if (chasers.size() > 1)
	{
		bool clash = true;
		while (clash)
		{
			clash = false;
			for (int a = 0; a < chasers.size()-1; a++)
			{
				for (int b = 1; b < chasers.size(); b++)
				{
					if (chasers[a]->positionWant == chasers[b]->positionWant)
					{
						clash = true;
						if (chasers[a]->positionWant != chasers[a]->position)
						{
							chasers[a]->oldPosition = chasers[a]->object->localPosition;
							chasers[a]->targetPosition = dungeonPosToObjectScale(chasers[a]->positionWant);
							chasers[a]->stateVisual = DungeonEnemyChase::BOUNCING;
							chasers[a]->positionWant = chasers[a]->position;
							chasers[a]->bounceCurrent = 0.0f;
						}
						
						if (chasers[b]->positionWant != chasers[b]->position)
						{
							chasers[b]->oldPosition = chasers[b]->object->localPosition;
							chasers[b]->targetPosition = dungeonPosToObjectScale(chasers[b]->positionWant);
							chasers[b]->stateVisual = DungeonEnemyChase::BOUNCING;
							chasers[b]->positionWant = chasers[b]->position;
							chasers[b]->bounceCurrent = 0.0f;
						}
					}
				}
			}
		}
	}

	for (auto& chaser : chasers)
	{
		if(chaser->positionWant != chaser->position)
			chaser->ExecuteMove();
	}
	for (auto& chaser : chasers)
		chaser->ExecuteDamage();

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
		if (!spikes->disabled) DamageAtPosition(spikes->position, spikes);
	}

	// All Sword Blocker Enemies Update
	for (auto& blocker : blockers)
		blocker->Update();

	for (auto& switcher : switchers)
		switcher->Update();

	player->UpdatePointOfInterestTilt();
}

void Crawl::Dungeon::UpdateVisuals(float delta)
{
	for (int i = 0; i < chasers.size(); i++)
	{
		chasers[i]->UpdateVisuals(delta);
		{
			if (chasers[i]->stateVisual == Crawl::DungeonEnemyChase::DYING && chasers[i]->isDead)
			{
				RemoveEnemyChase(chasers[i]);
				i--;
			}
		}
	}

	for (int i = 0; i < slugs.size(); i++)
	{
		slugs[i]->UpdateVisuals(delta);
		{
			if (slugs[i]->isDead && slugs[i]->state == Crawl::DungeonEnemySlug::IDLE)
			{
				RemoveSlug(slugs[i]);
				i--;
			}
		}
	}
	
	for (auto& slug : slugs)
		slug->UpdateVisuals(delta);

	for (int i = 0; i < pushableBlocks.size(); i++)
	{
		pushableBlocks[i]->UpdateVisuals(delta);
		if (pushableBlocks[i]->isDead && pushableBlocks[i]->state == DungeonPushableBlock::STATE::IDLE)
		{
			RemovePushableBlock(pushableBlocks[i]->position);
			i--;
		}
	}

	for (auto& door : activatable)
		door->UpdateVisuals(delta);

	for (auto& lever : interactables)
		lever->UpdateVisuals(delta);

	for (auto& light : pointLights)
		light->UpdateVisual(delta);
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

bool Crawl::Dungeon::ShouldHavePillar(ivec2 coordinate)
{
	// check North East Tile
	DungeonTile* tileCheck = GetTile(coordinate + pillarToTileCoordinates[NORTHEAST_INDEX]);
	if (tileCheck && (tileCheck->wallVariants[WEST_INDEX] >= 0 || tileCheck->wallVariants[SOUTH_INDEX] >= 0)) return true;

	// check South East Tile
	tileCheck = GetTile(coordinate + pillarToTileCoordinates[SOUTHEAST_INDEX]);
	if (tileCheck && (tileCheck->wallVariants[NORTH_INDEX] >= 0 || tileCheck->wallVariants[WEST_INDEX] >= 0)) return true;

	// check South West Tile
	tileCheck = GetTile(coordinate + pillarToTileCoordinates[SOUTHWEST_INDEX]);
	if (tileCheck && (tileCheck->wallVariants[EAST_INDEX] >= 0 || tileCheck->wallVariants[NORTH_INDEX] >= 0)) return true;

	// check North West Tile
	tileCheck = GetTile(coordinate + pillarToTileCoordinates[NORTHWEST_INDEX]);
	if (tileCheck && (tileCheck->wallVariants[SOUTH_INDEX] >= 0 || tileCheck->wallVariants[EAST_INDEX] >= 0)) return true;

	return false;	
}

void Crawl::Dungeon::UpdatePillarsForTileCoordinate(ivec2 coordinate)
{
	for (int i = 0; i < 4; i++)
	{
		ivec2 pillarCoordinate = coordinate + tileToPillarCoordinates[i];
		if (ShouldHavePillar(pillarCoordinate))
		{
			// check if there is a pillar in the pillars list
			auto pillar = pillars.find(pillarCoordinate);
			// if not, add it
			if (pillar == pillars.end())
			{
				Object* pillarObj = Scene::CreateObject(pillarsParentObject);
				pillarObj->LoadFromJSON(ReadJSONFromDisk("crawler/model/tile_pillar.object"));
				pillarObj->SetLocalPosition(dungeonPosToObjectScale(coordinate));
				pillarObj->AddLocalPosition({ directionsDiagonal[i].x, directionsDiagonal[i].y, 0 });
				pillars.emplace(pillarCoordinate, pillarObj);
			}
		}
		else
		{
			// check if there is a pillar in the pillars list
			auto pillar = pillars.find(pillarCoordinate);
			// if not, add it
			if (pillar != pillars.end())
			{
				// if there is, remove it.
				pillar->second->markedForDeletion = true;
				pillars.erase(pillar);
			}
		}
	}
}