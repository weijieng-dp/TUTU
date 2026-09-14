/**___________________________________________________________________________/
@file          LarvaStates.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

StateMachine for the eye enemy

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"
#include "GameobjectPoolScript.h"
#include "ProjectileScript.h"
#include "LarvaProjectileScript.h"
#include <cstdlib>
#include "../CoreLib/Camera.h"
#include "WeightedRNG.h"

LarvaStateMachine::LarvaStateMachine() {
	StateList[es::attack] = std::make_shared<LarvaAttackState>();
	StateList[es::chase] = std::make_shared<LarvaChaseState>();
	StateList[es::idle] = std::make_shared<LarvaIdleState>();

	StartState = StateList[es::idle];
	CurrState = StateList[es::idle];
	CurrStateName = es::idle;
}



LarvaChaseState::LarvaChaseState() {
	transitions.push_back(std::make_shared<ChaseAttackLTransition>());
	transitions.push_back(std::make_shared<ChaseIdleTransition>());
}

void LarvaChaseState::OnStart(Registry& r, EntityRegistry::Entity e) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::larvaChaseAnim);
	ec->targetPos = es::GetPlayerPos(r);

	ec->timeCount = 0;
	ec->maxTime = EnemyConfig::larvaChaseMax;
	ec->lockFacing = false;
}

void LarvaChaseState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	ec->targetPos = es::GetPlayerPos(r);

	float dist = (tc->translate - ec->targetPos).Length();

	ec->timeCount += dt;
	ec->enemyStruct.larvaStruct.attTimer += dt;

	// Pathfind towards the player if player is far
	if (dist > ec->attackRange * 0.8f) {
		std::pair<Vec2, Vec2> ret = CEO::Instance().GetManager<Pathfind>()->MoveTo(tc->translate, ec->targetPos);
		ec->targetPos = ret.first;
		ret.second.Normalise();
		pc->velocity = ret.second * ec->velocity;
	}
	else {
		// Try and move towards the side of the player
		
		// This is basically "on level" enough such that it is fine.
		if (std::abs(ec->targetPos.y - tc->translate.y) < 160) return;

		if (ec->targetPos.x > tc->translate.x && ec->targetPos.y > tc->translate.y ||
			ec->targetPos.x < tc->translate.x && ec->targetPos.y < tc->translate.y) {

			Mat3 rot; Mat3RotDeg(rot, -90);
			Vec2 dir = tc->translate - ec->targetPos;
			dir.Normalise();
			dir = rot * dir;
			pc->velocity = dir * ec->velocity;
		}
		else {
			Mat3 rot; Mat3RotDeg(rot, 90);
			Vec2 dir = tc->translate - ec->targetPos;
			dir.Normalise();
			dir = rot * dir;
			pc->velocity = dir * ec->velocity;
		}
	}

}

void LarvaChaseState::OnExit(Registry&, EntityRegistry::Entity) {}



LarvaAttackState::LarvaAttackState() {
	transitions.push_back(std::make_shared<AttackIdleTransition>());
}

void LarvaAttackState::OnStart(Registry& r, EntityRegistry::Entity e) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);

	//tc->scale.x = 210.0f;
	
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::larvaAttackAnim);
	ac->isPlaying = true;
	pc->velocity = Vec2();
	ec->flags = 0;
	ec->timeCount = 0;
	ec->enemyStruct.larvaStruct.attTimer = 0.0f;
	
	Vec2 targetDir = (tc->translate.x < CEO::Get<Pathfind>()->GetPlayerPos().x ? Vec2{ 1.f,0.f } : Vec2{ -1.f, 0.f });

	tc->scale.x = targetDir.x > 0 ? -210.0f : 210.0f;

	// Spawn projectiles.
	int projCount = std::rand() % 3 + 5;	// 5-7 projectiles
	for (int i{}; i < projCount; i++) {
		GameObject projectile = ((GameobjectPoolScript*)(ec->enemyStruct.larvaStruct.projectileManager))->GetGameobject("LarvaProjectile");
		ProjectileScript* psProj = projectile.GetComponent<ProjectileScript>();
		TransformComponent* tcProj = projectile.GetComponent<TransformComponent>();
		PhysicsComponent* pcProj = projectile.GetComponent<PhysicsComponent>();
		ActiveComponent* acProj = projectile.GetComponent<ActiveComponent>();

		// +- 30 degrees of spread
		float angle = (static_cast<float>(std::rand()) / RAND_MAX) * 60.f - 30.f;
		// 1000 - 1200 initial projectile speed
		float speed = (static_cast<float>(std::rand()) / RAND_MAX) * 200.f + 1000.f;

		Mat3 rot; Mat3RotDeg(rot, angle);
		Vec2 tempDir(targetDir);
		tempDir = rot * tempDir;
		tempDir *= speed;

		ec->lockFacing = true;
		ec->timeCount = 0;
		psProj->damage = ec->damage;
		psProj->lifespan = 3.f;
		psProj->timeAlive = 0.f;
		pcProj->velocity = tempDir;
		tcProj->translate = r.GetComponent<TransformComponent>(e)->translate;
		acProj->isActiveSelf = true;

		// Audio playing to be an audio cue for when the enemy starts attacking
		Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tc->translate);

		Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

		float maxDistance = size.Length() * 1.5f;   // allow beyond screen
		float dist = distanceInView.Length();

		float normalized = dist / maxDistance;
		normalized = std::clamp(normalized, 0.0f, 1.0f);

		// Smooth falloff (quadratic)
		float volume = (1.0f - (normalized)) * 0.5f;

		float mapValue = distanceInView.x / (size.x);
		mapValue = std::clamp(mapValue, -0.7f, 0.7f);
		std::string rand = std::to_string(WeightedRNG::getRand("Larva") + 1);

		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Larva\\LarvaShoot" + rand + ".wav").Play(volume, 1, mapValue);
	}
}

void LarvaAttackState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	//AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);

	r.GetComponent<PhysicsComponent>(e)->velocity = Vec2();	// Keep it stationary to deal with knockback effect

	ec->timeCount += dt;
	// Set the flag when the animation is finished
	if (ec->timeCount > 1.f) {
		ec->flags |= es::FLAGS::FINISH_ATTACK;
	}

}

void LarvaAttackState::OnExit(Registry& r, EntityRegistry::Entity e) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->lockFacing = false;
}



LarvaIdleState::LarvaIdleState() {
	transitions.push_back(std::make_shared<IdleChaseTransition>());
}

void LarvaIdleState::OnStart(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	tc->scale.x = 160.0f;

	// Idle and not move for a random amount of time.
	ec->maxTime = static_cast<float>(std::rand()) / RAND_MAX * (EnemyConfig::larvaIdleMax - EnemyConfig::larvaIdleMin) + EnemyConfig::larvaIdleMin;
	ec->timeCount = 0;
	pc->velocity = Vec2();
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::larvaChaseAnim);
}

void LarvaIdleState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->timeCount += dt;
	ec->enemyStruct.larvaStruct.attTimer += dt;
}

void LarvaIdleState::OnExit(Registry&, EntityRegistry::Entity) {}
