/**___________________________________________________________________________/
@file          GuyStates.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

StateMachine for the guy enemy

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "GameobjectPoolScript.h"
#include "ProjectileScript.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"
#include "../CoreLib/Camera.h"

GuyStateMachine::GuyStateMachine() {
	StateList[es::attack] = std::make_shared<GuyAttackState>();
	StateList[es::idle] = std::make_shared<GuyIdleState>();

	StartState = StateList[es::idle];
	CurrState = StateList[es::idle];
	CurrStateName = es::idle;
}


GuyAttackState::GuyAttackState() {
	transitions.push_back(std::make_shared<AttackIdleTransition>());
}

void GuyAttackState::OnStart(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	NameComponent* nc = r.GetComponent<NameComponent>(e);
	std::string an = nc->name == "Guy" ? EnemyConfig::guyAttackAnim:EnemyConfig::guyAttackAnimAlt;

	ec->timeCount = 0.f;
	ec->flags = 0;
	ec->enemyStruct.guyStruct.shotBullet = false;

	pc->velocity = { 0,0 };
	ac->currAnim = &CEO::Get<ResourceManager>()->GetAnimation(an);
	ac->isPlaying = true;
	ac->currFrame = 0;

	// Lead calculation for attacking the player
	Pathfind& path = *CEO::Get<Pathfind>();
	Vec2 playerPosition = path.GetPlayerPos();
	PhysicsComponent* ppc = r.GetComponent<PhysicsComponent>(path.GetPlayerID());
	Vec2 playerDirection = ppc->velocity;	// This should not have dt in it.

	// This is effectively me setting the prediction to the player's position 0.5s into the future.
	playerPosition += playerDirection * 0.5; 
	ec->targetPos = (playerPosition - tc->translate).Normalised() * 1200.f;

	// This is adding a +- 15 degrees of random spread to the shot.
	float degrees = static_cast<float>(rand()) / RAND_MAX * 30.f - 15.f; 
	Mat3 rot;
	Mat3RotDeg(rot, degrees);
	ec->targetPos = rot * ec->targetPos;

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

	std::string rand = std::to_string((std::rand() % 2) + 1);

	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Guy\\GuyGrunt" + rand + ".wav").Play(volume, 1, mapValue);
}

void GuyAttackState::OnUpdate(Registry& r, EntityRegistry::Entity e, float) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);

	// Keep resetting it back to 0 if the player bumps into the dood
	pc->velocity = Vec2();

	if (!ac->isPlaying) ec->flags |= es::FINISH_ATTACK;
	if (ac->currFrame == 5 && !ec->enemyStruct.guyStruct.shotBullet) {

		// Initializing stats for the projectile.
		// The projectileManager is initialized by EnemyConfig.cpp 
		// It is a void pointer as the engine is not able to know
		// the type of the GameobjectPoolScript, since that is
		// defined in the cpp file.

		GameObject projectile = ((GameobjectPoolScript*)(ec->enemyStruct.guyStruct.projectileManager))->GetGameobject("GuyProjectile");
		ProjectileScript* psProj = projectile.GetComponent<ProjectileScript>();
		TransformComponent* tcProj = projectile.GetComponent<TransformComponent>();
		PhysicsComponent* pcProj = projectile.GetComponent<PhysicsComponent>();
		ActiveComponent* acProj = projectile.GetComponent<ActiveComponent>();

		ec->lockFacing = true;
		ec->enemyStruct.guyStruct.shotBullet = true;
		psProj->damage = ec->damage;
		psProj->lifespan = 5.f;
		psProj->timeAlive = 0.f;
		pcProj->velocity = ec->targetPos;
		tcProj->translate = r.GetComponent<TransformComponent>(e)->translate;
		acProj->isActiveSelf = true;

	}
}

void GuyAttackState::OnExit(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->lockFacing = false;
}


GuyIdleState::GuyIdleState() {
	transitions.push_back(std::make_shared<IdleAttackTransition>());
}	

void GuyIdleState::OnStart(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	NameComponent* nc = r.GetComponent<NameComponent>(e);
	std::string an = nc->name == "Guy" ? EnemyConfig::guyIdleAnim: EnemyConfig::guyIdleAnimAlt;

	ec->enemyStruct.guyStruct.idleUnmoving = false;
	ec->enemyStruct.guyStruct.attTimer = 0.f;
	ec->maxTime = 3.f;
	ec->timeCount = 0.f;
	ec->flags = 0;
	pc->velocity = { 0,0 };
	ac->currAnim = &CEO::Get<ResourceManager>()->GetAnimation(an);
	ac->isPlaying = true;
	ac->currFrame = 0;
	ec->lockFacing = false;
}

void GuyIdleState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	Pathfind& path = *CEO::Get<Pathfind>();

	ec->timeCount += dt;
	ec->enemyStruct.guyStruct.attTimer += dt;

	if (pc->velocity.LengthSquared() == 0 && !ec->enemyStruct.guyStruct.idleUnmoving) {
		// Just entered the idle state, find for a random direction to move towards.
		// If the player is far away, he can move any random direction.
		// But if the player is close enough to him, he will move backwards in an
		// attempt to maintain distance with the player
		if ((tc->translate - path.GetPlayerPos()).LengthSquared() < ec->attackRange * ec->attackRange * 0.64f) {
			Vec2 dir = (tc->translate - path.GetPlayerPos()).Normalised();
			float degrees = static_cast<float>(rand()) / RAND_MAX * 90 - 45.f;

			Mat3 rot;
			Mat3RotDeg(rot, degrees);
			dir = rot * dir; // Give it a bit of a random rotation so the enemy does not move perfectly in line away from the player
			pc->velocity = dir * ec->velocity;
		}
		else {
			float degrees = static_cast<float>(rand()) / RAND_MAX * 360.f; // Move in a completely random direction
			Mat3 rot;
			Mat3RotDeg(rot, degrees);
			pc->velocity = (rot * Vec2{ 1,0 }) * ec->velocity;
		}
	}

	// As long as the player is in range, and his attack is off cooldown, he can 
	// immediately exit the idle state to start attacking the player
	if (ec->enemyStruct.guyStruct.attTimer >= EnemyConfig::guyAttackCd) {
		if ((tc->translate - path.GetPlayerPos()).LengthSquared() < ec->attackRange * ec->attackRange) {
			ec->flags |= es::EXIT_IDLE;
		}
	}

	if (ec->timeCount > ec->maxTime) {
		// This means he finished moving, he will now afk on a spot for 2s
		// Can be randomized in the future.
		if (!ec->enemyStruct.guyStruct.idleUnmoving) {
			ec->enemyStruct.guyStruct.idleUnmoving = true;
			pc->velocity = { 0,0 };
			ec->timeCount = 0.f;
			ec->maxTime = 2.f;
		}
		else {
			// Restart the idle loop.
			OnStart(r, e);
		}
	}

}

void GuyIdleState::OnExit(Registry&, EntityRegistry::Entity) {}