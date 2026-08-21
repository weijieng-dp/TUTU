/*!
@file       HealthbarScript.cpp
@author     Ng Wei Jie (weijie.ng) (100%)
@date       5/2/2026
@brief
    Implements the HealthbarScript, which is responsible for visually
    representing an entity's health using heart icons in the UI.

    The script spawns, positions, and updates heart GameObjects based on
    the target entity's current and maximum health. It supports full,
    half, and empty heart states.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*/
/*________________________________________________________________________*/



#include "HealthbarScript.h"
#include "../CoreLib/HierarchyManager.h"
#include "HealthScript.h"
#include "../CoreLib/StatsManager.h"

void HealthbarScript::OnStart(Registry& )
{
	//setting the entity itself as a gameobject
	//float TotalHealthPadding = 0;

	HealthScript* health =  TargetGameobject.GetComponent<HealthScript>();
	int totalHearts = (health->maxHealth + 1) / 2;
	// spawn in additional hearts
	for (size_t i = HealthGameObjects.size(); i < totalHearts; i++)
	{
		//push gameobject
		AddHealthGameobject();
	}
	FullHeartTexture = &CEO::Get<ResourceManager>()->GetTexture("ui/game/full_heart.png");
	HalfHeartTexture = &CEO::Get<ResourceManager>()->GetTexture("ui/game/half_heart.png");
	EmptyHeartTexture = &CEO::Get<ResourceManager>()->GetTexture("ui/game/empty_heart.png");
	UpdateHealthbar(health->currHealth, health->maxHealth);
	// spawn in additional hearts
	for (size_t i = 0; i < CEO::Get<StatsManager>()->maxHealth.Max(); i++)
	{
		//push gameobject
		AddHealthGameobject();
	}
};


void HealthbarScript::OnUpdate(Registry&, float , bool )
{
	StatsManager* stats = CEO::Get<StatsManager>();
	UpdateHealthbar(static_cast<int>(stats->health.GetNetValue()), stats->maxHealth.Get());
};

void HealthbarScript::OnFixedUpdate(Registry&, float, bool)
{
}
void HealthbarScript::AddHealthGameobject()
{
	GameObject CurGameObject(GetComponent<HealthbarScript>(*CEO::Get<Registry>())->entity);

	size_t paddingCount = HealthGameObjects.size();

	GameObject healthObject = HealthPrefab.Instantiate();
	HealthGameObjects.push_back(healthObject);
	CurGameObject.SetChild(healthObject.GetEntityID());

	//setup padding for healthbar
	UITransformComponent* transform = healthObject.GetComponent<UITransformComponent>();
	transform->relativePos.x += paddingCount * HealthPadding;
}

void HealthbarScript::UpdateHealthbar(int currHealth, int maxHealth)
{
	currHealth = std::clamp(currHealth, 0, maxHealth);

	int totalHearts = (maxHealth + 1) / 2;   // ceil(maxHealth / 2)
	int fullHearts = currHealth / 2;
	bool hasHalf = (currHealth % 2) == 1;

	// spawn in additional hearts
	for (size_t i = HealthGameObjects.size(); i < totalHearts; i++)
	{
		//push gameobject
		AddHealthGameobject();
	}
	for (auto& heart : HealthGameObjects) {
		heart.GetComponent<ActiveComponent>()->isActiveSelf = false;
	}
	for (int i = 0; i < totalHearts; ++i)
	{
		HealthGameObjects[i].GetComponent<ActiveComponent>()->isActiveSelf = true;
		HealthGameObjects[i]
			.GetComponent<SpriteRendererComponent>()
			->texture = EmptyHeartTexture;
	}

	for (int i = 0; i < fullHearts; ++i)
	{
		HealthGameObjects[i]
			.GetComponent<SpriteRendererComponent>()
			->texture = FullHeartTexture;
	}

	if (hasHalf && fullHearts < totalHearts)
	{
		HealthGameObjects[fullHearts]
			.GetComponent<SpriteRendererComponent>()
			->texture = HalfHeartTexture;
	}
}
;
