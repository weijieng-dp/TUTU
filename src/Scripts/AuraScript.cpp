/*!
@file       AuraScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Implements the AuraScript class. Handles damage application and aura-based
			projectile interactions when the aura collider triggers with other entities.
*/

#include "AuraScript.h"
#include "HealthScript.h"
#include "ProjectileScript.h"
#include "../CoreLib/StatsManager.h"

void AuraScript::OnStart(Registry& )
{
};


void AuraScript::OnUpdate(Registry&, float, bool)
{
};

void AuraScript::OnFixedUpdate(Registry&, float, bool)
{
};

void AuraScript::OnTriggerEnter(Collider const& other) {
	Registry* registry = CEO::Get<Registry>();
	ProjectileScript* projectile = parentProjectile.GetComponent<ProjectileScript>();
	if (registry->HasComponent<HealthScript>(other.entity))
	{
		registry->GetComponent<HealthScript>(other.entity)->TakeDamage(3);
	}
	if (CEO::Get<StatsManager>()->auraBuff.GetNetValue()) {
		projectile->ProjectileInteractions(registry, other);
	}
}
