/**___________________________________________________________________________/
@file       MapManager.cpp
@author     t.junjie@digipen.edu
@date       03/02/2026	(DD/MM/YYYY)
@brief		Manager that handles manages and spawns a grid-based map based on
			user input. Handles spawning of enemies and applying gimmick for
			each tile.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "MapManager.h"
#include "Pathfind.h"
#include "PersistentDataManager.h"
#include "SceneManager.h"
#include "CEO.h"
#include "StatsManager.h"
#include "AchievementManager.h"

namespace {
#ifdef PLATFORM_WINDOWS
	const std::string basePath("Assets\\");
	void ReplaceSlashes(std::string& s) {
		std::replace(s.begin(), s.end(), '/', '\\');
	}
#endif
#ifdef PLATFORM_ANDROID
	const std::string basePath("");
#endif
}

void MapManager::Init(const std::string& mapData, const std::string& waveData) {
	if (initialized) return;				// skip if already initialized
	mapDataFilePath = basePath + mapData;	// set up the file path for reading tile archetype
	waveDataFilePath = basePath + waveData;
	tileGrid.resize(mapGridHeight, std::vector<int>(mapGridWidth, -1));	// pre allocate tileGrid based on map grid & height

	LoadWaveData();
	initialized = true;						// set initialized to true
}

bool MapManager::LoadTileData() {
	if (!allTiles.empty()) return false;	// if allTiles container isn't empty, means tile data already loaded, early exit from func
	// ----- For reading tile data JSON
	rapidjson::Document doc;
	std::stringstream ifs{ CEO::Instance().GetManager<FileManager>()->ReadFile(mapDataFilePath) };
	if (!ifs) {
		LOGE("Error in reading %s MapData", mapDataFilePath.c_str());
		return false;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	
	doc.Parse(buffer.str());
	if (doc.HasParseError()) {
		LOGE("MapData json is corrupted or invalid!");
		return false;
	}
	else if (!doc.HasMember("Tiles") || !doc["Tiles"].IsArray()) {
		LOGE("MapData json does not have Tiles array!");
		return false;
	}

	const rapidjson::Value& tiles{ doc["Tiles"] };
	for (rapidjson::SizeType i{}; i < tiles.Size(); ++i) {
		const rapidjson::Value& t{ tiles[i] };
		TileData tile;

		tile.tileName = t["tileName"].GetString();
		const auto& prefabs{ t["prefabName"] };
		for (rapidjson::SizeType j{}; j < prefabs.Size(); ++j) {
			tile.prefabsName.emplace_back(prefabs[j].GetString());
		}

		std::string path{ t["spritePath"].GetString() };
#ifdef PLATFORM_WINDOWS
		ReplaceSlashes(path);
#endif

		tile.sprite = std::make_pair(&CEO::Instance().GetManager<ResourceManager>()->GetTexture(path), path);
		
		std::string type{ t["tileType"].GetString() };
		if (type == "Start") tile.type = TileType::START;
		else if (type == "End") tile.type = TileType::END;
		else if (type == "Treasure") tile.type = TileType::TREASURE;
		else tile.type = TileType::COMBAT;
		tile.unlocked = t["unlocked"].GetBool();

		const rapidjson::Value& positions{ t["tilePos"] };
		for (rapidjson::SizeType j{}; j < positions.Size(); ++j) {
			tile.tilePosition.emplace_back(positions[j][0].GetInt(), positions[j][1].GetInt());
		}
		allTiles.push_back(std::move(tile));
	}
	return true;
}

bool MapManager::LoadWaveData() {
	if (!waves.empty()) return false;
	// ----- For reading tile data JSON
	rapidjson::Document doc;
	std::stringstream ifs{ CEO::Instance().GetManager<FileManager>()->ReadFile(waveDataFilePath) };
	if (!ifs) {
		LOGE("Error in reading %s WaveData", waveDataFilePath.c_str());
		return false;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();

	doc.Parse(buffer.str());
	if (doc.HasParseError()) {
		LOGE("WaveData json is corrupted or invalid!");
		return false;
	}
	else if (!doc.HasMember("Waves") || !doc["Waves"].IsArray()) {
		LOGE("WaveData json does not have Tiles array!");
		return false;
	}

	const rapidjson::Value& waveInfo{ doc["Waves"] };
	waves.reserve(waveInfo.Size());
	for (rapidjson::SizeType i{}; i < waveInfo.Size(); ++i) {
		const rapidjson::Value& w{ waveInfo[i] };
		WaveInfo wi;

		const rapidjson::Value& enemies{ w["enemies"] };
		wi.enemiesPrefab.reserve(enemies.Size());
		wi.enemiesWeight.reserve(enemies.Size());
		for (rapidjson::SizeType j{}; j < enemies.Size(); ++j) {
			wi.enemiesPrefab.push_back(enemies[j]["type"].GetString());
			wi.enemiesWeight.push_back(enemies[j]["weight"].GetFloat());
		}

		wi.totalEnemies = w["totalEnemies"].GetInt();
		wi.maxSpawnAmount = w["maxSpawnAmount"].GetInt();
		wi.spawnDelay = w["spawnDelay"].GetFloat();
		wi.waveDelay = w["waveDelay"].GetFloat();

		wi.enemyDistribution = std::discrete_distribution<>(wi.enemiesWeight.begin(), wi.enemiesWeight.end());

		waves.push_back(wi);
	}
	return true;

}

void MapManager::InitStartingMapData() {
	if (!tileInstances.empty()) return;			// starting tiles already generated

	if (allTiles.empty() && !LoadTileData()) {	// check if tile data load was successful
		LOGE("Failed to Load tile data!");
		return;
	}

	const TileData* start{ nullptr };
	const TileData* end{ nullptr };
	const TileData* treasure{ nullptr };

	for (const auto& t : allTiles) {			// search for start, end, and treasure tile data
		switch (t.type) {
			case TileType::START:		start = &t; break;
			case TileType::END:			end = &t; break;
			case TileType::TREASURE:	treasure = &t; break;
		}
	}

	if (!start || !end || !treasure) {
		LOGE("Missing special tiles in tile data!");
		return;
	}

	// ====== Generate Start Tile ======
	startTile = PlaceSpecialTile(TileType::START, start);		// pre place starting tile
	if (startTile == -1) {
		LOGE("Error in generating start tile!");
		return;
	}

	// ====== Generate End Tile ======
	endTile = PlaceSpecialTile(TileType::END, end);				// pre-place ending tile
	if (endTile == -1) {
		LOGE("Error in generating end tile!");
		return;
	}

	// ====== Generate Treasure Tile ======
	treasureTile = PlaceSpecialTile(TileType::TREASURE, treasure);	// pre-place treasure tile
	if (treasureTile == -1) {
		LOGE("Error in generating treasure tile!");
		return;
	}

#ifdef _DEBUG
	LOGD("Starting Tiles initialized!");
#endif
}

void MapManager::GenerateTileOffers() {
	// if start, end, and treasure tile isn't already placed, initialize starting map data
	if (tileInstances.empty()) InitStartingMapData();	

	std::vector<int> candidates{ GetPlaceableTiles(allTiles.size()) };	// get candidate tiles

	std::fill(currentTileOffers.begin(), currentTileOffers.end(), TileChoice{});

	size_t n{ candidates.size() > currentTileOffers.size() ? currentTileOffers.size() : candidates.size() };

	std::mt19937 rng{ std::random_device{}() };		// for randomising

	for (size_t i{}; i < n; ++i) {
		currentTileOffers[i].tileData = &allTiles[candidates[i]];			// generate a new tile archetype
		currentTileOffers[i].prefabIndex = GeneratePrefab(rng);		// generates the prefab to use
		currentTileOffers[i].enemyType = GenerateEnemyType(rng);			// generate a new enemy type
		currentTileOffers[i].gimmickType = GenerateGimmickType(rng);		// generate a new gimmick type
		currentTileOffers[i].itemTier = GenerateItemTier(					// generate a new item tier
			currentTileOffers[i].gimmickType,
			*currentTileOffers[i].tileData,
			currentTileOffers[i].enemyType
		);
	}
#ifdef _DEBUG
	LOGD("Generated Tile offers!");
#endif
}

bool MapManager::CheckPathValidity(const TileData* tileData, const GridPos& anchor) {
	if(startTile == -1 || tileData == nullptr) return false;	// if there is no start tile or invalid input, path is invali
	// visited[y][x] marks whether the cell has been reached by bfs
	std::vector<std::vector<bool>> visited(mapGridHeight, std::vector<bool>(mapGridWidth, false));
	std::queue<GridPos> queue;

	// push all grid cells occupied by start tile as the BFS source
	for (const auto& cell : CalculateGridPos(tileInstances[startTile])) {
		visited[cell.second][cell.first] = true;
		queue.push(cell);
	}

	// Add existing tile to the walkable grid
	std::vector<std::vector<bool>> walkable(mapGridHeight, std::vector<bool>(mapGridWidth, false));
	for (int y{}; y < mapGridHeight; ++y) {
		for (int x{}; x < mapGridWidth; ++x) {
			if (tileGrid[y][x] != -1) walkable[y][x] = true;
		}
	}

	// temporarily add the new tile into the walkable grid
	for (const auto& cell : CalculateGridPos(*tileData, anchor)) {
		if (InMapBounds(cell.first, cell.second)) 
			walkable[cell.second][cell.first] = true;
	}

	static const std::array<GridPos, 4> directions{
		GridPos{1, 0}, GridPos{-1, 0}, GridPos{0, 1}, GridPos{0, -1}
	};

	while (!queue.empty()) {		// BFS search
		GridPos current{ queue.front() };
		queue.pop();

		for (const auto& dir : directions) {
			int nx{ current.first + dir.first }, ny{ current.second + dir.second };

			if (!InMapBounds(nx, ny)) continue;		// out of bounds check
			if (!walkable[ny][nx]) continue;		// check if its walkable
			if (visited[ny][nx]) continue;			// check if it already has been visited

			visited[ny][nx] = true;
			queue.emplace(nx, ny);
		}
	}

	// A tile is valid if any of it's cell touches a visited cell in any
	// of the 4 cardinal direction
	for (const auto& cell : CalculateGridPos(*tileData, anchor)) {
		if (visited[cell.second][cell.first]) return true;
	}

	return false;	// else tile path is not valid
}

bool MapManager::CheckPathValidity(int startTileIndex, int endTileIndex) {
	if (startTileIndex == -1 || startTileIndex >= tileInstances.size() || endTileIndex == -1 || endTileIndex >= tileInstances.size()) return false;

	std::vector<std::vector<bool>> visited(mapGridHeight, std::vector<bool>(mapGridWidth, false));
	std::queue<GridPos> queue;

	// push all grid cells occupied by start tile as the BFS source
	for (int y{}; y < mapGridHeight; ++y) {
		for (int x{}; x < mapGridWidth; ++x) {
			if (tileGrid[y][x] == startTileIndex) {
				visited[y][x] = true;
				queue.emplace(x, y);
			}
		}
	}

	static const std::array<GridPos, 4> directions{
		GridPos{1, 0}, GridPos{-1, 0}, GridPos{0, 1}, GridPos{0, -1}
	};	

	while (!queue.empty()) {		// BFS search
		GridPos current{ queue.front() };
		queue.pop();

		for (const auto& dir : directions) {
			int nx{ current.first + dir.first }, ny{ current.second + dir.second };

			if (!InMapBounds(nx, ny)) continue;		// out of bounds check)
			if (visited[ny][nx]) continue;			// check if it already has been visited

			if (tileGrid[ny][nx] == -1) continue;	// only walk on placed tiles

			visited[ny][nx] = true;
			queue.emplace(nx, ny);
		}
	}

	// check if we reached any of the end tile cells
	for (int y{}; y < mapGridHeight; ++y) {
		for (int x{}; x < mapGridWidth; ++x) {
			if (tileGrid[y][x] == endTileIndex && visited[y][x]) {
				return true;
			}
		}
	}
	return false;	// else tile path is not valid
}

TileInstance* MapManager::GetTile(int x, int y) {
	// bounds checking
	if (x < 0 || x >= mapGridWidth || y < 0 || y >= mapGridHeight) return nullptr;

	int index{ tileGrid[y][x] };	// get the tile instance index in the specified grid position

	if (index == -1) return nullptr;	// if index == -1, means no tile has been placed so empty spot
	else return &tileInstances[index];
}

TileInstance* MapManager::GetTile(const Vec2& worldPos) {
	int tileIndex{ GetTileInstanceIndex(worldPos) };
	if (tileIndex == -1) return nullptr;
	return &tileInstances[tileIndex];
}

TileInstance* MapManager::GetTile(TileType tileType) {
	switch (tileType) {
		case TileType::START:		return &tileInstances[startTile];
		case TileType::END:			return &tileInstances[endTile];
		case TileType::TREASURE:	return &tileInstances[treasureTile];
		default: return nullptr;
	}
}

TileInstance* MapManager::GetTileInstance(int index) {
	if (index == -1 || index >= tileInstances.size()) return nullptr;
	else return &tileInstances[index];
}

std::vector<int> MapManager::GetNearbyTiles(int x, int y) {
	std::vector<int> tiles(4, -1);

	static const std::array<GridPos, 4> directions{	// Direction: 0 is NORTH | 1 == SOUTH | 2 == EAST | WEST == 3
		GridPos{0, 1}, GridPos{0, -1}, GridPos{1, 0}, GridPos{-1, 0}
	};

	for (int i{}; i < 4; ++i) {
		int newX{ x + directions[i].first }, newY{ y + directions[i].second };
		if (!InMapBounds(newX, newY)) continue;	// out of bounds check
		
		tiles[i] = tileGrid[newY][newX];
	}

	return tiles;
}

std::vector<int> MapManager::GetNearbyTiles(const GridPos& gridPos) { return GetNearbyTiles(gridPos.first, gridPos.second); }

size_t MapManager::GetTileInstanceSize() const { return tileInstances.size(); }

PlaceTileResponse MapManager::PlaceTile(const TileChoice& tile, const GridPos& anchor) {
	const TileData* data{ tile.tileData };
	auto placeResult{ IsTilePlaceAble(*data, anchor) };
	if (placeResult != PlaceTileResult::SUCCESS) return { placeResult, -1 };
	if (!CheckPathValidity(tile.tileData, anchor)) return { PlaceTileResult::INVALID_CONNECTION, -1 };

	int index{ static_cast<int>(tileInstances.size()) };
	TileInstance& instance{ tileInstances.emplace_back() };
	instance.tileData = data;
	instance.anchor = anchor;
	instance.prefabIndex = tile.prefabIndex;
	instance.enemyType = tile.enemyType;
	instance.gimmickType = tile.gimmickType;
	instance.itemTier = tile.itemTier;
	itemRarityCache = static_cast<int>(tile.itemTier);
	for (const auto& cell : data->tilePosition) {
		int gx{ cell.first + anchor.first }, gy{ cell.second + anchor.second };
		tileGrid[gy][gx] = index;
	}
	return { PlaceTileResult::SUCCESS, index };
}

PlaceTileResponse MapManager::PlaceTile(const TileChoice& tile, int x, int y) { return PlaceTile(tile, std::make_pair(x, y)); }

void MapManager::RemoveTile(int index) {
	if (index < 0 || index >= tileInstances.size()) return;
	auto& tile{ tileInstances[index] };
	for (const auto& cell : tile.tileData->tilePosition) {
		int gx{ cell.first + tile.anchor.first }, gy{ cell.second + tile.anchor.second };
		tileGrid[gy][gx] = -1;
	}
	tileInstances.erase(tileInstances.begin() + index);
}

void MapManager::SetTileCleared(int index, bool cleared) {
	if (index < 0 || index >= tileInstances.size()) return;
	tileInstances[index].cleared = cleared;
	if (index == tileInstances.size() - 1 && cleared) {
		auto registry{ CEO::Get<Registry>() };
		for (auto ent : registry->GetEntitiesWithComponent<ButtonComponent>()) {
			if (registry->GetComponent<NameComponent>(ent)->name == "BackToMap") {	// set back to map button to active
				registry->GetComponent<SpriteRendererComponent>(ent)->color = { 1.f, 1.f, 1.f, 1.f };
				//registry->GetComponent<ActiveComponent>(ent)->isActiveSelf = true;
				break;
			}
		}
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\DoorUnlock.wav").Play();
		CEO::Get<PersistentDataManager>()->Set<int>("EnemiesCount", 0);	// update enemies count tracker to 0
	}
}
void MapManager::SetTileCleared(int x, int y, bool cleared) {
	if (!InMapBounds(x, y)) return;
	tileInstances[tileGrid[y][x]].cleared = cleared;
	if (tileGrid[y][x] == tileInstances.size() - 1 && cleared) {	// for now, temporarily show the back to map button in Game.Scene	
		auto registry{ CEO::Get<Registry>() };
		for (auto ent : registry->GetEntitiesWithComponent<ButtonComponent>()) {
			if (registry->GetComponent<NameComponent>(ent)->name == "BackToMap") {
				registry->GetComponent<SpriteRendererComponent>(ent)->color = { 1.f, 1.f, 1.f, 1.f };
				break;
			}
		}
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\DoorUnlock.wav").Play();
		CEO::Get<PersistentDataManager>()->Set<int>("EnemiesCount", 0);	// update enemies count tracker to 0
	}
}

void MapManager::SetTileCleared(const GridPos& gridPos, bool cleared) { SetTileCleared(gridPos.first, gridPos.second, cleared); }

std::vector<TileChoice> MapManager::GetTileOffers() const {
	std::vector<TileChoice> tiles(currentTileOffers.size());
	for (size_t i{}; i < currentTileOffers.size(); ++i) {
		tiles[i] = currentTileOffers[i];
	}
	return tiles;
}

std::string MapManager::GetEnemyTypeString(EnemyType type) const {
	switch (type) {
		case EnemyType::SMALL: return "SMALL";
		case EnemyType::MEDIUM: return "MEDIUM";
		case EnemyType::LARGE: return "LARGE";
		case EnemyType::SMALL_AND_LARGE: return "SMALL AND LARGE";
		case EnemyType::SMALL_AND_MEDIUM: return "SMALL AND MED";
		case EnemyType::MEDIUM_AND_LARGE: return "MED AND LARGE";
		case EnemyType::EVERYTHING: return "EVERYTHING";
		default: return "NONE";
	}
}

std::string MapManager::GetGimmickTypeString(GimmickType type) const {
	switch (type) {
		case GimmickType::NONE: return "NONE";
		case GimmickType::SWARM: return "SWARM";
		case GimmickType::FAST_ENEMIES: return "FAST ENEMIES";
		case GimmickType::BLIND: return "BLIND";
		default: return "";
	}
}

std::string MapManager::GetItemTierSpritePath(ItemTier type) const {
	std::string path{ "ui\\game\\map_tier_" };
	switch (type) {
		case ItemTier::TIER1: path += "1"; break;
		case ItemTier::TIER2: path += "2"; break;
		case ItemTier::TIER3: path += "3"; break;
		case ItemTier::TIER4: path += "4"; break;
	}
	path += ".png";
	return path;
}

PlaceTileResult MapManager::IsTilePlaceAble(const TileData& data, const GridPos& anchor) {
	for (const GridPos& pos : CalculateGridPos(data, anchor)) {
		if (!InMapBounds(pos.first, pos.second))	// out of bounds check
			return PlaceTileResult::OUTOFBOUNDS;

		if(tileGrid[pos.second][pos.first] != -1)	// overlap check
			return PlaceTileResult::OVERLAP;
	}

	return PlaceTileResult::SUCCESS;
}

int MapManager::PlaceSpecialTile(TileType tileType, const TileData* tileData) {
	if (tileType == TileType::COMBAT) return -1;	// end if tile is not of special type
	std::mt19937 rng{ std::random_device{}() };		// for randomising

	int index{ static_cast<int>(tileInstances.size()) };

	TileInstance& tile{ tileInstances.emplace_back() };
	switch (tileType) {
		case TileType::START: {
			std::vector<int> startingTilePositions{ 0, 1, mapGridHeight - 2, mapGridHeight - 1 };
			std::uniform_int_distribution<> startingY{ 0, static_cast<int>(startingTilePositions.size()) - 1 };

			tile.anchor = { 0, startingTilePositions[startingY(rng)] }; 
			break;
		}
		case TileType::END: {
			tile.anchor.first = mapGridWidth - 1;
			if (startTile == -1) tile.anchor.second = 0;
			else {
				auto gridPos{ CalculateGridPos(tileInstances[startTile]) };
				tile.anchor.second = 6 - gridPos[0].second;
			}
			break;
		}
		case TileType::TREASURE: {
			tile.anchor.first = static_cast<int>(mapGridWidth / 2.f);
			if (startTile == -1) tile.anchor.second = 0;
			else {
				int offset{ mapGridHeight - 2 };
				auto gridPos{ CalculateGridPos(tileInstances[startTile]) };
				if (gridPos[0].second > tile.anchor.first) tile.anchor.second = gridPos[0].second - offset;
				else tile.anchor.second = gridPos[0].second + offset;
			}
			tile.itemTier = ItemTier::TIER4;	// treasure tile will have tier 4 items
			break;
		}
	}
	tile.tileData = tileData;						// set tile archetype 
	tile.enemyType = EnemyType::NONE;				// set enemy type for special tiles to NONE
	tile.gimmickType = GimmickType::NONE;			// set no gimmick type for special tiiles
	tile.prefabIndex = 0;							// get which prefab to use for special tiles
	tile.cleared = true;

	for (const auto& local : tileData->tilePosition) {
		int gx{ local.first + tile.anchor.first }, gy{ local.second + tile.anchor.second };
		tileGrid[gy][gx] = index;	// update tile grid map with the placed special tile
	}

	return index;
}

std::vector<MapManager::GridPos> MapManager::CalculateGridPos(const TileInstance& tile) { return CalculateGridPos(*tile.tileData, tile.anchor); }

std::vector<MapManager::GridPos> MapManager::CalculateGridPos(const TileData& tile, const GridPos& anchor) {
	std::vector<GridPos> gridPositions(tile.tilePosition);
	for (int i{}; i < gridPositions.size(); ++i) {
		gridPositions[i].first += anchor.first;
		gridPositions[i].second += anchor.second;
	}
	return gridPositions;
}

MapManager::GridPos MapManager::ClampAnchor(const TileData& tile, const GridPos& anchor) {
	GridPos min{INT_MAX, INT_MAX }, max{INT_MIN, INT_MIN };
	for (const auto& pos : tile.tilePosition) {
		min.first = std::min(min.first, pos.first);
		min.second = std::min(min.second, pos.second);
		max.first = std::max(max.first, pos.first);
		max.second = std::max(max.second, pos.second);
	}

	GridPos clamped{
		std::clamp(anchor.first, -min.first, mapGridWidth - 1 - max.first),
		std::clamp(anchor.second, -min.second, mapGridHeight - 1 - max.second)
	};

	return clamped;
}

std::vector<Vec2> MapManager::CalculateWorldPos(const TileInstance& tile) {
	auto origin { (startTile != -1) ? CalculateGridPos(*GetTileInstance(startTile)).front() : GridPos(0, 0) };

	auto positions{ CalculateGridPos(tile) };
	std::vector<Vec2> worldPos;
	for (auto& pos : positions) {
		worldPos.emplace_back((pos.first - origin.first) * tileMultiplier.x, 
			(pos.second - origin.second) * tileMultiplier.y);
	}

	return worldPos;
}

Vec2 MapManager::ConvertGridToWorld(const GridPos& gridPos) {
	auto origin{ (startTile != -1) ? CalculateGridPos(*GetTileInstance(startTile)).front() : GridPos(0, 0) };
	
	return Vec2(
		(gridPos.first - origin.first) * tileMultiplier.x,
		(gridPos.second - origin.second) * tileMultiplier.y
	);
}

MapManager::GridPos MapManager::ConvertWorldToGrid(const Vec2& worldPos) {
	auto origin{ (startTile != -1) ? CalculateGridPos(*GetTileInstance(startTile)).front() : GridPos(0, 0) };
	return GridPos{
		static_cast<int>(std::floor(worldPos.x / tileMultiplier.x)) + origin.first,
		static_cast<int>(std::floor(worldPos.y / tileMultiplier.y)) + origin.second
	};
}

int MapManager::GetTileInstanceIndex(int x, int y) {
	if (!InMapBounds(x, y)) return -1;
	return tileGrid[y][x];
}

int MapManager::GetTileInstanceIndex(const GridPos& gridPos) { return GetTileInstanceIndex(gridPos.first, gridPos.second); }

int MapManager::GetTileInstanceIndex(const Vec2& worldPos) { return GetTileInstanceIndex(ConvertWorldToGrid(worldPos)); }

int MapManager::GetTileInstanceIndex(EntityRegistry::Entity ent) {
	auto it{ std::find_if(tileInstances.begin(), tileInstances.end(), [ent](const TileInstance& tile) { return tile.tileId == ent; }) };
	if (it == tileInstances.end()) return -1;
	else return static_cast<int>(it - tileInstances.begin());
}

void MapManager::SpawnMapTiles() {
	// The starting tile prefab is spawned by default so no need to spawn.
	Registry& registry{ *CEO::Get<Registry>() };
	
	{
		auto playerPos{ CEO::Get<PersistentDataManager>()->Get<Vec2>("PlayerPosition") };
		int i{};
		for (auto ent : registry.GetEntitiesWithComponent<NameComponent>()) {
			if (i >= 3) break;
			auto name{ registry.GetComponent<NameComponent>(ent)->name };
			if (name == "Player") {
				if (playerPos) registry.GetComponent<TransformComponent>(ent)->translate = *playerPos;	// Move player to where player was last at
				++i;
			}
			if (name == "Main_Camera") {
				if (playerPos) registry.GetComponent<TransformComponent>(ent)->translate = *playerPos;	// move camera to where player was last at
				++i;
			}
			if (name == "Minimap_Camera") {
				++i;
				auto pos{ CEO::Get<PersistentDataManager>()->GetOrCreate<Vec2>("MinimapCamPos") };
				registry.GetComponent<TransformComponent>(ent)->translate = pos;						// update minimap camera position to last position

			}
		}
	}

	for (int i{}; i < tileInstances.size(); ++i) {
		const auto& tile{ tileInstances[i] };
		if (tile.tileData->tileName == "Starting_Level") continue;	// skip starting level as it already exist in Game.scene
		tileInstances[i].tileId = CEO::Get<ResourceManager>()->InstantiatePrefab(registry, tile.tileData->prefabsName[tile.prefabIndex]);	// instantiate the tile prefab
		auto worldPositions{ CalculateWorldPos(tile) };	// calculate the world position of the tile
		auto pos{ worldPositions.front()};
		if (tile.tileData->tileName == "lshape_dl") { pos.y -= tileMultiplier.x; }	// handle tile outlier case

		registry.GetComponent<TransformComponent>(tileInstances[i].tileId)->translate = pos;			// set the tile prefab's position
	}
}

void MapManager::SetEnemiesKilled(int count) {
	auto& pdm{ *CEO::Get<PersistentDataManager>() };
	auto waveMode{ pdm.Get<bool>("IsWaveMode") };
	if (waveMode && *waveMode) {
		auto enemiesSpawned{ pdm.Get<int>("WaveEnemiesSpawned") };
		int spawned{ enemiesSpawned ? *enemiesSpawned : 0 };
		if (enemiesSpawned) spawned = std::max(spawned - count, 0);
		pdm.Set("WaveEnemiesSpawned", spawned);
	} 
	else {
		auto enemies{ pdm.Get<int>("EnemiesCount") };
		int enemiesCount{ enemies == nullptr ? 0 : *enemies };

		if (count <= 0 || enemies == 0 || tileInstances.size() <= 3) return;

		auto& persistentData{ *CEO::Get<PersistentDataManager>() };

		enemiesCount = std::max(0, enemiesCount - count);
		persistentData.Set("EnemiesCount", enemiesCount);

		if (enemiesCount == 0) {	// if enemies have all been killed, set recent room / tile as cleared
			SetTileCleared(static_cast<int>(tileInstances.size()) - 1, true);
			pdm.Set("RoomCleared", true);
			pdm.Set("InCombat", false);
			pdm.Set("DoorsClosed", false);

			auto minimapToggle{ pdm.Get<bool>("ToggleMinimapOffDuringCombat") };
			if (minimapToggle && *minimapToggle) {
				auto minimapEnt{ pdm.Get<EntityRegistry::Entity>("MinimapEnt") };
				if (minimapEnt) {
					CEO::Get<Registry>()->GetComponent<ActiveComponent>(*minimapEnt)->isActiveSelf = true;
				}
			}
			CEO::Get<SceneManager>()->QueueSceneAction("Item", SceneManager::PUSH);
			CEO::Get<StatsManager>()->SetRarity(itemRarityCache);
			itemRarityCache = 0;
		}
	}
}

bool MapManager::IsPlayerInsideTile(int index, const Vec2& pos) {
	CEO::Get<PersistentDataManager>()->Set("PlayerPosition", pos);	// update player position tracker
	if (index < 0 || index >= tileInstances.size()) return false;	// check if tile instance index is valid

	auto worldPositions{ CalculateWorldPos(tileInstances[index]) };	// get world position of specified tile

	// check if player is within tile's boundaries
	Vec2 halfTile{ unitTileScale * static_cast<float>(unitTileSize >> 1) };
	for (const auto& worldPos : worldPositions) {
		Vec2 min{ worldPos.x - halfTile.x , worldPos.y - halfTile.y };
		Vec2 max{ worldPos.x + halfTile.x, worldPos.y + halfTile.y };
		
		if (pos.x >= min.x && pos.x <= max.x && pos.y >= min.y && pos.y <= max.y)
			return true;
	}
	return false;
}

void MapManager::ClearMapData() { 
	for (auto& row : tileGrid) {
		std::fill(row.begin(), row.end(), -1);	// change all index in tile grid to -1 (which denotes empty)
	}
	tileInstances.clear();

	for (auto& td : allTiles) td.firstEntry = true;	// reset back to true
}

void MapManager::SetTileFirstEntry(int index, bool val) {
	if (index < 0 || index >= tileInstances.size()) return;
	const TileData* tileData{ tileInstances[index].tileData };
	if (!tileData) return;

	int tileDataIndex{ static_cast<int>(tileData - allTiles.data()) };
	if (tileDataIndex < 0 || tileDataIndex >= allTiles.size()) return;

	allTiles[tileDataIndex].firstEntry = val;
}

void MapManager::LogTileGrid() {
#ifdef _DEBUG
	for (int j{ mapGridHeight - 1 }; j >= 0; --j) {
		std::string temp;
		for (int i{}; i < mapGridWidth; ++i) {
			TileInstance* tile{ GetTile(i, j) };
			if (!tile) { temp += "0"; }
			else {
				switch (tile->tileData->type) {
					case TileType::START: temp += "S"; break;
					case TileType::END: temp += "E"; break;
					case TileType::TREASURE: temp += "T"; break;
					case TileType::COMBAT: temp += "N"; break;
				}
			}
			temp += " | ";
		}
		LOGD("%s", temp.c_str());
	}
#endif
}

std::vector<int> MapManager::GetPlaceableTiles(size_t n) {
	std::mt19937 rng{ std::random_device{}() };		// for randomising 

	std::vector<int> tiles;
	auto achievementManager{ *CEO::Get<AchievementManager>() };
	auto unlockedTile{ achievementManager.GetUnlockedAchievements(RewardType::TILE) };

	for (int i{}; i < allTiles.size(); ++i) {
		if (allTiles[i].type == TileType::COMBAT) {
			if (!allTiles[i].unlocked) {
				auto it{ std::find_if(unlockedTile.begin(), unlockedTile.end(),[currentTile = allTiles[i]](const Achievement& unlocked) {
					for (int i{}; i < unlocked.rewardItem.size(); ++i) if (currentTile.tileName == unlocked.rewardItem[i]) return true;
					return false; }) };
				if (it == unlockedTile.end()) continue;
			}
			tiles.push_back(i);
		}
	}

	std::shuffle(tiles.begin(), tiles.end(), rng);	// shuffle the normal tiles

	if (n > tiles.size()) n = tiles.size();

	return std::vector<int>(tiles.begin(), tiles.begin() + n);
}

EnemyType MapManager::GenerateEnemyType(std::mt19937& rng) {
	std::uniform_int_distribution<> randEnemyType{ 0, static_cast<int>(EnemyType::NONE) - 1};
	EnemyType enemy;
	auto unlockedEnemy{ CEO::Get<AchievementManager>()->GetUnlockedAchievements(RewardType::ENEMY) };
	std::vector<Achievement>::iterator it;
	do {	// check if generated enemy type is unlocked, else generate again
		enemy = static_cast<EnemyType>(randEnemyType(rng));
		if (enemy == EnemyType::SMALL) break;
		it = std::find_if(unlockedEnemy.begin(), unlockedEnemy.end(), [currEnemy = GetEnemyTypeString(enemy)](const Achievement& unlocked) {
			for (int i{}; i < unlocked.rewardItem.size(); ++i) if (currEnemy.find(unlocked.rewardItem[i]) != std::string::npos) return true;
				return false;
			}
		);
	} while (it == unlockedEnemy.end());
	
	return enemy;
}

GimmickType MapManager::GenerateGimmickType(std::mt19937& rng) {
	std::discrete_distribution<> randGimmickType({ (int)GimmickWeights::NONE, (int)GimmickWeights::FAST_ENEMIES, (int)GimmickWeights::SWARM, (int)GimmickWeights::BLIND });
	GimmickType gimmick;
	auto unlockedGimmick{ CEO::Get<AchievementManager>()->GetUnlockedAchievements(RewardType::GIMMICK) };
	std::vector<Achievement>::iterator it;

	do {	// check if generated gimmick is unlocked, else generate again
		gimmick = static_cast<GimmickType>(randGimmickType(rng));
		if (gimmick == GimmickType::NONE) break;
		it = std::find_if(unlockedGimmick.begin(), unlockedGimmick.end(), [currGimmick = GetGimmickTypeString(gimmick)](const Achievement& unlocked) {
			for (int i{}; i < unlocked.rewardItem.size(); ++i) if (currGimmick.find(unlocked.rewardItem[i]) != std::string::npos) return true;
			return false;
			}
		);
	} while (it == unlockedGimmick.end());
	return gimmick;
}

ItemTier MapManager::GenerateItemTier(GimmickType gimmick, const TileData& tile, EnemyType enemy) {
	// Gimmick difficulty
	int gimmickWeight = 0;
	switch (gimmick) {
	case GimmickType::SWARM:         gimmickWeight = 3; break;
	case GimmickType::FAST_ENEMIES:  gimmickWeight = 3; break;
	case GimmickType::BLIND:         gimmickWeight = 4; break;
	default: /* NONE */              gimmickWeight = 0; break;
	}

	int sizeWeight = 0;
	size_t cellCount = tile.tilePosition.size();
	if (cellCount == 3)          sizeWeight = 1;
	else if (cellCount == 4)     sizeWeight = 2;
	else                         sizeWeight = 0;

	int enemyWeight = 1;
	switch (enemy) {
	case EnemyType::SMALL:              enemyWeight = 1; break;
	case EnemyType::MEDIUM:             enemyWeight = 2; break;
	case EnemyType::LARGE:              enemyWeight = 3; break;
	case EnemyType::SMALL_AND_MEDIUM:   enemyWeight = 3; break;
	case EnemyType::SMALL_AND_LARGE:    enemyWeight = 4; break;
	case EnemyType::MEDIUM_AND_LARGE:   enemyWeight = 5; break;
	case EnemyType::EVERYTHING:         enemyWeight = 6; break;
	default: break;
	}


	// Total difficulty (max ~4+2+6=12)
	int difficultyScore = gimmickWeight + sizeWeight + enemyWeight;


	// Map to base tier (1�E3)
	int baseTier = 1;
	if (difficultyScore >= 9)      baseTier = 3;   // hard ? tier?3
	else if (difficultyScore >= 5) baseTier = 2;   // medium ? tier?2
	else                           baseTier = 1;   // easy ? tier?1

	// Add luck 
	int luck = static_cast<int>(CEO::Get<StatsManager>()->GetLuck());
	int finalTier = baseTier + luck;
	finalTier = std::clamp(finalTier, static_cast<int>(ItemTier::TIER1),
		static_cast<int>(ItemTier::TIER4));

	return static_cast<ItemTier>(finalTier);
}

int MapManager::GeneratePrefab(std::mt19937& rng, float weight) {
	std::uniform_real_distribution<float> dist(0.f, 1.f);
	float r{ dist(rng) };
	if (r < weight) return 0;
	else return 1;
}

void MapManager::ClearTileChoices() { std::fill(currentTileOffers.begin(), currentTileOffers.end(), TileChoice()); }

bool MapManager::InMapBounds(int x, int y) const { return x >= 0 && x < mapGridWidth && y >= 0 && y < mapGridHeight; }

bool MapManager::InMapBounds(GridPos gridPos) const { return InMapBounds(gridPos.first, gridPos.second); }

int MapManager::GetTileInstanceSize() { return static_cast<int>(tileInstances.size()); }

bool MapManager::AdvanceWave(int amount) {
	currentWave += amount; 
	if (currentWave >= waves.size()) return true;
	else return false;
}