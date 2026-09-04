/**___________________________________________________________________________/
@file          EnemyCollisionScript.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Enemy scripting file. Currently contains 
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/

#include "EnemyCollisionScript.h"
#include "EnemyStatusEffectScript.h"
#include "FlashingVFX.h"
#include "HealthScript.h"
#include "../CoreLib/AudioManager.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/Camera.h"
#include "WeightedRNG.h"

void EnemyCollisionScript::OnStart(Registry& r) {
	EnemyComponent* ec = GetComponent<EnemyComponent>(r);
	if (!ec) return;

	HealthScript* health = GetComponent<HealthScript>(r);
	if (health)
	{
		health->onHitCallback = std::bind(&EnemyCollisionScript::OnDamageTaken, this, std::placeholders::_1);
		health->onDeathCallback = std::bind(&EnemyCollisionScript::OnDeath, this);
	}
}

void EnemyCollisionScript::OnUpdate(Registry& r, float dt, bool firstframe) {
	// empty
	(void)dt; (void)firstframe;
	EnemyComponent* ec = GetComponent<EnemyComponent>(r);
	if (!ec) return;

	if (ec->hp <= 0) {	// reach 0, destroy entity
		// not working, wei jie pls help
		//auto nsc{ GetComponent<NativeScriptingComponent>(r) };

		//for (auto& [name, script] : nsc->scripts) {
		//	if (script.OnDeleteScript) script.OnDeleteScript(script);
		//}
		//nsc->scripts.clear();
		//r.DestroyEntity(entity);
		//SpriteRendererComponent* renderer{GetComponent<SpriteRendererComponent>(r)};
		//UpdateStackComponent* stack{ GetComponent<UpdateStackComponent>(r) };
		//if (renderer && renderer->visible) renderer->visible = false;
		//if (stack && stack->stack != UpdateStackComponent::StackLevel::PAUSE) stack->stack = UpdateStackComponent::StackLevel::PAUSE;
	}
}
void EnemyCollisionScript::OnFixedUpdate(Registry & r, float dt, bool firstframe) {
	(void)dt; (void)firstframe; (void)r; // Unused params

}

void EnemyCollisionScript::OnTriggerStay(const Collider& other)
{
	Registry* registry = CEO::Get<Registry>();

	LayerComponent* otherLayer = registry->GetComponent<LayerComponent>(other.entity);
	if ((static_cast<unsigned long long>(1) << otherLayer->layer) & CEO::Get<LayerManager>()->GetLayerMaskByName("Player"))
	{
		HierarchyComponnent* hc = GetComponent<HierarchyComponnent>(*CEO::Get<Registry>());
		EnemyComponent* ec{ nullptr };
		if (hc->parent)
			ec = registry->GetComponent<EnemyComponent>(hc->parent);
		else
			ec = registry->GetComponent<EnemyComponent>(entity);

		if (ec)
		{

			HierarchyComponnent* phc = registry->GetComponent<HierarchyComponnent>(other.entity);

			HealthScript* playerHealth{ nullptr };
			if (phc->parent)
			{
				playerHealth = registry->GetComponent<HealthScript>(phc->parent);
				HierarchyComponnent* phc2 = registry->GetComponent<HierarchyComponnent>(phc->parent);
				if (phc2->parent)
				{

					playerHealth = registry->GetComponent<HealthScript>(phc2->parent);
				}

			}
			else
			{
				playerHealth = registry->GetComponent<HealthScript>(other.entity);
			}
			

			if (playerHealth)
			{
				playerHealth->TakeDamage(ec->damage);
			}
		}
	}
}

void EnemyCollisionScript::OnCollisionEnter(const Collider& other)
{
	// contact damage
	Registry* registry = CEO::Get<Registry>();

	LayerComponent* otherLayer = registry->GetComponent<LayerComponent>(other.entity);
	if ((static_cast<unsigned long long>(1) << otherLayer->layer) & CEO::Get<LayerManager>()->GetLayerMaskByName("Player"))
	{
		HierarchyComponnent* hc = GetComponent<HierarchyComponnent>(*CEO::Get<Registry>());
		EnemyComponent* ec{ nullptr };
		if (hc->parent)
			ec = registry->GetComponent<EnemyComponent>(hc->parent);
		else
			ec = registry->GetComponent<EnemyComponent>(entity);

		if (ec)
		{
			HierarchyComponnent* phc = registry->GetComponent<HierarchyComponnent>(other.entity);

			HealthScript* playerHealth{ nullptr };
			if (phc->parent)
			{
				playerHealth = registry->GetComponent<HealthScript>(phc->parent);
			}
			else
			{
				playerHealth = registry->GetComponent<HealthScript>(other.entity);
			}
			if (playerHealth)
				playerHealth->TakeDamage(ec->damage);
		}
	}
}

void EnemyCollisionScript::OnDamageTaken(int)
{
	
	FlashingVFX* flashing = GetComponent<FlashingVFX>(*CEO::Get<Registry>());
	EnemyStatusEffectScript* statusEffect = GetComponent<EnemyStatusEffectScript>(*CEO::Get<Registry>());
	if (flashing && 
		(statusEffect->slow.timeElapsed >= statusEffect->slow.duration
		&& statusEffect->poison.timeElapsed >= statusEffect->poison.duration
		&& statusEffect->freeze.timeElapsed >= statusEffect->freeze.duration))
		flashing->StartFlashing();

	/*AudioComponent* hitSFX = GetComponent<AudioComponent>(*CEO::Get<Registry>());
	if (hitSFX)*/

	
			//hitSFX->audio->Play(volume, 1, mapValue);
}

void EnemyCollisionScript::OnDeath()
{
	Registry* registry = CEO::Get<Registry>();
	TransformComponent* tC = GetComponent<TransformComponent>(*CEO::Get<Registry>());

	Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tC->translate);

	Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

	float maxDistance = size.Length() * 1.5f;   // allow beyond screen
	float dist = distanceInView.Length();

	float normalized = dist / maxDistance;
	normalized = std::clamp(normalized, 0.0f, 1.0f);

	// Smooth falloff (quadratic)
	float volume = 1.0f - (normalized);

	float mapValue = distanceInView.x / (size.x);
	mapValue = std::clamp(mapValue, -0.7f, 0.7f);
	std::string enemyName = registry->GetComponent<NameComponent>(entity)->name;
	size_t altPos = enemyName.find("Alt");
	if (altPos != std::string::npos)
	{
		enemyName = enemyName.substr(0, altPos);
	}

	std::string rand = std::to_string(WeightedRNG::getRand("Collision") + 1);
	if (enemyName == "Guy")
	{
		volume = volume * 0.5f;
	}
	if (enemyName == "Oni")
	{
		rand = std::to_string((std::rand() % 8) + 1);
	}
	
	
	std::string sound = "SFX\\Enemy\\" + enemyName + "\\" + enemyName + "Death" + rand + ".wav";
	CEO::Instance().GetManager<ResourceManager>()->GetAudio(sound).Play(volume, 1, mapValue);

	CEO::Get<AchievementManager>()->UpdateTracker("ENEMY");
	GetComponent<ActiveComponent>(*CEO::Get<Registry>())->isActiveSelf = false;
	CEO::Get<MapManager>()->SetEnemiesKilled(1);
} 