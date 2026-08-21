/*!
@file       TreasureChestScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan) (90%)
@author     Tan Jun Jie (kaedenjiawei.tan) (10%)
@date		09/03/2026 (DD/MM/YYYY)
@brief      A script that handles the treasure chest in Treasure tile.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*____________________________________________________________________________*/

#include "TreasureChestScript.h"
#include "PlayerControllerScript.h"
#include "../CoreLib/StatsManager.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/PersistentDataManager.h"

void TreasureChestScript::OnStart(Registry& registry) {
	if (CEO::Get<StatsManager>()->GetTreasureCollected()) {
		auto spriteComp{ GetComponent<SpriteRendererComponent>(registry) };
		if (spriteComp) spriteComp->visible = false;
	}
};


void TreasureChestScript::OnUpdate(Registry &, float , bool )
{
};

void TreasureChestScript::OnFixedUpdate(Registry&, float, bool)
{
};
void TreasureChestScript::OnTriggerEnter(const Collider& other) {
	StatsManager* stats = CEO::Get<StatsManager>();
	stats->SetRarity(3);
	if (!stats->GetTreasureCollected() && other.GetComponent<PlayerControllerScript>()) {
		stats->SetRarity(3);
		stats->SetTreasureCollected();
		auto spriteComp{ GetComponent<SpriteRendererComponent>(*CEO::Get<Registry>()) };
		if (spriteComp) spriteComp->visible = false;
		CEO::Get<SceneManager>()->QueueSceneAction("Treasure", SceneManager::PUSH);
	}
}