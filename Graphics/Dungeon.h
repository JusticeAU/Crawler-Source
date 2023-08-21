#pragma once

#include "DungeonHelpers.h"
#include "DungeonTile.h"

#include <string>
#include <map>
#include <vector>

using glm::ivec2;
using std::string;


namespace Crawl
{
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
	class DungeonEnemyChase;
	class DungeonEnemySwitcher;
	class DungeonCheckpoint;
	class DungeonMirror;
	class DungeonEnemySlug;
	class DungeonEnemySlugPath;
	class DungeonDecoration;

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
		DungeonTile* AddTile(DungeonTile& dungeonTile);
		void UpdateTileNeighbors(ivec2 position);

		bool FindPath(ivec2 from, ivec2 to, int facing = -1);
		static bool CostToPlayerGreaterThan(const DungeonTile* first, const DungeonTile* second)
		{
			if (first->cost > second->cost) return true;
			else return false;
		}

		bool SetTileMask(ivec2 position, int mask);
		DungeonTile* GetTile(ivec2 pos);
		// Deletes a dungeonTile from the grid. returns true if a deletion happened.
		bool DeleteTile(ivec2 position);

		// Ensures the dungeonTile node has a visual representation in the Scene Graph.
		void CreateTileObject(DungeonTile* tile);


		bool IsOpenTile(ivec2 position);

		bool HasLineOfSight(ivec2 fromPos, int directionIndex);
		bool PlayerCanMove(ivec2 fromPos, int directionIndex);
		bool IsDoorBlocking(DungeonTile* fromTile, int directionIndex);

		void SetPlayer(DungeonPlayer* player) { this->player = player; }

		bool DoInteractable(unsigned int id);
		void DoActivate(unsigned int id);
		void DoActivate(unsigned int id, bool on);

		// Returns true if this hit something
		bool DamageAtPosition(ivec2 position, void* dealer, bool fromPlayer = false);
		bool DoKick(ivec2 position, FACING_INDEX facing);
		
		DungeonInteractableLever* CreateLever(ivec2 position, unsigned int directionMask, unsigned int id, unsigned int doorID, bool startStatus);
		void RemoveLever(DungeonInteractableLever* lever);

		DungeonDoor* CreateDoor(ivec2 position, unsigned int directionMask, unsigned int id, bool startOpen);
		void RemoveDoor(DungeonDoor* door);

		DungeonActivatorPlate* CreatePlate(ivec2 position, unsigned int activateID);
		void RemovePlate(DungeonActivatorPlate* plate);

		DungeonTransporter* CreateTransporter(ivec2 position);
		void RemoveTransporter(DungeonTransporter* transporter);
		DungeonTransporter* GetTransporter(string transporterName);

		DungeonSpikes* CreateSpikes(ivec2 position, bool disabled = false);
		void RemoveSpikes(ivec2 position);
		bool IsSpikesAtPosition(ivec2 position);

		DungeonPushableBlock* CreatePushableBlock(ivec2 position);
		void RemovePushableBlock(ivec2 position);
		bool IsPushableBlockAtPosition(ivec2 position);
		DungeonPushableBlock* GetPushableBlockAtPosition(ivec2 position);

		DungeonShootLaser* CreateShootLaser(ivec2 position, FACING_INDEX facing, unsigned int id);
		void RemoveDungeonShootLaser(DungeonShootLaser* laser);

		void CreateDamageVisual(ivec2 position, bool fromPlayer = false);
		void CreateShootLaserProjectile(void* dealer, ivec2 position, FACING_INDEX direction);

		DungeonEnemyBlocker* CreateEnemyBlocker(ivec2 position, FACING_INDEX direction);
		void RemoveEnemyBlocker(DungeonEnemyBlocker* blocker);
		bool IsEnemyBlockerAtPosition(ivec2 position);

		DungeonEnemyChase* CreateEnemyChase(ivec2 position, FACING_INDEX direction);
		void RemoveEnemyChase(DungeonEnemyChase* chaser);
		bool IsEnemyChaserAtPosition(ivec2 position);
		DungeonEnemyChase* GetEnemyChaseAtPosition(ivec2 position);

		DungeonEnemySwitcher* CreateEnemySwitcher(ivec2 position, FACING_INDEX direction);
		void RemoveEnemySwitcher(DungeonEnemySwitcher* switcher);
		bool IsEnemySwitcherAtPosition(ivec2 position);

		DungeonCheckpoint* CreateCheckpoint(ivec2 position, FACING_INDEX direction, bool activated = false);
		void RemoveCheckpoint(DungeonCheckpoint* checkpoint);
		DungeonCheckpoint* GetCheckpointAt(ivec2 position);

		DungeonMirror* CreateMirror(ivec2 position, FACING_INDEX direction);
		void RemoveMirror(DungeonMirror* mirror);
		DungeonMirror* GetMirrorAt(ivec2 position);
		void RotateMirror(DungeonMirror* mirror, int direction);

		DungeonEnemySlug* CreateSlug(ivec2 position, FACING_INDEX direction);
		void RemoveSlug(DungeonEnemySlug* slug);
		
		DungeonEnemySlugPath* CreateSlugPath(ivec2 position);
		void RemoveSlugPath(DungeonEnemySlugPath* slugPath);
		bool IsSlugPath(ivec2 position);
		DungeonEnemySlugPath* GetSlugPath(ivec2 position);

		DungeonDecoration* CreateDecoration(ivec2 position, FACING_INDEX facing);
		void RemoveDecoration(DungeonDecoration* decoration);
	
		void Save(std::string filename);
		void Load(std::string filename);
		bool TestDungeonExists(std::string filename);

		ordered_json GetDungeonSerialised();
		void RebuildDungeonFromSerialised(ordered_json& serialised);
	
		// Calculates the tile mask based on adjacent tiles
		unsigned int GetAutoTileMask(ivec2 position);
		// returns pointer to the template tile for the Scene to duplicate.
		Object* GetTileTemplate(int mask);
		void SetParentTileObject(Object* object);

		static unsigned int GetReverseDirectionMask(unsigned int direction);

		void Update();
		void UpdateVisuals(float delta);

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

		// Pillars
		// https://stackoverflow.com/questions/62103494/using-stdmap-with-a-glmivec2-as-keyvalue
		// to be able to store vec2s in a map
		struct ivec2Compare {
			bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
				return (((std::int64_t)a.x << 30) + a.y) < (((std::int64_t)b.x << 30) + b.y);
			}
		};
		std::map<ivec2, Object*, ivec2Compare> pillars;

		bool ShouldHavePillar(ivec2 coordinate);
		void UpdatePillarsForTileCoordinate(ivec2 coordinate);
	public:
		ordered_json serialised;
		string dungeonFileExtension = ".dungeon";
		string dungeonFileName = "";
		string dungeonFilePath = "";

		unsigned int turn = 0;
		ivec2 defaultPlayerStartPosition = { 0,0 };
		FACING_INDEX defaultPlayerStartOrientation = EAST_INDEX;
		
		bool playerHasKnife = false;
		bool playerCanKickBox = false;
		bool playerCanPushBox = false;
		bool playerTurnIsFree = true;
		bool playerInteractIsFree = true;
		bool switchersMustBeLookedAt = true;
		bool playerCanPushMirror = false;

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

		std::vector<DungeonEnemyChase*> chasers;
		std::vector<DungeonEnemySwitcher*> switchers;
		std::vector<DungeonEnemySlug*> slugs;
		std::vector<DungeonEnemySlugPath*> slugPaths;

		std::vector<DungeonCheckpoint*> checkpoints;

		std::vector<DungeonMirror*> mirrors;

		std::vector<DungeonDecoration*> decorations;
		
		DungeonPlayer* player = nullptr;


		// Path Finding Visual Test
		std::vector<DungeonTile*> goodPath;
		std::vector<DungeonTile*> badNodes;
	};
}

