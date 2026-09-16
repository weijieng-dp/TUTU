/*!
@file       EnemySpawnerScript.h
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Handles spawning of enemies for normal game mode and wave mode.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"
#include <random>

class EnemySpawnerScript: public ScriptInstance {
public:
	void BindFrom() { 
        GetComponent<EnemySpawnerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<EnemySpawnerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<EnemySpawnerScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    /*!
    * \brief Spawns enemies (inactive) for the tile this script is attached to.
    * \param[in, out] registry - The entity registry.
    */
    void SpawnEnemies(Registry& registry);

    /*!
    * \brief Enable enemies that were spawned by this script.
    * \param[in, out] registry - The entity registry.
    */
    void EnableEnemies(Registry& registry);

    void RefreshWaveAvailability();

    float enemySpawnRadius{ 350.f };        // how far away the spawn points need to be from the player (world space)

    bool isWave{ false };                   // whether this script will handle spawning wave enemies

    GameObject fadeBG;

    GameObject fadeToCutscene;

    REFLECTABLE_PROPERTIES;
private:
    /*!
   * \brief Helper function to randomise the spawn point to spawn enemies on.
   * \param[in, out] registry   - The entity registry.
   * \param[in] playerEnt       - The entity id of the player (for calculating distance).
   * \param[in, out] rn         - The random engine to randomise with.
   */
    Vec2 RandomiseEnemyWorldPosition(Registry& registry);

    std::mt19937 rng{ std::random_device{}() };		        // for randomising
    std::uniform_int_distribution<int> randSpawnPoints{};   // distribution for randomising which spawn points to use
    std::vector<EntityRegistry::Entity> enemies;            // store all the enemies spawned (non-wave)
    std::vector<EntityRegistry::Entity> spawnPoints;	    // store all the spawn points
    EntityRegistry::Entity playerEnt{};                     // the player ent (cached for faster access)
    EntityRegistry::Entity dialogueEnt{};                   // the dialogue text (cached for faster access)
    EntityRegistry::Entity waveTxtEnt{};                    // Wave text (cached for faster access) [can be removed]

    std::vector<std::string> availableEnemiesPrefab;        // used for waves, track the enemies prefab that are available to be spawned
    std::discrete_distribution<> availableEnemyDist;        // available enemy weight distrubtion, used to randomise enemy wave spawning

    float spawnDelayTimer{}, waveDelayTimer{};          // Timers for delay, 1 for delaying spawning and the other to delay the wave
    int totalWaveEnemiesSpawned{};                      // Counter for the total number of enemies for this wave
    bool waveDelayActive{ true };                       // flag to track whether wave delay is on
    float delayTimer{ 0.f };                            // delay timer for showing WIN scene after all wave is cleared
    float delayDuration{ 3.0f };                        // delay duration for showing WIN scene after all wave is cleared
	bool finishedWave{ false };                         // whether all waves has been completed
};
REFL_AUTO(
    type(EnemySpawnerScript),
    field(enemySpawnRadius),
    field(isWave),
    field(fadeBG),
	field(fadeToCutscene)
)