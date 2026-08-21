/**___________________________________________________________________________/
@file          EnemyStates.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple statemachine to flip the sprite based on movement direction
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"

class LeftRightTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) {
		EnemyComponent* ec = registry.GetComponent<EnemyComponent>(e);
		if (ec->lockFacing) return false;
		if (!playerID) {
			auto v = registry.GetEntitiesWithComponent<PlayerControllerScript>();
			if (v.empty()) return false;
			playerID = v[0];
		}
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		TransformComponent* ptc = registry.GetComponent<TransformComponent>(playerID);
		return tc->translate.x > ptc->translate.x;
	}
	std::string getNextState() { return "right"; }
private:
	Registry::Entity playerID{ 0 };
};

class RightLeftTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) {
		EnemyComponent* ec = registry.GetComponent<EnemyComponent>(e);
		if (ec->lockFacing) return false;
		if (!playerID) {
			auto v = registry.GetEntitiesWithComponent<PlayerControllerScript>();
			if (v.empty()) return false;
			playerID = v[0];
		}
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		TransformComponent* ptc = registry.GetComponent<TransformComponent>(playerID);
		return tc->translate.x < ptc->translate.x;
	}
	std::string getNextState() { return "left"; }
private:
	Registry::Entity playerID{ 0 };
};

class LeftState : public IState {
public:
	LeftState() { transitions.push_back(std::make_shared<LeftRightTransition>()); }
	void OnStart(Registry& registry, EntityRegistry::Entity e) {
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		if (tc->scale.x > 0) tc->scale.x *= -1;
	}
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float) {
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		if (tc->scale.x > 0) tc->scale.x *= -1;
	}
	void OnExit(Registry&, EntityRegistry::Entity) {}
};

class RightState : public IState {
public:
	RightState() { transitions.push_back(std::make_shared<RightLeftTransition>()); }
	void OnStart(Registry& registry, EntityRegistry::Entity e) {
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		if (tc->scale.x < 0) tc->scale.x *= -1;
	}
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float) {
		TransformComponent* tc = registry.GetComponent<TransformComponent>(e);
		if (tc->scale.x < 0) tc->scale.x *= -1;
	}
	void OnExit(Registry&, EntityRegistry::Entity) {}
};

class FlipStateMachine : public IStateMachine {
public:
	FlipStateMachine() {
		StateList["right"] = std::make_shared<RightState>();
		StateList["left"] = std::make_shared<LeftState>();

		StartState = StateList["right"];
		CurrState = StateList["right"];
		CurrStateName = "right";
	}
};