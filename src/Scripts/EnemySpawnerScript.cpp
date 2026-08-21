/*!
@file       EnemySpawnerScript.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Handles spawning of enemies for normal game mode and wave mode.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "EnemySpawnerScript.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/PersistentDataManager.h"
#include "../CoreLib/Pathfind.h"
#include "../CoreLib/Camera.h"
#include "DialogueScript.h"
#include "../CoreLib/SceneManager.h"
#include "FadeScript.h"

void EnemySpawnerScript::OnStart(Registry& registry) {
	auto vpSize{ CEO::Get<CameraManager>()->GetViewportSize() };

	enemySpawnRadius = std::max(vpSize.x, vpSize.y) * 0.65f;	// set enemy spawn radius based on percentage of viewport size

	// get the player entity
	for (auto ent : registry.GetEntitiesWithComponent<NameComponent>()) {
		if (registry.GetComponent<NameComponent>(ent)->name == "Player") {
			playerEnt = ent;
			break;
		}
	}

	for (auto ent : registry.GetEntitiesWithComponent<DialogueScript>()) {
		dialogueEnt = ent;
		break;
	}

	// ======= Get all the spawn points this script is responsible for =============
	auto hierarchyComp{ GetComponent<HierarchyComponnent>(registry) };
	EntityRegistry::Entity ent{ hierarchyComp->firstChild };
	while (ent) {											// get all the spawn points on the tile
		spawnPoints.push_back(ent);
		ent = registry.GetComponent<HierarchyComponnent>(ent)->nextSibling;
	}
	// set up the distribution for randomising
	randSpawnPoints = std::uniform_int_distribution<>(0, static_cast<int>(spawnPoints.size() - 1));

	if (!isWave) SpawnEnemies(registry);
	else {
		CEO::Get<PersistentDataManager>()->Set("IsWaveMode", true);
		CEO::Get<PersistentDataManager>()->Set<int>("WaveEnemiesSpawned", 0);
		CEO::Get<Pathfind>()->SetMapMinMax(Vec2(-3000.f, -3000.f), Vec2(3000.f, 3000.f));
		for (auto e : registry.GetEntitiesWithComponent<TextRendererComponent>()) {
			auto nameComp{ registry.GetComponent<NameComponent>(e) };
			if (nameComp && nameComp->name == "WaveText") {
				waveTxtEnt = e;
				break;
			}
		}
	}
};

void EnemySpawnerScript::OnUpdate(Registry& registry, float dt, bool ) {
	if (!isWave) return;	// Non-wave mode enemies are spawned in SpawnEnemy(). Below handles spawning wave-mode enemies

	auto& mapManager{ *CEO::Get<MapManager>() };
	auto& pm{ *CEO::Get<PersistentDataManager>() };
	auto ds{ registry.GetComponent<DialogueScript>(dialogueEnt) };

	if (finishedWave) {
		auto am = CEO::Instance().GetManager<AchievementManager>();
		if (delayTimer == 0)
		{
			am->UpdateTracker("WIN");
			pm.Set("IsWin", true);
			ds->Trigger("WIN");
		}
		delayTimer += dt;
		if (delayTimer < delayDuration) return;	// delay before showing win screen
		mapManager.ResetWave();
		if (am->GetLockedAchievements().size() == 0 && am->popUpQueue.size() == 0 && !am->shown) {
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav", 0.0f, 0.0f);
			registry.GetComponent<FadeScript>(fadeToCutscene.GetEntityID())->FadeToBlack();
			am->shown = true;
			am->showEndCutscene = false;
		}
		else
		{
			registry.GetComponent<FadeScript>(fadeBG.GetEntityID())->FadeToBlack();
		}
		return;
	}

	auto waveInfo{ mapManager.GetCurrentWave() };
	if (waveInfo == nullptr) return;

	auto enemiesSpawnedTracker{ pm.Get<int>("WaveEnemiesSpawned") };			// enemies spawned in this wave
	int enemiesSpawned{ enemiesSpawnedTracker ? *enemiesSpawnedTracker : 0 };

	// wave is cleared if no enemies are alived in this wave and total enemies spawned == wave's total enemies
	if (enemiesSpawned == 0 && totalWaveEnemiesSpawned >= waveInfo->totalEnemies ) waveInfo->cleared = true;

	if (waveInfo->cleared) {					// if wave is cleared, advance to wave
		pm.Set<int>("WaveEnemiesSpawned", 0);	// reset enemies spawned back to 0
		totalWaveEnemiesSpawned = 0;			// reset total enemies spawned back to 0
		if (mapManager.AdvanceWave()) {			// AdvanceWave() returns true if all wave is cleared
			finishedWave = true;				// set all wave finished flag to true
		}
		waveDelayActive = true;					// set wave delay flag to true
		CEO::Get<PersistentDataManager>()->Set<bool>("InCombat", false);

		if (waveTxtEnt) {			// update wave number text if it exists
			if (!finishedWave) {
				auto textComp{ registry.GetComponent<TextRendererComponent>(waveTxtEnt) };
				textComp->text = "WAVE " + std::to_string(mapManager.GetCurrentWaveNumber());
			}
		}
		return;
	}
	// Wave delay to give the player some breathing room
	if (waveDelayActive) {
		if (waveDelayTimer == 0.f)
		{
			if (ds)
			{
				auto nextWave{ mapManager.GetCurrentWaveNumber() };
				switch (nextWave)
				{
				case 1: ds->Trigger("WAVE1"); break;
				case 2: ds->Trigger("WAVE2"); break;
				case 3: ds->Trigger("WAVE3"); break;
				}
			}
		}
		waveDelayTimer += dt;
		if (waveDelayTimer >= waveInfo->waveDelay) {
			waveDelayTimer = 0.f;
			waveDelayActive = false;
			CEO::Get<PersistentDataManager>()->Set<bool>("InCombat", true);
		}
		else return;
	}
	// don't spawn enemies if max is spawned or total enemies has been spawned
	if (totalWaveEnemiesSpawned >= waveInfo->totalEnemies || enemiesSpawned >= waveInfo->maxSpawnAmount) return;

	spawnDelayTimer += dt;
	if (spawnDelayTimer <= waveInfo->spawnDelay) return;

	std::string chosenPrefab{ waveInfo->enemiesPrefab[waveInfo->enemyDistribution(rng)] };	// get the randomised enemy prefab to spawn

	auto enemyEnt{ CEO::Get<ResourceManager>()->InstantiatePrefab(registry, chosenPrefab) };				// spawn the wave enemy
	registry.GetComponent<TransformComponent>(enemyEnt)->translate = RandomiseEnemyWorldPosition(registry);	// randoomise their position in Final Wave scene

	// Increment trackers
	++totalWaveEnemiesSpawned;
	++enemiesSpawned;

	pm.Set("WaveEnemiesSpawned", enemiesSpawned);
	spawnDelayTimer = 0.f;
};

void EnemySpawnerScript::OnFixedUpdate(Registry&, float, bool)
{
};

void EnemySpawnerScript::SpawnEnemies(Registry& registry) {
	auto& mapManager{ *CEO::Get<MapManager>() };
	auto hierarchyComp{ GetComponent<HierarchyComponnent>(registry) };
	EntityRegistry::Entity root{ entity };
	while (hierarchyComp && hierarchyComp->parent != 0) {	// loop to get the parent of the door
		root = hierarchyComp->parent;
		hierarchyComp = registry.GetComponent<HierarchyComponnent>(root);
	}
	int tileIndex = mapManager.GetTileInstanceIndex(root);	// get the tile instance index that this script is attached to
	auto tile{ mapManager.GetTileInstance(tileIndex) };		// get the tile
	if (tile == nullptr || tile->cleared) return;			// skip if tile is invalid or has been cleared.

	int totalEnemies{};										// tracker for the total number of enemies spawned

	auto worldPositions{ mapManager.CalculateWorldPos(*tile) };		// calculate the world position of the tile
	auto tileMultiplier{ mapManager.GetTileMultiplier() };

	int enemiesCount{static_cast<int>(ceil(tile->tileData->tilePosition.size() * 2.0f))  };	// set the initial starting count of enemies to spawn

	enemiesCount += static_cast<int>((static_cast<float>((mapManager.GetTileInstanceSize()) * 1.2f) - 3));

	if (tile->gimmickType == GimmickType::SWARM) enemiesCount = static_cast<int>(enemiesCount * 1.5f);				// double if tile has Swarm gimmick
	int smallCount{}, mediumCount{}, largeCount{};
	enemiesCount = std::min(enemiesCount, 25);

	switch (tile->enemyType) {				// spawn the approriate enemy based on the tile's enemy type
		case EnemyType::SMALL: smallCount = static_cast<int>(floor(enemiesCount / 1.2f)); break;
		case EnemyType::MEDIUM: mediumCount = static_cast<int>(floor(enemiesCount / 1.3f)); break;
		case EnemyType::LARGE: largeCount = static_cast<int>(ceil(enemiesCount / 2.3f)); break;
		case EnemyType::SMALL_AND_MEDIUM:
			smallCount = static_cast<int>(floor(enemiesCount * 0.5f));
			mediumCount = static_cast<int>(floor((enemiesCount * 0.5f) / 1.3f));
			break;
		case EnemyType::SMALL_AND_LARGE:
			smallCount = static_cast<int>(floor(enemiesCount * 0.5f));
			largeCount = static_cast<int>(floor((enemiesCount * 0.5f) / 1.6f));
			break;
		case EnemyType::MEDIUM_AND_LARGE:
			mediumCount = static_cast<int>(floor((enemiesCount * 0.5f) / 1.5f));
			largeCount = static_cast<int>(floor((enemiesCount * 0.5f) / 1.8f));
			break;
		case EnemyType::EVERYTHING:
			smallCount = static_cast<int>(floor(enemiesCount * 0.4f));
			mediumCount = static_cast<int>(floor((enemiesCount * 0.35f) / 1.3f));
			largeCount = static_cast<int>(floor((enemiesCount * 0.25f) / 1.6f));
			break;
	}

	enemies.reserve(enemiesCount);

	float enemySpeedMultiplier{ tile->gimmickType == GimmickType::FAST_ENEMIES ? 1.5f : 1.f };
	for (int j{}; j < smallCount; ++j) {	// spawn the small type enemies
		int rand = std::rand() % 2;
		std::string enemyType[] = {"Eye", "Larva"};
		auto enemyEnt{ CEO::Get<ResourceManager>()->InstantiatePrefab(registry, enemyType[rand])};
		registry.GetComponent<EnemyComponent>(enemyEnt)->velocity *= enemySpeedMultiplier;
		registry.GetComponent<ActiveComponent>(enemyEnt)->isActiveSelf = false;
		enemies.push_back(enemyEnt);
	}

	for (int j{}; j < mediumCount; ++j) {		// spawn the medium type enemies
		int rand = std::rand() % 4;
		std::string enemyType[] = { "Girl", "GirlAlt" , "Guy" , "GuyAlt"};
		auto enemyEnt{ CEO::Get<ResourceManager>()->InstantiatePrefab(registry, enemyType[rand])};
		registry.GetComponent<EnemyComponent>(enemyEnt)->velocity *= enemySpeedMultiplier;
		registry.GetComponent<ActiveComponent>(enemyEnt)->isActiveSelf = false;
		enemies.push_back(enemyEnt);
	}

	for (int j{}; j < largeCount; ++j) {		// spawn the large type enemies
		int rand = std::rand() % 2;
		std::string enemyType[] = { "Oni", "OniAlt" };
		auto enemyEnt{ CEO::Get<ResourceManager>()->InstantiatePrefab(registry, enemyType[rand])};
		registry.GetComponent<EnemyComponent>(enemyEnt)->velocity *= enemySpeedMultiplier;
		registry.GetComponent<ActiveComponent>(enemyEnt)->isActiveSelf = false;
		enemies.push_back(enemyEnt);
	}

	totalEnemies = smallCount + mediumCount + largeCount;				// update the enemies tracker
	CEO::Get<PersistentDataManager>()->Set("EnemiesCount", totalEnemies);

	if (totalEnemies == 0) mapManager.SetTileCleared(tileIndex, true);	// if no enemies was spawned, tile is automatically cleared
	else {	// set the min and max values for the collision map (for the enemy pathfinding)
		Vec2 min{ worldPositions.front() }, max{ worldPositions.front() };
		for (int j{ 1 }; j < worldPositions.size(); ++j) {
			min.x = std::min(min.x, worldPositions[j].x);
			min.y = std::min(min.y, worldPositions[j].y);
			max.x = std::max(max.x, worldPositions[j].x);
			max.y = std::max(max.y, worldPositions[j].y);
		}

		min = { min.x - tileMultiplier.x, min.y - tileMultiplier.y };
		max = { max.x + tileMultiplier.x, max.y + tileMultiplier.y };
		CEO::Get<Pathfind>()->SetMapMinMax(min, max);
	}
}

Vec2 EnemySpawnerScript::RandomiseEnemyWorldPosition(Registry& registry) {
	if (spawnPoints.empty()) return Vec2(0.f, 0.f);		// if no spawn points are avaible, default to [0, 0]
	int index{ }; 
	float distSq{}, spawnRadiusSquare{ enemySpawnRadius * enemySpawnRadius };
	Vec2 spawnPos{}, playerPos{ registry.GetComponent<TransformComponent>(playerEnt)->translate };;
	do {	// find a spawn point that is further away than radius squared from the player
		index = randSpawnPoints(rng);
		auto transformComp{ registry.GetComponent<TransformComponent>(spawnPoints[index]) };
		spawnPos = Vec2(transformComp->transform.m[6], transformComp->transform.m[7]);
		float dx{ spawnPos.x - playerPos.x }, dy{ spawnPos.y - playerPos.y };
		distSq = dx * dx + dy * dy;
	} while (distSq < spawnRadiusSquare);

	return spawnPos;	// return the randomly generated spawn position
}

void EnemySpawnerScript::EnableEnemies(Registry& registry) {
	for (auto enemy : enemies) {	// loop through all enemies spawned by this script and enable them
		registry.GetComponent<ActiveComponent>(enemy)->isActiveSelf = true;
		registry.GetComponent<TransformComponent>(enemy)->translate = RandomiseEnemyWorldPosition(registry);
	}
	CEO::Get<PersistentDataManager>()->Set<bool>("InCombat", true);	// set in combat flag to true

	auto& mapManager{ *CEO::Get<MapManager>() };
	auto hierarchyComp{ GetComponent<HierarchyComponnent>(registry) };
	EntityRegistry::Entity root{ entity };
	while (hierarchyComp && hierarchyComp->parent != 0) {	// loop to get the parent of the door
		root = hierarchyComp->parent;
		hierarchyComp = registry.GetComponent<HierarchyComponnent>(root);
	}
	int tileIndex = mapManager.GetTileInstanceIndex(root);
	auto tile{ mapManager.GetTileInstance(tileIndex) };
	auto ds{ registry.GetComponent<DialogueScript>(dialogueEnt) };

	switch (tile->gimmickType) {
		case GimmickType::FAST_ENEMIES: ds->Trigger("PUNISH_F"); return;
		case GimmickType::SWARM: ds->Trigger("PUNISH_S"); return;
		case GimmickType::BLIND: ds->Trigger("PUNISH_B"); return;
		case GimmickType::NONE: break;
	}

	// if reach here, gimmick type == NONE
	switch (tile->enemyType) {				// spawn the approriate enemy based on the tile's enemy type
		case EnemyType::SMALL: ds->Trigger("COMBAT_S"); break;
		case EnemyType::MEDIUM: ds->Trigger("COMBAT_M"); break;
		case EnemyType::LARGE: ds->Trigger("COMBAT_L"); break;
		case EnemyType::SMALL_AND_MEDIUM: ds->Trigger("COMBAT_SM"); break;
		case EnemyType::SMALL_AND_LARGE: ds->Trigger("COMBAT_SL"); break;
		case EnemyType::MEDIUM_AND_LARGE: ds->Trigger("COMBAT_ML"); break;
		case EnemyType::EVERYTHING: ds->Trigger("COMBAT_E"); break;
	}
}