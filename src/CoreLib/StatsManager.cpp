/*!
@file       StatsManager.cpp
@author     Ng Wei Jie (weijie.ng) (20%)
@author     Kaeden Tan (kaedenjiawei.tan) (80%)
@date		03/02/2026

This file implements the functionality of the StatsManager system and
the runtime behavior of status effects used by the player.

It contains the implementation of player stat initialization, reset
logic, projectile type management, and stat lookup functionality.
The StatsManager uses reflection to automatically register all
statistics and resources that derive from the IValue interface,
allowing them to be accessed dynamically through string identifiers.

Additionally, this file implements the update behavior for status
effects such as poison, slow, and freeze. These effects modify
gameplay values such as health or movement speed over time based
on their configured duration and effectiveness.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*____________________________________________________________________________*/



#include "StatsManager.h"
#include "pch.h"

// Initializes lookup table for reflected members
StatsManager::StatsManager() : projectiletype{}, memberLookup{}, treasureCollected{ false } {
	refl::type_descriptor<StatsManager> td;
	refl::util::for_each(td.members, [&](auto member, size_t) {
		if constexpr (std::is_base_of_v<IValue, typename decltype(member)::value_type>) {
			memberLookup[member.name.str()] = &(member(*this));
		}
		else {
			LOGI("%s is neither a valid stat or valid resource", member.name.str());
		}
		if constexpr (std::is_base_of_v<StatusEffectStat, typename decltype(member)::value_type>) {
			statusEffects[member.name.str()] = &(member(*this));
		}
		});
}

// Resets all player statistics, projectile modifiers, status effects, and special abilities.
void StatsManager::ResetPlayerStats()
{
	for (auto const& [key, value] : memberLookup) {
		value->Reset();
	}
	projectiletype.clear();

	ResetLuck();
	ResetTreasureCollected();
}

// Restores all player resources (e.g., health) to their starting values.
void  StatsManager::ResetPlayerResources() {
	health.Reset();
	health.Max(&maxHealth);
}

// Fully resets the player state including stats, resources, and temporary modifiers.
void StatsManager::ResetPlayer() {
	ResetPlayerStats();
	ResetPlayerResources();
	ResetLuck();
}

// Assigns a strength value to a specific projectile type.
void StatsManager::SetProjectileType(std::string Projectilename, float strength)
{
	projectiletype[Projectilename] = strength;
}

// Returns the strength value associated with a projectile type.
float StatsManager::GetProjectileType(std::string projectileName)
{
	if (projectiletype.find(projectileName) != projectiletype.end())
		return projectiletype[projectileName];
	return 0.0f;
}

// Retrieves a stat or resource by its string name.
IValue* StatsManager::GetByName(std::string const& statName) {
	if (auto search = memberLookup.find(statName); search != memberLookup.end()) {
		return search->second;
	}
	else {
		return nullptr;
	}
}
// Sets the rarity value for the current item.
void StatsManager::SetRarity(int value) {
	currentItemRarity = value;
}

// Adds luck to the player's current luck value (clamped to valid range).
void StatsManager::DepositLuck(int value) {
	luck = std::clamp(luck + value, 0, 3);
}

// Resets the player's accumulated luck to zero.
void StatsManager::ResetLuck() {
	luck = 0;
}

// Returns the current item rarity value.
int StatsManager::GetRarity() const {
	return std::clamp(currentItemRarity, 1, 4);
}

// Returns the player's current luck value.
int StatsManager::GetLuck() const {
	return luck;
}

// Returns whether treasure has been collected.
bool StatsManager::GetTreasureCollected() const {
	return treasureCollected;
}

// Marks treasure as collected.
void StatsManager::SetTreasureCollected() {
	treasureCollected = true;
}

// Resets the treasure collected state.
void StatsManager::ResetTreasureCollected() {
	treasureCollected = false;
}

// Updates poison over time and applies periodic health damage while active.
double PoisonEffect::UpdateCallback(float dt, double hp){
	timeElapsed += dt;                           // always advance so the effect can expire/clear
	if (tickCount < 3) {                         // deal at most 3 ticks per application
		tickTimeElapsed += dt;
		if (tickTimeElapsed >= 1.0f) {           // one tick per second
			hp -= effectiveness;                 // damage per tick = poison value
			tickTimeElapsed -= 1.0f;
			tickCount++;
		}
	}
	return hp;
}

// Recalculates poison effect parameters based on the current poison strength.
void PoisonStat::Recalculate() {
	if (strength > 0) {
		effect.duration = 3.f;
		effect.effectiveness = static_cast<float>(strength);
	}
	else {
		effect.duration = 0;
		effect.effectiveness = 0;
	}
}

// Applies a slow modifier to movement speed while the slow effect is active.
double SlowEffect::UpdateCallback(float dt, double movespeed) {
	if (timeElapsed < duration) {
		movespeed = movespeed * (1 - effectiveness);
		timeElapsed += dt;
	}
	return movespeed;
}

// Recalculates slow effect duration and slowdown percentage from strength.
void SlowStat::Recalculate() {
	if (strength >= 2) {              // Chocolate + Chocolate Strawberry held -> evolved tier
		effect.duration = 3.f;
		effect.effectiveness = 0.9f;   // enemy speed x0.1 for 3s
	}
	else if (strength > 0) {         // Chocolate Daifuku only
		effect.duration = 2.f;
		effect.effectiveness = 0.5f;   // enemy speed x0.5 for 2s
	}
	else {
		effect.duration = 0;
		effect.effectiveness = 0;
	}
}

double FreezeEffect::UpdateCallback(float dt, double hp) {
	if (timeElapsed < duration) {
		if (buildUp >= threshold) {
			hp = hp - effectiveness;
			buildUp = 0;
		}
		timeElapsed += dt;
	}
	return hp;
}

// Recalculates freeze duration and effectiveness from the current strength.
void FreezeStat::Recalculate() {
	if (strength >= 2) {             // Blue + Blue Strawberry held -> evolved tier
		effect.duration = 3.f;
		effect.effectiveness = 15.f;  // 15 dmg every 3 hits
	}
	else if (strength > 0) {         // Blue Daifuku only
		effect.duration = 3.f;
		effect.effectiveness = 10.f;  // 10 dmg every 3 hits
	}
	else {
		effect.duration = 0;
		effect.effectiveness = 0.f;
	}
}

double KnockbackEffect::UpdateCallback(float dt, double) {
	if (timeElapsed < duration) {
		timeElapsed += dt;
		return true;
	}
	return false;
}

void KnockbackStat::Recalculate() {
	if (strength > 0) {
		effect.duration = 0.2f;
		effect.effectiveness = static_cast<float>(strength * 40000.f);
	}
	else {
		effect.duration = 0;
		effect.effectiveness = 0.f;
	}
}

bool StatsManager::GodMode() {
	godMode = !godMode;
	projectileDamage.AddMultiplier(godMode ? 1.f : -1.f);
	minHealth.AddOffset(godMode ? maxHealth.Get() : -minHealth.Get());
	return godMode;
}