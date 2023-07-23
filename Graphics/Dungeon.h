#pragma once

#include "DungeonHelpers.h"
#include "DungeonTile.h"

#include <string>
#include <map>
#include <vector>

using glm::ivec2;


namespace Crawl
{
	const int DUNGEON_GRID_SCALE = 5;
	class DungeonPlayer;
	class DungeonInteractable;
	class DungeonInteractableLever;
	class DungeonDoor;
	class DungeonActivatorPlate;
	class DungeonTransporter;
	class DungeonSpikes;
	class DungeonPushableBlock;
	class DungeonShootLaser;
	class DungeonDamageVisual;
	class DungeonShootLaserProjectile;
	class DungeonEnemyBlocker;

	struct Column
	{
		std::map<int, DungeonTile> row;
	};
	class Dungeon
	{
	public:
		Dungeon();
		// Tile manipulation
		DungeonTile* AddTile(ivec2 position);
		void AddTile(DungeonTile& dungeonTile);

		bool SetTileMask(ivec2 position, int mask);
		DungeonTile* GetTile(ivec2 pos);
		// Deletes a dungeonTile from the grid. returns true if a deletion happened.
		bool DeleteTile(ivec2 position);

		// Ensures the dungeonTile node has a visual representation in the Scene Graph.
		void CreateTileObject(DungeonTile* tile);


		bool IsOpenTile(ivec2 position);

		bool HasLineOfSight(ivec2 fromPos, int directionIndex);
		bool CanMove(ivec2 fromPos, int directionIndex);

		void SetPlayer(DungeonPlayer* player) { this->player = player; }

		bool DoInteractable(unsigned int id);
		DungeonInteractableLever* CreateLever(ivec2 position, unsigned int directionMask, unsigned int id, unsigned int doorID, bool startStatus);
		void DoActivate(unsigned int id);
		void DoActivate(unsigned int id, bool on);
		
		// Returns true if this hit something
		bool DamageAtPosition(ivec2 position, bool fromPlayer = false);
		bool DoKick(ivec2 position, FACING_INDEX facing);

		DungeonDoor* CreateDoor(ivec2 position, unsigned int directionMask, unsigned int id, bool startOpen);
		DungeonActivatorPlate* CreatePlate(ivec2 position, unsigned int activateID);
		DungeonTransporter* CreateTransporter(ivec2 position);
		DungeonSpikes* CreateSpikes(ivec2 position);
		void RemoveSpikes(ivec2 position);
		DungeonPushableBlock* CreatePushableBlock(ivec2 position);
		void RemovePushableBlock(ivec2 position);
		DungeonTransporter* GetTransporter(string transporterName);

		DungeonShootLaser* CreateShootLaser(ivec2 position, FACING_INDEX facing, unsigned int id);
		void RemoveDungeonShootLaser(ivec2 position);
		void CreateDamageVisual(ivec2 position, bool fromPlayer = false);
		void CreateShootLaserProjectile(ivec2 position, FACING_INDEX direction);

		DungeonEnemyBlocker* CreateEnemyBlocker(ivec2 position, FACING_INDEX direction);
		void RemoveEnemyBlocker(ivec2 position);
	
		void Save(std::string filename);
		void Load(std::string filename);

		void BuildSerialised();
		void RebuildFromSerialised();
	
		// Calculates the tile mask based on adjacent tiles
		unsigned int GetAutoTileMask(ivec2 position);
		// returns pointer to the template tile for the Scene to duplicate.
		Object* GetTileTemplate(int mask);
		void SetParentTileObject(Object* object);

		static unsigned int GetReverseDirectionMask(unsigned int direction);

		void Update();

	protected:
		void InitialiseTileMap();

		// Before loading a dungeon, this will mark every scene object for deletion before clearing off the tiles.
		void DestroySceneFromDungeonLayout();
		// After loading a dungeon, this will build it in the Scene graph based on tile adjacency. used on editor and playmode dungeon loading.
		void BuildSceneFromDungeonLayout();

		const int version = 1; // increment this when the .dungeon file schema changes and ensure backwards compatibility.
		std::map<int, Column> tiles;
		Object* tile_template;
		Object* tilesParentObject = nullptr;
	public:
		ordered_json serialised;

		unsigned int turn = 0;
		ivec2 defaultPlayerStartPosition = { 0,0 };
		FACING_INDEX defaultPlayerStartOrientation = EAST_INDEX;
		
		bool playerHasKnife = false;
		bool playerCanKickBox = false;
		bool playerCanPushBox = false;
		bool playerTurnIsFree = true;

		std::vector<DungeonInteractableLever*> interactables;
		std::vector<DungeonDoor*> activatable;
		std::vector<string> wallVariantPaths;

		// Test on movement
		std::vector<DungeonActivatorPlate*> activatorPlates;
		std::vector<DungeonTransporter*> transporterPlates;
		std::vector<DungeonSpikes*> spikesPlates;
		std::vector<DungeonPushableBlock*> pushableBlocks;
		std::vector<DungeonShootLaser*> shootLasers;
		std::vector<DungeonShootLaserProjectile*> shootLaserProjectiles;
		std::vector<DungeonDamageVisual*> damageVisuals;
		std::vector<DungeonEnemyBlocker*> blockers;
		
		DungeonPlayer* player = nullptr;
	};
}

