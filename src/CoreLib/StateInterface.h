/**___________________________________________________________________________/
@file          StateInterface.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Interface for the statemachine.
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include <vector>
#include <memory>
#include <map>

class IStateMachine;
class IState;
class ITransition;

class IState {
public:
    virtual ~IState() = default;
    // Called when it is "swapped" to this state
    virtual void OnStart(Registry& registry, EntityRegistry::Entity e) = 0;
    // Called every frame
    virtual void OnUpdate(Registry& registry, EntityRegistry::Entity e, float dt) = 0;
    // Called when a state change happens.
    virtual void OnExit(Registry& registry, EntityRegistry::Entity e) = 0;
    // Get the list of possible transitions.
    virtual std::vector<std::shared_ptr<ITransition>>& GetTransitions() final { return transitions; }

protected:
    std::vector<std::shared_ptr<ITransition>> transitions;
};

class ITransition {
public:
    virtual ~ITransition() = default;
    // Boolean function to determine if we should transition to the next state
    virtual bool Compare(Registry& registry, EntityRegistry::Entity e) = 0;
    virtual std::string getNextState() = 0;
};

class IStateMachine {
public:
    virtual ~IStateMachine() = default;

    // Initializer for the statemachine
    virtual void OnStart(Registry& registry, EntityRegistry::Entity e) final {

        if (!StartState) {
            LOGE("STARTSTATE IS NOT INITIALIZED!!!\n");
            return;
        }

        if (CurrStateName.empty()) {
            LOGE("Please initialize variable CurrStateName with the name of the starting state!!!\n");
        }

        StartState->OnStart(registry, e);
        CurrState = StartState;
    }


    virtual void Update(Registry& registry, EntityRegistry::Entity e, float dt) final {
        if (!CurrState) {
            LOGE("CURRSTATE IS NOT INITIALIZED!!!\n");
            return;
        }

        // Runs the update for the current active state
        CurrState->OnUpdate(registry, e, dt);

        // A state may have multiple valid transitions, i.e. a going left state can transition to up, down and right.
        // So I grab the vector within the state containing all possible transitions, then I iterate through their 
        // Compare function. I will use the first transition that returns true from the compare, if there are multiple
        // valid transitions active at once.
        // When a compare function returns true, I run the exit on the current state, get the next state, and run the
        // on start on the new state.
        std::vector<std::shared_ptr<ITransition>> tranVec = CurrState->GetTransitions();
        for (std::shared_ptr<ITransition> transition : tranVec) {
            if (transition->Compare(registry, e)) {
                CurrState->OnExit(registry, e);
                std::shared_ptr<IState> NextState = StateList[transition->getNextState()];
                CurrState = NextState;
                CurrState->OnStart(registry, e);
                break;
            }
        }
    }

    virtual std::string GetCurrentState() final { return CurrStateName; }
protected:
    std::map<std::string, std::shared_ptr<IState>> StateList;
    std::shared_ptr<IState> StartState;
    std::shared_ptr<IState> CurrState;
    std::string CurrStateName;
};
