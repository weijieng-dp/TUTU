/**___________________________________________________________________________/
@file          EnemyStates.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Enemy states header file. 
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include <memory>
#include <string>
#include "PlayerControllerScript.h"

// ------------------------------------------------- //
//                    EYE STATES                     //
// ------------------------------------------------- //

class EyeStateMachine : public IStateMachine {
public: 
	EyeStateMachine();
	~EyeStateMachine() = default;
};

class EyeChaseState : public IState {
public:
	EyeChaseState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class EyeAttackState : public IState {
public:
	EyeAttackState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
	bool attackDone;
};

class EyeIdleState : public IState {
public:
	EyeIdleState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

// ------------------------------------------------- //
//                   LARVA STATES                    //
// ------------------------------------------------- //

class LarvaStateMachine : public IStateMachine {
public:
	LarvaStateMachine();
	~LarvaStateMachine() = default;
};

class LarvaChaseState : public IState {
public:
	LarvaChaseState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class LarvaAttackState : public IState {
public:
	LarvaAttackState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class LarvaIdleState : public IState {
public:
	LarvaIdleState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

// ------------------------------------------------- //
//                    GIRL STATES                    //
// ------------------------------------------------- //

class GirlStateMachine : public IStateMachine {
public:
	GirlStateMachine();
	~GirlStateMachine() = default;
};

class GirlIdleState : public IState {
public:
	GirlIdleState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class GirlAttackState : public IState {
public:
	GirlAttackState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

// ------------------------------------------------- //
//                    ONI STATES                     //
// ------------------------------------------------- //

class OniStateMachine : public IStateMachine {
public:
	OniStateMachine();
	~OniStateMachine() = default;
};

class OniChaseState : public IState {
public:
	OniChaseState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class OniAttackState : public IState {
public:
	OniAttackState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
	bool attackDone;
	EntityRegistry::Entity attackId;
};

// ------------------------------------------------- //
//                    GUY STATES                     //
// ------------------------------------------------- //

class GuyStateMachine : public IStateMachine {
public:
	GuyStateMachine();
	~GuyStateMachine() = default;
};

class GuyIdleState : public IState {
public:
	GuyIdleState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

class GuyAttackState : public IState {
public:
	GuyAttackState();
	void OnStart(Registry& registry, EntityRegistry::Entity e) override;
	void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) override;
	void OnExit(Registry& registry, EntityRegistry::Entity e) override;
};

// ------------------------------------------------- //
//                    TRANSITIONS                    //
// ------------------------------------------------- //

class ChaseIdleTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

class ChaseAttackTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

class AttackIdleTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

class AttackChaseTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

class IdleAttackTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

class IdleChaseTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

// Special transition for larva
// Larva will always just attack off cooldown
class ChaseAttackLTransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};

// Special transition for eye
class IdleAttackETransition : public ITransition {
public:
	bool Compare(Registry& registry, EntityRegistry::Entity e) override;
	std::string getNextState() override;
};