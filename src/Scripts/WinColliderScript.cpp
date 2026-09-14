/**___________________________________________________________________________/
@file       WinColliderScript.cpp
@author     d.lorenzoyongoyong@digipen.edu	80%
@co-author	Tan Jun Jie (t.junjie)			20%
@date       01/02/2026   (DD/MM/YYYY)
@brief      Script attached to the win collider trigger zone. Detects when
            the player enters the win trigger, starts spiral animation, and
            transitions to Final Wave scene upon completion of animation.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "WinColliderScript.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/PersistentDataManager.h"
#include "FadeScript.h"

void WinColliderScript::OnStart(Registry& r)
{
	auto fade = r.GetEntitiesWithComponent<FadeScript>();
	for (auto ent : fade) {
		if (r.GetComponent<NameComponent>(ent)->name == "Fade To Wave") {
			fadeBg = ent;
			break;
		}
	}
};


void WinColliderScript::OnUpdate(Registry& registry, float dt, bool) {
	PlayerSpiral(registry, dt);
};

void WinColliderScript::OnFixedUpdate(Registry&, float, bool)
{
};

void WinColliderScript::OnTriggerEnter(const Collider& other)
{
	if (isSpiralActive) return;						// if already spiralling, skip
	Registry* registry = CEO::Get<Registry>();
	LayerComponent* otherLayer = registry->GetComponent<LayerComponent>(other.entity);
	if (CEO::Get<LayerManager>()->GetLayerName(otherLayer->layer) == "PlayerEnvBox")
	{
		StartSpiral(*registry, other.entity);		// start spiral animation if player is the one t
	}
}
void WinColliderScript::StartSpiral(Registry& registry, EntityRegistry::Entity player) {
	playerEnt = player;
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Wee.wav").Play();
	auto playerTransformComp{ registry.GetComponent<TransformComponent>(playerEnt) };
	if (!playerTransformComp) return;

	isSpiralActive = true;		// set spiral active flag to true
	spiralTimer = 0.f;			// reset spiral timer

	auto winColliderTransformComp{ GetComponent<TransformComponent>(registry) };
	if (winColliderTransformComp) {	// set up initial values for spiral animation
		spiralCenter = { winColliderTransformComp->transform.m[6], winColliderTransformComp->transform.m[7] };
		Vec2 offset{ playerTransformComp->translate - spiralCenter };
		initialRadius = offset.Length();
		initialAngle = std::atan2f(offset.y, offset.x);
	}
	initialScale = playerTransformComp->scale;
}

void WinColliderScript::PlayerSpiral(Registry& registry, float dt) {
	if (!isSpiralActive) return;		// end if spiral is not active

	if(spiralTimer >= spiralDuration) {					// if spiral reached duration
		auto& pm{ *CEO::Get<PersistentDataManager>() };
		pm.Set("InCombat", false);						// reset in combat flag to false
		pm.Set("RoomCleared", false);					// reset room cleared flag to false
		pm.Set("IsBlindModeOn", false);					// reset blindmodeon flag to false
		pm.Set<int>("WaveEnemiesSpawned", 0);			// reset wave enemies spawned tracker to 0
		fadeBg.GetComponent<FadeScript>()->sceneToTransition = "EndingCutscene";
		fadeBg.GetComponent<FadeScript>()->FadeToBlack();	// fade to Final Wave scene
		spiralTimer = 0.f;								// reset spiral timer
		isSpiralActive = false;							// reset spiral active flag to false
		registry.GetComponent<ActiveComponent>(playerEnt)->isActiveSelf = false;	// deactivate player after spiral ends
	}

	spiralTimer += dt;		// increase spiral timer by dt

	auto playerTransformComp{ registry.GetComponent<TransformComponent>(playerEnt) };
	if (!playerTransformComp) return;

	float t{ spiralTimer / spiralDuration };
	float radius{ (1.f - t) * initialRadius };
	float angle{ initialAngle + t * PI * 6.f };

	// Update player position
	playerTransformComp->translate.x = spiralCenter.x + radius * std::cosf(angle);
	playerTransformComp->translate.y = spiralCenter.y + radius * std::sinf(angle);

	float scale{ (1.f - t) };
	playerTransformComp->scale.x = initialScale.x * scale;
	playerTransformComp->scale.y = initialScale.y * scale;

	playerTransformComp->rotation += angle * 0.2f;
}