/**___________________________________________________________________________/
@file       MapManager.h
@author     t.junjie@digipen.edu
@date       03/2/2026	(DD/MM/YYYY)
@brief		Manager that handles manages and spawns a grid-based map based on
			user input. Handles spawning of enemies and applying gimmick for
			each tile.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include <string>
#include "texture.h"
#include "MathLib.h"
#include <unordered_map>
#include <random>
#include "GameObjects.h"
#include "AchievementManager.h"


// Enum for different tile gimmicks
enum class GimmickType {
	NONE = 0,			// No Gimmick
	FAST_ENEMIES,		// Enemies speed will be increased
	SWARM,				// More enemies will be spawned
	BLIND,				// Player will be blinded, visual effect
};

enum class GimmickWeights : int {
	NONE = 70,
	FAST_ENEMIES = 15,
	SWARM = 10,
	BLIND = 5
};
// Item tier class, higher the number the better the items
enum class ItemTier {
	NONE = 0, TIER1, TIER2, TIER3, TIER4
};

// The different type of tiles available
enum class TileType {
	START, END, TREASURE, COMBAT
};
// The tile archetype, read from MapData.json
struct TileData {
	std::string tileName{};							// name of the tile archetype
	std::vector<std::string> prefabsName{};						// name of the tile archetype prefab to use
	std::pair<TextureObj*, std::string> sprite{ &CEO::Get<ResourceManager>()->GetErrorTex(), "" };	// The sprite texture of the tile archetype
	//TextureObj* sprite{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };	
	TileType type{ TileType::COMBAT};				// the type of the tile
	bool unlocked{ true };							// Whether the tile is unlocked or locked (based on achievements) [CURRENTLY NOT USED]
	std::vector<std::pair<int, int>> tilePosition;	// The shape of the tile archetype in grid coordinates
	bool firstEntry{ true };						// whether it is the first time the player is entering the tile.
};
// Tiles generated for user to choice and place down
struct TileChoice {
	const TileData* tileData{ nullptr };			// pointer to the tile archetype this tile is based on
	int prefabIndex{};
	EnemyType enemyType{ EnemyType::NONE };			// randomly generated enemy type that this tile will spawn
	GimmickType gimmickType{ GimmickType::NONE };	// randomly generated gimmick for this tile
	ItemTier itemTier{ ItemTier::NONE };			// randomly generated tier of items that this tile will contain [NOT IMPLEMENTED CURRENTLY]
};
// THe instance of the tile (tiles that have been placed and exist in the game)
struct TileInstance {
	const TileData* tileData{ nullptr };			// pointer to the tile archetype this tile instance is based on
	std::pair<int, int> anchor{ 0, 0 };				// The anchor that the tileData's position is based on
	int prefabIndex{};
	EnemyType enemyType{ EnemyType::NONE };			// The enemy type for this tile instance
	GimmickType gimmickType{ GimmickType::NONE };	// The gimmick for this tile instance
	ItemTier itemTier{ ItemTier::NONE };			// The tier of items that this tiile will contain [NOT CURRENTLY IMPLEMENTED]
	Registry::Entity tileId{ 0 };					// the entity id of this tile instance
	bool cleared{ false };							// Whether the tile has been cleared (all enemies killed).
};

// used to denote the result of place tile function
enum class PlaceTileResult {
	SUCCESS = 0,		// Tile placement was successful
	OUTOFBOUNDS,		// Tile placement was out of map grid bounds
	OVERLAP,			// Tile placement overlaps another existing tile
	INVALID_CONNECTION	// Tile placement not connected to starting tile
};
// Response given from PlaceTile() function
struct PlaceTileResponse {
	PlaceTileResult result;	// The result of the tile placement
	int index;				// Index of the tile instance if successful, -1 if unsuccessful
};

struct WaveInfo {
	std::vector<std::string> enemiesPrefab;				// the prefab name of the enemies for this waave info
	std::vector<float> enemiesWeight;					// the spawn weightage of the enemies
	std::discrete_distribution<> enemyDistribution;		// the enemy distribution built with enemiesWeight (built during loading)
	int totalEnemies{};									// the total number of enemies in this wave
	int maxSpawnAmount{};								// max amount of enemies that can be alive at a time
	float spawnDelay{};									// spawning delay
	float waveDelay{};									// delay before next wave commences
	bool cleared{ false };								// whether this wave has been cleared
};

class MapManager {
public:
	using GridPos = std::pair<int, int>;

	/*!
	* \brief Initialize MapManager.
	* \param[in] mapData - The JSON file to read the map data (tile archetypes) from.
	*/
	void Init(const std::string& mapData, const std::string& waveData);
	/*!
	* \brief Loads in the tile data from JSON into memory.
	*/
	bool LoadTileData();
	/*!
	* \brief Loads in the wave data from JSON into memory.
	*/
	bool LoadWaveData();

	/*!
	* \brief 
	*	Creates the starting 3 tiles (start tile, end tile, treasure tile)
	*	and generate their position on the map grid.
	*/
	void InitStartingMapData();

	/*!
	* \brief Generates 3 tiles for players to use to build their level
	*/
	void GenerateTileOffers();

	/*!
	* \brief 
	*	Check whether the tile path is valid (is connected to starting tile)
	*	using BFS. Used to check if tiles are placeable on map grid.
	* \param[in] tileData	- The tile data.
	* \param[in] anchor		- The bottom left anchor of the tile.
	* \return - Whether the path is valid.
	*/
	bool CheckPathValidity(const TileData* tileData, const GridPos& anchor);

	/*!
	* \brief
	*	Check whether the tile path is valid (is connected to starting tile)
	*	using BFS. Used to check if tiles are placeable on map grid.
	* \param[in] startTileIndex	- The tile instance index for the starting tile of the path.
	* \param[in] endTileIndex	- The tile instance index for the ending tile of the path.
	* \return - Whether the path is valid.
	*/
	bool CheckPathValidity(int startTileIndex, int endTileIndex);

	/*!
	* \brief Get the tile instance on specified map grid position
	* \param[in] x	- The x position in the map grid.
	* \param[in] y	- The y position in the map grid.
	* \return - A pointer to the tile instance, nullptr if no tile is in specified position.
	*/
	TileInstance* GetTile(int x, int y);

	/*!
	* \brief Get the tile instance based on tile type
	* \param[in] tileType	- The tile type to get.
	* \return - A pointer to the tile instance for tiles that are not COMBAT type. 
	*	COMBAT tile type returns a nullptr.
	*/
	TileInstance* GetTile(TileType tileType);

	/*!
	* \brief Get the tile instance based on world position.
	* \param[in] worldPos - The world position of the tile to get.
	* \return - A pointer to the tile instance if found, else nullptr.
	*/
	TileInstance* GetTile(const Vec2& worldPos);

	/*!
	* \brief Get the tile instance with index.
	* \param[in] index	- The index into tileInstances vector.
	* \return - A pointer to the tile instance, nullptr if index is out of range.
	*/
	TileInstance* GetTileInstance(int index);

	/*!
	* \brief
	*	Returns the nearby tiles of the specified tile in grid position.
	*	
	*	index 0: LEFT
	*	index 1: RIGHT
	*	index 2: UP
	*	index 3: DOWN
	* 
	* \param[in] x - The x grid coordinate of the tile to get ajacent tiles of.
	* \param[in] y - The y grid coordinate of the tile to get adjacent tiles of.
	* \return - A vector that contains the tile instance indices of the nearby tiles.
	*/
	std::vector<int> GetNearbyTiles(int x, int y);
	/*!
	* \brief
	*	Returns the nearby tiles of the specified tile in grid position.
	*
	*	index 0: LEFT
	*	index 1: RIGHT
	*	index 2: UP
	*	index 3: DOWN
	*
	* \param[in] gridPos - The x and y grid coordinate of the tile to get ajacent tiles of.
	* \return - A vector that contains the tile instance indices of the nearby tiles.
	*/
	std::vector<int> GetNearbyTiles(const GridPos& gridPos);

	/*!
	* \brief Get the current number of tile instances.
	* \return - The current number of tile instances.
	*/
	size_t GetTileInstanceSize() const;

	/*!
	* \brief Place tile onto our map.
	* \param[in] tile	- The tile choice that is being placed.
	* \param[in] anchor - The offset of the tile into the map grid, 
	*	measured from the the bottom left of the tile
	* \return - The result from the placement.
	*/
	PlaceTileResponse PlaceTile(const TileChoice& tile, const GridPos& anchor);
	/*!
	* \brief Place tile onto our map.
	* \param[in] tile	- The tile choice that is being placed.
	* \param[in] x		- The x offset of the tile into the map grid,
	*	measured from the the bottom left of the tile
	* \param[in] y		- The y offset of the tile into the map grid,
	*	measured from the the bottom left of the tile
	* \return - The result from the placement.
	*/
	PlaceTileResponse PlaceTile(const TileChoice& tile, int x, int y);

	/*!
	* \brief Removes the specified tile instance.
	* \param[in] index	- The index into tileInstances vector.
	*/
	void RemoveTile(int index);

	/*!
	* \brief To set the clear flag for specified tile.
	* \param[in] index		- The index into tileInstances vector.
	* \param[in] cleared	- The flag to set for clear.
	*/
	void SetTileCleared(int index, bool cleared);
	/*!
	* \brief To set the clear flag for specified tile.
	* \param[in] x			- The tile grid x position.
	* \param[in] y			- The tile grid y position.
	* \param[in] cleared	- The flag to set for clear.
	*/
	void SetTileCleared(int x, int y, bool cleared);
	/*!
	* \brief To set the clear flag for specified tile.
	* \param[in] gridPos	- The grid x-y position of the tile.
	* \param[in] cleared	- The flag to set for clear.
	*/
	void SetTileCleared(const GridPos& gridPos, bool cleared);

	/*!
	* \brief
	*	Calculate the grid position of the tile instance based off
	*	it's anchor/offset.
	* \param[in] tile	- The tile instance to calculate grid position of.
	* \return A vector for calculated tile's grid position.
	*/
	std::vector<GridPos> CalculateGridPos(const TileInstance& tile);
	/*!
	* \brief
	*	Calculate the grid position of the tile instance based off
	*	it's anchor/offset.
	* \param[in] tile	- The tile's archetype data.
	* \param[in] anchor	- The anchor that the tile archetype's position is based on/offset from.
	* \return A vector for calculated tile's grid position.
	*/
	std::vector<GridPos> CalculateGridPos(const TileData& tile, const GridPos& anchor);

	/*!
	* \brief Clamp the tile's anchor to within the map grid's dimensions.
	* \param[in] tile		- The tile data to get the tiles positions.
	* \param[in] anchor		- The anchor to clamp.
	* \return - The clamped anchor.
	*/
	GridPos ClampAnchor(const TileData& tile, const GridPos& anchor);

	/*!
	* \brief Calculate the tile instance's world position.
	* \param[in] tile		- The tile instance to calculate grid position of.
	* \param[in] multiplier	- The tile world size multiplier.
	* \param[in] origin		- The grid position of the starting tile (as starting tile is [0,0] in world space).
	* \return A vector for calculated tile's world position.
	*/
	std::vector<Vec2> CalculateWorldPos(const TileInstance& tile);

	/*!
	* \brief Convert position in grid coordinates to world coordinates.
	* \param[in] gridPos - The position in grid coordinate to convert.
	* \return The converted position in world coordinates.
	*/
	Vec2 ConvertGridToWorld(const GridPos& gridPos);
	/*!
	* \brief Convert position in world coordinates to grid coordinates.
	* \param[in] worldPos - The position in world coordinate to convert.
	* \return The converted position in grid coordinates.
	*/
	GridPos ConvertWorldToGrid(const Vec2& worldPos);

	/*!
	* \brief Get the tile instance index based on grid's position.
	* \param[in] x - x position in grid coordinates.
	* \param[in] y - y position in grid coordinates.
	* \return The tile instance index if found, else -1.
	*/
	int GetTileInstanceIndex(int x, int y);

	/*!
	* \brief Get the tile instance index based on grid's position.
	* \param[in] gridPos - x and y position in grid coordinates.
	* \return The tile instance index if found, else -1.
	*/
	int GetTileInstanceIndex(const GridPos& gridPos);

	/*!
	* \brief Get the tile instance index based on world position.
	* \param[in] worldPos - world position of the tile instance.
	* \return The tile instance index if found, else -1.
	*/
	int GetTileInstanceIndex(const Vec2& worldPos);

	/*!
	* \brief Get the tile instance index based on entity id of the created tile.
	* \param[in] ent - Entity ID of the tile instance.
	* \return The tile instance index if found, else -1.
	*/
	int GetTileInstanceIndex(EntityRegistry::Entity ent);

	/*!
	* \brief Get the generated tiles for user to place on map.
	* \return - A vector of 3 TileChoice.
	*/
	std::vector<TileChoice> GetTileOffers() const;

	/*!
	* \brief Helper function to get gimmick type enum as string.
	* \param[in] type		- The gimmick type.
	* \return - The string of the gimmick type.
	*/
	std::string GetGimmickTypeString(GimmickType type) const;
	/*!
	* \brief Helper function to get item tier enum as string.
	* \param[in] type		- The item tier type.
	* \return - The string of the item tier type.
	*/
	std::string GetItemTierSpritePath(ItemTier type) const;

	/*!
	* \brief Checks whether a tile is placeable onto map grid.
	* \param[in] data		- The tile data
	* \param[in] anchor		- The bottom left anchor of the tile
	* \return - False if tile is out of bounds or is being placed on another tile, true otherwise.
	*/
	PlaceTileResult IsTilePlaceAble(const TileData& data, const GridPos& anchor);

	/*!
	* \brief 
	*	Spawns all map tiles (except starting) into Game scene.
	*	Handles enemy spawning as well.
	*/
	void SpawnMapTiles();

	/*!
	* \brief Get the tile world scale multiplier.
	* \return Return the scale multiplier of tiles.
	*/
	Vec2 GetTileMultiplier() const { return tileMultiplier; }

	/*!
	* \brief 
	*	Subtracts count from enemies. If enemies reaches 0, set recently placed
	*	tile as cleared.
	* \param[in] count - Number of enemies killed.
	*/
	void SetEnemiesKilled(int count);

	/*!
	* \brief Check whether player is within a specific tile.
	* \param[in] index		- The tile instance index to check if player is in.
	* \param[in] playerPos	- The player world position.
	* \return True if player is in specified tile, false otherwise.
	*/
	bool IsPlayerInsideTile(int index, const Vec2& playerPos);

	/*!
	* \brief Set the value for first entry into specified tile type.
	* \param[in] index	- The tile instance index.
	* \param[in] val	- True for first tile entry, false for tile has been entered before. (default false).
	*/
	void SetTileFirstEntry(int index, bool val = false);

	/*!
	* \brief Clears all current tile instance.
	*/
	void ClearMapData();

	/*!
	* \brief 
	*	For logging, just prints out the contents of the
	*	current map grid and it's tiles.
	*/
	void LogTileGrid();

	/*!
	* \brief Getter for the map grid's width.
	* \return - The map grid's width as an integer.
	*/
	int GetMapGridWidth() const { return mapGridWidth; }
	/*!
	* \brief Getter for the map grid's height.
	* \return - The map grid's height as an integer.
	*/
	int GetMapGridHeight() const { return mapGridHeight; }

	/*!
	* \brief Getter for the number of tile instances.
	* \return - The number of tile instances as an integer.
	*/
	int GetTileInstanceSize();

	/*!
	* \brief Gets a pointer to the current wave information.
	* \return - nullptr if no wave data was loaded, else the pointer to the wave info.
	*/
	WaveInfo* GetCurrentWave() { if (waves.empty() || currentWave >= waves.size() || currentWave < 0 ) return nullptr; else return &waves[currentWave]; }
	/*!
	* \brief Get the cuurrent wave number. (will return with starting index 1).
	* \return - The current wave number, starting from 1.
	*/
	int GetCurrentWaveNumber() const { return currentWave + 1; }
	/*!
	* \brief Advance wave by given amount.
	* \param[in] amount		- How much to advance current wave by, defaulted to 1.
	* \return - True if after advancing, wave number is over the total nnumber of waves, else false.
	*/
	bool AdvanceWave(int amount = 1);
	/*!
	* \brief Reset wave back to default. Sets cleared flag to false and reset wave index.
	*/
	void ResetWave() { currentWave = 0; for (auto& w : waves) w.cleared = false; }

	/*!
	* \brief Getter for the starting tile's instance index.
	* \return - The starting tile's instance index.
	*/
	int GetStartTileIndex() const { return startTile; }
	/*!
	* \brief Getter for the end tile's instance index.
	* \return - The end tile's instance index.
	*/
	int GetEndTileIndex() const { return endTile; }
private:
	/*!
	* \brief Helper function to pre-spawn the special tiles (start, end, and treasure).
	* \param[in] tileType - What is the tile type, does nothing if tileType is COMBAT.
	* \param[in] tileData - The special tile's tile archetype data.
	* \return The index into tileInstances vector after placement. -1 if failed.
	*/
	int PlaceSpecialTile(TileType tileType, const TileData* tileData);

	/*!
	* \brief 
	*	Simple helper function to get the tiles that can be placed by user. Shuffles
	*	the order of tiles. Used for generating tile choices. If n is bigger than
	*	number of tile archetypes, cap n to number of tile archetypes.
	* \param[in] n - Number of tiles to retrieve.
	* \return A vector of indexes, of size n, of the tile archetype that is placeable.
	*/
	std::vector<int> GetPlaceableTiles(size_t n);

	/*!
	* \brief Resets tile choices array
	*/
	void ClearTileChoices();

	/*!
	* \brief Randomly generates an enemy type.
	* \param[in, out] rng - The random engine to use for randomising.
	* \return The generated enemy type.
	*/
	EnemyType GenerateEnemyType(std::mt19937& rng);

	/*!
	* \brief Randomly generates a gimmick type
	* \param[in, out] rng - The random engine to use for randomising.
	* \return The generated gimmick type.
	*/
	GimmickType GenerateGimmickType(std::mt19937& rng);

	/*!
	* \brief Randomly generates item tier based on gimmick type.
	* \param[in] gimmick - The gmmick of the tile.
	* \return The generated item tier.
	*/
	ItemTier GenerateItemTier(GimmickType gimmick, const TileData& tile, EnemyType enemy);

	/*!
	* \brief Randomly generates which prefab to use.
	* \param[in, out] rng	- The random engine to use for randomising.
	* \param[in] weight		- Weight of first choice (will change in future to accomdate more prefab options)
	* \return The generated prefab index
	*/
	int GeneratePrefab(std::mt19937& rng, float weight = 0.5f);
	/*!
	* \brief Simple helper function to check if a grid [x][y] is within bounds
	* \param[in] x - The grid's x value to check
	* \param[in] y - The grid's y value to check
	* \return True if it is within bounds, false if not
	*/
	bool InMapBounds(int x, int y) const;
	/*!
	* \brief Simple helper function to check if a grid [x][y] is within bounds
	* \param[in] gridPos - The grid's x and y coordinates to check.
	* \return True if it is within bounds, false if not
	*/
	bool InMapBounds(GridPos gridPos) const;
private:
	std::vector<TileData> allTiles;					// the tile archetypes
	std::vector<TileInstance> tileInstances;		// the tiles that has been placed
	std::vector<std::vector<int>> tileGrid;			// map of grid pos to tile index (in tileInstances). [height][width]
	std::array<TileChoice, 3> currentTileOffers;	// simple array that holds the generated tile choices

	std::vector<WaveInfo> waves;					// container of the waves for the final wave level

	const int unitTileSize{ 8 };					// The number of tileset tiles in a tile [Currently it's 8x8]
	const Vec2 unitTileScale{ 320.f, 320.f };		// The scale of each tileset tile
	const Vec2 tileMultiplier{ unitTileScale * static_cast<float>(unitTileSize) };		// multiplier to convert grid position into world position

	// ==== for ease of access ==== 
	int startTile{ -1 };							// index to get start tile in tileInstances vector
	int endTile{ -1 };								// index to get end tile in tileInstances vector
	int treasureTile{ -1 };							// index to get treasure tile in tileInstances vector

	int itemRarityCache{};

	int currentWave{};								// the index for the current wave

	std::string mapDataFilePath{};					// the string to the mapData json
	std::string waveDataFilePath{};
	const int mapGridWidth{ 7 };					// width of the map grid (number of cells)
	const int mapGridHeight{ 7 };					// height of the map grid (number of cells)
	bool initialized{ false };						// whether the map manager has already been initialized
};