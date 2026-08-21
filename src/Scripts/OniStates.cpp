/**___________________________________________________________________________/
@file          OniStates.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

StateMachine for the oni enemy

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"
#include "../CoreLib/Camera.h"


OniStateMachine::OniStateMachine() {
	StateList[es::attack] = std::make_shared<OniAttackState>();
	StateList[es::chase] = std::make_shared<OniChaseState>();

	StartState = StateList[es::chase];
	CurrState = StateList[es::chase];
	CurrStateName = es::chase;
}


OniChaseState::OniChaseState() {
	transitions.push_back(std::make_shared<ChaseAttackTransition>());
}

void OniChaseState::OnStart(Registry& r, EntityRegistry::Entity e) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.oniStruct.walkID);
	ActiveComponent* aac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.walkID);
	

	ac->currFrame = 0;
	aac->isActiveSelf = true;
	ActiveComponent* atc = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.attID); atc->isActiveSelf = false;
	ec->targetPos = es::GetPlayerPos(r);

}

void OniChaseState::OnUpdate(Registry& r, EntityRegistry::Entity e, float ) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	// Pathfind towards the player
	ec->targetPos = es::GetPlayerPos(r);

	std::pair<Vec2, Vec2> ret = CEO::Instance().GetManager<Pathfind>()->MoveTo(tc->translate, ec->targetPos);
	ec->targetPos = ret.first;
	ret.second.Normalise();
	pc->velocity = ret.second * ec->velocity;

	ActiveComponent* atc = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.attID); atc->isActiveSelf = false;
}

void OniChaseState::OnExit(Registry&, EntityRegistry::Entity) {}



OniAttackState::OniAttackState() {
	transitions.push_back(std::make_shared<AttackChaseTransition>());
	attackId = 0;
}

void OniAttackState::OnStart(Registry& r, EntityRegistry::Entity e) {
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->lockFacing = true;
	pc->velocity = { 0,0 };
	attackDone = false;

	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.oniStruct.attID);
	ActiveComponent* aac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.attID);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	ac->timeElapsed = 0;
	ac->currFrame = 0;
	ac->isPlaying = true;
	aac->isActiveSelf = true;

	ActiveComponent* wac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.walkID);

	wac->isActiveSelf = false;

	// Audio playing to be an audio cue for when the enemy starts attacking
	Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tc->translate);

	Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

	float maxDistance = size.Length() * 1.5f;   // allow beyond screen
	float dist = distanceInView.Length();

	float normalized = dist / maxDistance;
	normalized = std::clamp(normalized, 0.0f, 1.0f);

	// Smooth falloff (quadratic)
	float volume = (1.0f - (normalized));

	float mapValue = distanceInView.x / (size.x);
	mapValue = std::clamp(mapValue, -0.7f, 0.7f);
	std::string rand = std::to_string((std::rand() % 7) + 1);

	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Oni\\OniAttack"+ rand +".wav").Play(volume, 1, mapValue);

}

void OniAttackState::OnUpdate(Registry& r, EntityRegistry::Entity e, float) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(ec->enemyStruct.oniStruct.attID);
	ActiveComponent* wac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.walkID);

	r.GetComponent<PhysicsComponent>(e)->velocity = Vec2();

	wac->isActiveSelf = false;

	if (ac->currFrame == 8 && attackDone == false)
	{
		attackDone = true;
		// Audio playing to be an audio cue for when the enemy starts attacking
		TransformComponent* tc = r.GetComponent<TransformComponent>(e);
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
		std::string rand = std::to_string((std::rand() % 3) + 1);

		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Oni\\Whoosh" + rand + ".wav").Play(volume, 1, mapValue);
	}

	// Activate extended hitbox when he swings
	if (ac->currFrame > 7) {
		ActiveComponent* eac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.hit2ID);
		eac->isActiveSelf = true;
	}

	if (!ac->isPlaying) ec->flags |= es::FLAGS::FINISH_ATTACK;
}

void OniAttackState::OnExit(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);

	ActiveComponent* wac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.walkID);
	wac->isActiveSelf = true;

	ActiveComponent* eac = r.GetComponent<ActiveComponent>(ec->enemyStruct.oniStruct.hit2ID);
	eac->isActiveSelf = false;

	ec->flags = 0;
	ec->lockFacing = false;

}