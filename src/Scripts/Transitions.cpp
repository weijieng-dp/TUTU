/**___________________________________________________________________________/
@file          Transitions.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Shared transitions used by the enemy statemachiens.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyConfig.h"
#include "EnemyStates.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"


bool ChaseIdleTransition::Compare(Registry& r, EntityRegistry::Entity e) { 
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	return ec->timeCount > ec->maxTime;
}
std::string ChaseIdleTransition::getNextState() { return es::idle; }



bool ChaseAttackTransition::Compare(Registry& r, EntityRegistry::Entity e) {
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	float dist = (tc->translate - es::GetPlayerPos(r)).LengthSquared();
	return dist <= ec->attackRange * ec->attackRange;
}
std::string ChaseAttackTransition::getNextState() { return es::attack; }



bool AttackIdleTransition::Compare(Registry& r, EntityRegistry::Entity e) { 
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	return (ec->flags & es::FLAGS::FINISH_ATTACK);
}
std::string AttackIdleTransition::getNextState() { return es::idle; }

bool AttackChaseTransition::Compare(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	return (ec->flags & es::FLAGS::FINISH_ATTACK);
}
std::string AttackChaseTransition::getNextState() { return es::chase; }



bool IdleAttackTransition::Compare(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	return (ec->flags & es::EXIT_IDLE);
}
std::string IdleAttackTransition::getNextState() { return es::attack; }



bool IdleChaseTransition::Compare(Registry& r, EntityRegistry::Entity e) { 
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	return ec->timeCount > ec->maxTime;
}
std::string IdleChaseTransition::getNextState() { return es::chase; }

bool ChaseAttackLTransition::Compare(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	return ec->enemyStruct.larvaStruct.attTimer > ec->enemyStruct.larvaStruct.attCooldown;
}
std::string ChaseAttackLTransition::getNextState() { return es::attack; }

bool IdleAttackETransition::Compare(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	return ec->timeCount > ec->maxTime;
}
std::string IdleAttackETransition::getNextState() { return es::attack; }
