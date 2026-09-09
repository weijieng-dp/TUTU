/**___________________________________________________________________________/
@file          EyeStates.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

StateMachine for the eye enemy

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"
#include <cstdlib>
#include "../CoreLib/Camera.h"
#include "WeightedRNG.h"


EyeStateMachine::EyeStateMachine() {
	StateList[es::attack] = std::make_shared<EyeAttackState>();
	//StateList[es::chase] = std::make_shared<EyeChaseState>();
	StateList[es::idle] = std::make_shared<EyeIdleState>();

	StartState = StateList[es::idle];
	CurrState = StateList[es::idle];
	CurrStateName = es::idle;
}



EyeChaseState::EyeChaseState() {
	transitions.push_back(std::make_shared<ChaseAttackTransition>());
	transitions.push_back(std::make_shared<ChaseIdleTransition>());
}

void EyeChaseState::OnStart(Registry& r, EntityRegistry::Entity e) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.eyeStruct.normID);

	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::eyeChaseAnim);
	ec->targetPos = es::GetPlayerPos(r);

	ec->timeCount = 0;
	ec->maxTime = EnemyConfig::eyeChaseMax;

}

void EyeChaseState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	ec->targetPos = es::GetPlayerPos(r);

	// Pathfind towards the player
	std::pair<Vec2, Vec2> ret = CEO::Instance().GetManager<Pathfind>()->MoveTo(tc->translate, ec->targetPos);
	ec->targetPos = ret.first;
	ret.second.Normalise();
	pc->velocity = ret.second * ec->velocity;

	ec->timeCount += dt;
}

void EyeChaseState::OnExit(Registry&, EntityRegistry::Entity) {}



EyeAttackState::EyeAttackState() {
	transitions.push_back(std::make_shared<AttackIdleTransition>());
}

void EyeAttackState::OnStart(Registry& r, EntityRegistry::Entity e) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.eyeStruct.normID);
	CollisionComponent* cc = r.GetComponent<CollisionComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	attackDone = false;
	cc->isTrigger = true;

	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::eyeAttackChargeAnim);
	ac->isPlaying = true;
	pc->velocity = Vec2();
	ec->flags = 0;
	ec->timeCount = 0;
	ec->targetPos = CEO::Get<Pathfind>()->GetPlayerPos();
	ec->enemyStruct.eyeStruct.attVecNorm = (ec->targetPos - tc->translate);
	ec->enemyStruct.eyeStruct.distance = ec->enemyStruct.eyeStruct.attVecNorm.Length();
	ec->enemyStruct.eyeStruct.attVecNorm.Normalise();
}

void EyeAttackState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.eyeStruct.normID);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);

	r.GetComponent<PhysicsComponent>(e)->velocity = Vec2();	// Keep it stationary to deal with knockback effect

	// Start with the charge up animation of the eyeball
	if (!ec->flags && !ac->isPlaying) {
		ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::eyeAttackAnim);
		ac->isPlaying = true;
		ec->flags |= es::FLAGS::START_ATTACK;
	}

	// If the charge up is finished, start the attack animation.
	if (ec->flags & es::FLAGS::START_ATTACK) {
		if (ac->isPlaying) {
			if (ac->currFrame > 3 && ac->currFrame <= 11) {
				// Setting it's own collision to be false while jumping, allowing it to jump
				// over other enemies.
				r.GetComponent<ActiveComponent>(ec->enemyStruct.eyeStruct.hitID)->isActiveSelf = false;

				// This is basically a upside down parabolic graph for the velocity. 
				auto velMag = [](float currTime) { return 3.33f - (8.11f * currTime - 1.82f) * (8.11f * currTime - 1.82f); };	// glorious values
				pc->velocity = std::max(velMag(ec->timeCount),0.f) * ec->enemyStruct.eyeStruct.attVecNorm * ec->velocity;
				ec->timeCount += dt;
				if (ac->currFrame == 10 && !attackDone)
				{
					attackDone = true;
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
					std::string rand = std::to_string(WeightedRNG::getRand("Eye") + 1);

					CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Eye\\EyeAttack" + rand + ".wav").Play(volume, 1, mapValue);
				}
			}
			else if (ac->currFrame > 11) {
				// The jump is finished in the animation, so the collision can be set back to active.
				r.GetComponent<ActiveComponent>(ec->enemyStruct.eyeStruct.hitID)->isActiveSelf = true;
			}
		}
		else {
			ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::eyeAttackAnim);
			ac->isPlaying = true;
			ec->flags |= es::FLAGS::FINISH_ATTACK;
		}
	}

}

void EyeAttackState::OnExit(Registry& r, EntityRegistry::Entity e) {
	CollisionComponent* cc = r.GetComponent<CollisionComponent>(e);
	cc->isTrigger = false;
}



EyeIdleState::EyeIdleState() {
	transitions.push_back(std::make_shared<IdleAttackETransition>());
}

void EyeIdleState::OnStart(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.eyeStruct.normID);

	// Idle and not move for a random amount of time.
	ec->maxTime = static_cast<float>(std::rand()) / RAND_MAX * (EnemyConfig::eyeIdleMax - EnemyConfig::eyeIdleMin) + EnemyConfig::eyeIdleMin;
	ec->timeCount = 0;
	pc->velocity = Vec2();
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(EnemyConfig::eyeIdleAnim);
}

void EyeIdleState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->timeCount += dt;
}

void EyeIdleState::OnExit(Registry&, EntityRegistry::Entity) {}
