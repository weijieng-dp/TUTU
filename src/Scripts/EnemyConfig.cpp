/**___________________________________________________________________________/
@file          EnemyConfig.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple static class used for initializing values that enemies use from a json
file.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyConfig.h"
#include "EnemyStates.h"
#include "FlipState.h"
#include "GameobjectPoolScript.h"

// initializing
std::string EnemyConfig::eyeAttackAnim{};
std::string EnemyConfig::eyeAttackChargeAnim{};
std::string EnemyConfig::eyeIdleAnim{};
std::string EnemyConfig::eyeChaseAnim{};
std::string EnemyConfig::girlIdleAnim{};
std::string EnemyConfig::girlIdleAnim2{};
std::string EnemyConfig::girlIdleAnim2Alt{};
std::string EnemyConfig::girlChaseAnim{};
std::string EnemyConfig::girlChaseAnimAlt{};
std::string EnemyConfig::girlChargeAnim{};
std::string EnemyConfig::girlChargeAnimAlt{};
std::string EnemyConfig::oniAttack1Anim{};
std::string EnemyConfig::oniAttack2Anim{};
std::string EnemyConfig::oniAttack2AnimAlt{};
std::string EnemyConfig::oniChaseAnim{};
std::string EnemyConfig::oniChaseAnimAlt{};
std::string EnemyConfig::guyAttackAnim{};
std::string EnemyConfig::guyAttackAnimAlt{};
std::string EnemyConfig::guyIdleAnim{};
std::string EnemyConfig::guyIdleAnimAlt{};
std::string EnemyConfig::larvaAttackAnim{};
std::string EnemyConfig::larvaChaseAnim{};

float EnemyConfig::eyeIdleMin;
float EnemyConfig::eyeIdleMax;
float EnemyConfig::eyeChaseMax;
float EnemyConfig::girlIdleMin;
float EnemyConfig::girlIdleMax;
float EnemyConfig::girlChaseMax;
float EnemyConfig::guyAttackCd;
float EnemyConfig::larvaIdleMin;
float EnemyConfig::larvaIdleMax;
float EnemyConfig::larvaChaseMax;

void EnemyConfig::Init() {
	static bool init{ false };
	if (init) return;
#ifdef PLATFORM_WINDOWS
	json j("Assets\\EnemyConfig.json");
#else
    json j("EnemyConfig.json");
#endif
	json::object* o{ j.GetObj("animation") };

	if (std::string* s = o->GetValue("eyeAttackAnim")->GetString(); s)		eyeAttackAnim = *s;
	if (std::string* s = o->GetValue("eyeAttackChargeAnim")->GetString(); s)	eyeAttackChargeAnim = *s;
	if (std::string* s = o->GetValue("eyeChaseAnim")->GetString(); s)			eyeChaseAnim = *s;
	if (std::string* s = o->GetValue("eyeIdleAnim")->GetString(); s)			eyeIdleAnim = *s;
	if (std::string* s = o->GetValue("girlIdleAnim")->GetString(); s)			girlIdleAnim = *s;
	if (std::string* s = o->GetValue("girlIdleAnim2")->GetString(); s)		girlIdleAnim2 = *s;
	if (std::string* s = o->GetValue("girlIdleAnim2Alt")->GetString(); s)		girlIdleAnim2Alt = *s;
	if (std::string* s = o->GetValue( "girlChaseAnim")->GetString(); s)			girlChaseAnim = *s;
	if (std::string* s = o->GetValue( "girlChaseAnimAlt")->GetString(); s)		girlChaseAnimAlt = *s;
	if (std::string* s = o->GetValue("girlChargeAnim")->GetString(); s)		girlChargeAnim = *s;
	if (std::string* s = o->GetValue("girlChargeAnimAlt")->GetString(); s)	girlChargeAnimAlt = *s;
	if (std::string* s = o->GetValue("oniAttack1Anim")->GetString(); s)		oniAttack1Anim = *s;
	if (std::string* s = o->GetValue("oniAttack2Anim")->GetString(); s)		oniAttack2Anim = *s;
	if (std::string* s = o->GetValue("oniAttack2AnimAlt")->GetString(); s)	oniAttack2AnimAlt = *s;
	if (std::string* s = o->GetValue( "oniChaseAnim")->GetString(); s)			oniChaseAnim = *s;
	if (std::string* s = o->GetValue( "oniChaseAnimAlt")->GetString(); s)		oniChaseAnimAlt = *s;
	if (std::string* s = o->GetValue("guyAttackAnim")->GetString(); s)		guyAttackAnim = *s;
	if (std::string* s = o->GetValue("guyAttackAnimAlt")->GetString(); s)		guyAttackAnimAlt = *s;
	if (std::string* s = o->GetValue("guyIdleAnim")->GetString(); s)			guyIdleAnim = *s;
	if (std::string* s = o->GetValue("guyIdleAnimAlt")->GetString(); s)		guyIdleAnimAlt = *s;
	if (std::string* s = o->GetValue("larvaAttackAnim")->GetString(); s)		larvaAttackAnim = *s;
	if (std::string* s = o->GetValue( "larvaChaseAnim")->GetString(); s)		larvaChaseAnim = *s;

	o = j.GetObj("time");

	if (float* s = o->GetValue("eyeIdleMin")->GetFloat(); s)		eyeIdleMin = *s;
	if (float* s = o->GetValue("eyeIdleMax")->GetFloat(); s)		eyeIdleMax = *s;
	if (float* s = o->GetValue("eyeChaseMax")->GetFloat(); s)		eyeChaseMax = *s;
	if (float* s = o->GetValue("girlIdleMin")->GetFloat(); s)		girlIdleMin = *s;
	if (float* s = o->GetValue("girlIdleMax")->GetFloat(); s)		girlIdleMax = *s;
	if (float* s = o->GetValue("girlChaseMax")->GetFloat(); s)		girlChaseMax = *s;
	if (float* s = o->GetValue("guyAttackCd")->GetFloat(); s)		guyAttackCd = *s;
	if (float* s = o->GetValue("larvaIdleMin")->GetFloat(); s)		larvaIdleMin = *s;
	if (float* s = o->GetValue("larvaIdleMax")->GetFloat(); s)		larvaIdleMax = *s;
	if (float* s = o->GetValue("larvaChaseMax")->GetFloat(); s)		larvaChaseMax = *s;

	init = true;
}

void EnemyConfig::OniInit(EnemyComponent* ec, Registry::Entity e) {
	auto& oniStuff = ec->enemyStruct.oniStruct;
	HierarchyComponnent* hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(e);
	Registry::Entity ID{};
	while (!(oniStuff.walkID && oniStuff.attID && oniStuff.hit1ID && oniStuff.hit2ID)) {
		if (hc->firstChild) ID = hc->firstChild;
		if (hc->nextSibling) ID = hc->nextSibling;

		NameComponent* nc = CEO::Get<Registry>()->GetComponent<NameComponent>(ID);

		if (nc->name == "Walk") {
			oniStuff.walkID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else if (nc->name == "Attack") {
			oniStuff.attID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else if (nc->name == "Hitbox1") {
			oniStuff.hit1ID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else if (nc->name == "Hitbox2") {
			oniStuff.hit2ID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else {
			LOGE("UNKOWN NAME OR SMTH!!! %s", nc->name.c_str());
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
	}

	StateComponent* sc = CEO::Get<Registry>()->GetComponent<StateComponent>(e);

	sc->stateMachines.push_back(std::make_shared<OniStateMachine>());
	sc->stateMachines.back()->OnStart(*CEO::Get<Registry>(), e);
	sc->stateMachines.push_back(std::make_shared<FlipStateMachine>());
	sc->stateMachines.back()->OnStart(*CEO::Get<Registry>(), e);
}

void EnemyConfig::EyeInit(EnemyComponent* ec, Registry::Entity e) {
	auto& eyeStuff = ec->enemyStruct.eyeStruct;
	HierarchyComponnent* hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(e);
	Registry::Entity ID{};
	while (!(eyeStuff.hitID && eyeStuff.normID)) {
		if (hc->firstChild) ID = hc->firstChild;
		if (hc->nextSibling) ID = hc->nextSibling;

		NameComponent* nc = CEO::Get<Registry>()->GetComponent<NameComponent>(ID);

		if (nc->name == "Normal") {
			eyeStuff.normID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else if (nc->name == "Hitbox") {
			eyeStuff.hitID = ID;
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
		else {
			LOGE("UNKOWN NAME OR SMTH!!! %s", nc->name.c_str());
			hc = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(ID);
		}
	}

	StateComponent* sc = CEO::Get<Registry>()->GetComponent<StateComponent>(e);

	sc->stateMachines.push_back(std::make_shared<EyeStateMachine>());
	sc->stateMachines.back()->OnStart(*CEO::Get<Registry>(), e);
	sc->stateMachines.push_back(std::make_shared<FlipStateMachine>());
	sc->stateMachines.back()->OnStart(*CEO::Get<Registry>(), e);
}

void EnemyConfig::GuyInit(EnemyComponent* ec, Registry::Entity e) {
	Registry& r = *CEO::Get<Registry>();

	auto goopVec = r.GetEntitiesWithComponent<GameobjectPoolScript>();

	if (goopVec.empty()) {
		// Create one if it does not exist yet
		GameObject go = CreateGameobject();
		go.AddComponent<GameobjectPoolScript>();
		goopVec.push_back(go.GetEntityID());
	}

	GameobjectPoolScript* goop = r.GetComponent<GameobjectPoolScript>(goopVec[0]);
	goop->AddPrefab(GameObject("GuyProjectile"));

	StateComponent* sc = r.GetComponent<StateComponent>(e);

	sc->stateMachines.push_back(std::make_shared<GuyStateMachine>());
	sc->stateMachines.back()->OnStart(r, e);
	sc->stateMachines.push_back(std::make_shared<FlipStateMachine>());
	sc->stateMachines.back()->OnStart(r, e);

	ec->enemyStruct.guyStruct.projectileManager = (void*)goop;
}

void EnemyConfig::LarvaInit(EnemyComponent* ec, Registry::Entity e) {
	Registry& r = *CEO::Get<Registry>();

	auto goopVec = r.GetEntitiesWithComponent<GameobjectPoolScript>();

	if (goopVec.empty()) {
		// Create one if it does not exist yet
		GameObject go = CreateGameobject();
		go.AddComponent<GameobjectPoolScript>();
		goopVec.push_back(go.GetEntityID());
	}

	GameobjectPoolScript* goop = r.GetComponent<GameobjectPoolScript>(goopVec[0]);
	goop->AddPrefab(GameObject("LarvaProjectile"));

	StateComponent* sc = r.GetComponent<StateComponent>(e);

	sc->stateMachines.push_back(std::make_shared<LarvaStateMachine>());
	sc->stateMachines.back()->OnStart(r, e);
	sc->stateMachines.push_back(std::make_shared<FlipStateMachine>());
	sc->stateMachines.back()->OnStart(r, e);

	ec->enemyStruct.larvaStruct.projectileManager = (void*)goop;
	ec->enemyStruct.larvaStruct.attCooldown = 3.0f;
}