/*!
@file       DoorManipulationScript.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Script that handles door behaviour (like opening and closing
			when player moves into a tile). Also updates the position
			for minimap camera.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "DoorManipulationScript.h"
#include "PlayerControllerScript.h"
#include "../CoreLib/MapManager.h"
#include "MinimapScript.h"
#include "../CoreLib/PersistentDataManager.h"
#include "EnemySpawnerScript.h"
#include "BlindManipulationScript.h"
#include "DialogueScript.h"

void DoorManipulationScript::OnStart(Registry& registry) {
	auto& mapManager{ *CEO::Instance().Get<MapManager>() };
	auto hierarchyComp{ GetComponent<HierarchyComponnent>(registry) };
	EntityRegistry::Entity root{ entity };
	while (hierarchyComp && hierarchyComp->parent != 0) {	// loop to get the parent of the door
		root = hierarchyComp->parent;
		hierarchyComp = registry.GetComponent<HierarchyComponnent>(root);
	}

	// get the tile instance index
	if(tileInstanceIndex == -1) tileInstanceIndex = mapManager.GetTileInstanceIndex(root);	// Get and save the tile instance index
	OpenDoor(registry);	// Open doors if room is cleared

	EntityRegistry::Entity child{ hierarchyComp ? hierarchyComp->firstChild : 0};
	while (child) {	// loop to find enemy spawner which is a child of tile.
		auto nameComp{ registry.GetComponent<NameComponent>(child) };
		if (nameComp && nameComp->name == "SpawnPoints") {
			enemySpawner = child;
			break;
		}
		child = registry.GetComponent<HierarchyComponnent>(child)->nextSibling;
	}

	// loop to find post process entity
	for (auto ent : registry.GetEntitiesWithComponent<PostProcessComponent>()) {
		auto blindScript{ registry.GetComponent<BlindManipulationScript>(ent) };
		if (blindScript) {
			postProcessEnt = ent;
			break;
		}
	}

	for (auto ent : registry.GetEntitiesWithComponent<MinimapScript>()) {
		if (registry.GetComponent<NameComponent>(ent)->name == "Minimap") {
			minimapEnt = ent;
			break;
		}
	}

	for (auto ent : registry.GetEntitiesWithComponent<DialogueScript>()) {
		dialogueEnt = ent;
		break;
	}

	for (auto ent : registry.GetEntitiesWithComponent<PlayerControllerScript>()) {
		if (registry.GetComponent<NameComponent>(ent)->name == "Player") {
			playerEnt = ent;
			break;
		}
	}
};


void DoorManipulationScript::OnUpdate(Registry& registry, float , bool) {
	auto check{ CEO::Get<PersistentDataManager>()->Get<bool>("RoomCleared") };
	if (check && *check) { if (!doorOpened) OpenDoor(registry); };	// open door if room is cleared and door isn't already opened
};

void DoorManipulationScript::OnFixedUpdate(Registry&, float, bool) {

};

void DoorManipulationScript::OnTriggerExit(const Collider& other) {
	if (tileInstanceIndex == -1) return;
	auto layer{ other.GetComponent<LayerComponent>() };
	if (layer == nullptr || CEO::Get<LayerManager>()->GetLayerName(layer->layer) != "Player") return;

	Registry& registry{ *CEO::Instance().Get<Registry>() };

	auto& mapManager{ *CEO::Instance().Get<MapManager>() };

	auto tile{ mapManager.GetTileInstance(tileInstanceIndex) };
	auto playerTransform{ registry.GetComponent<TransformComponent>(playerEnt) };
	if (mapManager.IsPlayerInsideTile(tileInstanceIndex, playerTransform->translate)) {
		if (!tile->cleared) CloseDoor(registry);	// close all doors if tile hasn't been cleared
		if (minimapEnt) {	// update minimap if exists
			auto& minimapScript{ *registry.GetComponent<MinimapScript>(minimapEnt) };
			minimapScript.UpdateMinimapCam(tileInstanceIndex);	// update minimap camera's position
			auto isCombat{ CEO::Get<PersistentDataManager>()->Get<bool>("InCombat") };
			if (isCombat && *isCombat) {	// toggle minimap off if flag is enabled
				auto minimapToggle{ CEO::Get<PersistentDataManager>()->Get<bool>("ToggleMinimapOffDuringCombat") };
				if (minimapToggle && *minimapToggle) registry.GetComponent<ActiveComponent>(minimapEnt)->isActiveSelf = false;
			}
		}
	}
}

void DoorManipulationScript::OnTriggerEnter(const Collider& other) {
	if (tileInstanceIndex == -1) return;
	auto layer{ other.GetComponent<LayerComponent>() };
	if (layer == nullptr || CEO::Get<LayerManager>()->GetLayerName(layer->layer) != "Player") return;
	EnableDialogue();
	CEO::Get<MapManager>()->SetTileFirstEntry(tileInstanceIndex);
}

void DoorManipulationScript::OpenDoor(Registry& registry) {
	if (tileInstanceIndex == -1) return;
	auto collisionComp{ GetComponent<CollisionComponent>(registry) };
	if (!collisionComp) return;
	auto& mapManager{ *CEO::Get<MapManager>() };

	auto tile{ mapManager.GetTileInstance(tileInstanceIndex) };
	if (tile == nullptr) return;

	MapManager::GridPos doorCell{ tile->anchor.first + localGridPosX, tile->anchor.second + localGridPosY };

	auto neighbours{ mapManager.GetNearbyTiles(doorCell) };	// get neightbouring tiles of the tile this door belongs to

	if (neighbours[direction] != -1 ) {		// if the direction has a tile
		collisionComp->isTrigger = true;	// turn off door collider
		if (!doorSpriteEnt) {				// to turn off sprite of the door
			auto doorParentEnt{ GetComponent<HierarchyComponnent>(registry)->parent };
			auto doorChildEnt{ registry.GetComponent<HierarchyComponnent>(doorParentEnt)->firstChild };
			while (doorChildEnt) {
				if (doorChildEnt != entity) {
					auto activeComp{ registry.GetComponent<ActiveComponent>(doorChildEnt) };
					if (activeComp) activeComp->isActiveSelf = false;
					break;
				}
				doorChildEnt = registry.GetComponent<HierarchyComponnent>(doorChildEnt)->nextSibling;
			}
		}
		else registry.GetComponent<ActiveComponent>(doorSpriteEnt)->isActiveSelf = false;

		doorOpened = true;	// set door opened to true
	}

	if (!doorOpened || !postProcessEnt || tile->gimmickType != GimmickType::BLIND) return;
	auto blindScript{ registry.GetComponent<BlindManipulationScript>(postProcessEnt) };
	if (blindScript) {
		blindScript->EnableBlind(false);	// disable blind if tile's gimmick was blind
	}
	
}
void DoorManipulationScript::CloseDoor(Registry& registry) {
	if (tileInstanceIndex == -1 ) return;

	auto check{ CEO::Get<PersistentDataManager>()->Get<bool>("DoorsClosed") };
	if (check && *check) return;

	for (auto ent : registry.GetEntitiesWithComponent<DoorManipulationScript>()) {	// loop through all doors to close
		auto collisionComp{ registry.GetComponent<CollisionComponent>(ent) };
		if (!collisionComp) continue;
		auto doorScript{ registry.GetComponent<DoorManipulationScript>(ent) };
		collisionComp->isTrigger = false;	// re-enable door's collision
		doorScript->doorOpened = false;	// reset door open flag to false

		auto doorParentEnt{ registry.GetComponent<HierarchyComponnent>(ent)->parent };
		auto doorChildEnt{ registry.GetComponent<HierarchyComponnent>(doorParentEnt)->firstChild };
		while (doorChildEnt) {
			if (doorChildEnt != ent) {
				auto activeComp{ registry.GetComponent<ActiveComponent>(doorChildEnt) };
				if(activeComp) activeComp->isActiveSelf = true;
				break;
			}
			doorChildEnt = registry.GetComponent<HierarchyComponnent>(doorChildEnt)->nextSibling;
		}
		registry.GetComponent<DoorManipulationScript>(ent)->doorOpened = false;	// reset door open flag to false
	}

	doorOpened = false;
	
	CEO::Get<PersistentDataManager>()->Set<bool>("DoorsClosed", true);

	if (!enemySpawner) return;
	
	auto& enemySpawnerScript{ *registry.GetComponent<EnemySpawnerScript>(enemySpawner) };
	enemySpawnerScript.EnableEnemies(registry);		// "Spawn enemies" by making them active

	// check if tile has blind
	auto tile{ CEO::Get<MapManager>()->GetTileInstance(tileInstanceIndex) };
	if (!tile || !postProcessEnt || tile->gimmickType != GimmickType::BLIND) return;

	auto blindScript{ registry.GetComponent<BlindManipulationScript>(postProcessEnt) };
	if (blindScript) {	// enable blind mode
		blindScript->EnableBlind(true);
	}
}

void DoorManipulationScript::EnableDialogue() {
	if (dialogueEnt == 0 || tileInstanceIndex == -1) return;	// if there is no dialogue entity in scene, skip
	auto tile{ CEO::Get<MapManager>()->GetTileInstance(tileInstanceIndex) };

	if (tile->tileData->firstEntry) {
		auto dialogueScript{ CEO::Get<Registry>()->GetComponent<DialogueScript>(dialogueEnt) };
		switch (tile->tileData->type) {
			case TileType::TREASURE:
				dialogueScript->Trigger("TREASURE");
				break;
			case TileType::END:
				dialogueScript->Trigger("EXIT");
				break;
		}
	}
}