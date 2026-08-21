/*!
@file       PhysicsSystem.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file implements the [PhysicsSystem] class, which applies velocity
and acceleration updates to entities each frame and manages global
simulation controls such as pausing, stepping, and debug visualization.
It also processes external forces and integrates motion in real time.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "PhysicsSystem.h"
#include "UIManager.h"
#include "UpdateStackManager.h"

static const float maxSpeed = 20000.f; // adjust as needed

static PhysicsSystem instance; // Singleton instance
PhysicsSystem& PhysicsSystem::Instance() { return instance; }


// ===============================
// Helper Utilities
// ===============================

// Trigger exactly one physics step when paused
// single step-does not work when unpaused
void PhysicsSystem::TriggerSingleStep() {
    singleStepTriggered = true;
}

// Applies given force to an entity
void PhysicsSystem::ApplyForce(EntityRegistry::Entity entity, const Vec2& force, Registry& registry) {

    if (auto* physicsComp = registry.GetComponent<PhysicsComponent>(entity)) {
        physicsComp->netForce += force;
    }
}

namespace{
    Vec2 TransformWorldDeltaToLocal(const TransformComponent * parentTransform, const Vec2 & worldDelta) {
        if (!parentTransform) {
            return worldDelta;
        }

        Mat3 parentNoTranslation = parentTransform->transform;
        parentNoTranslation.m[6] = 0.f;
        parentNoTranslation.m[7] = 0.f;

        Mat3 invParent = parentNoTranslation.Inversed();
        return invParent.TransformPoint(worldDelta);
    }
}



// ===============================
// Core Physics Update
// ===============================   

// Update all physics-enabled entities
void PhysicsSystem::Update(Registry& registry, float dt) {

    auto entities = registry.GetEntitiesWithComponents<TransformComponent, PhysicsComponent>();

    size_t dynCount = 0, kinCount = 0;

    UpdateStackManager* usm = CEO::Get<UpdateStackManager>();

    for (auto entity : entities) {
        if (usm->ShouldNotUpdate(entity)) continue;
        if (!usm->IsActive(entity)) continue;

        auto* transform = registry.GetComponent<TransformComponent>(entity);
        auto* physics = registry.GetComponent<PhysicsComponent>(entity);

        if (!transform || !physics) continue;

        // Skip kinematic entities
        if (physics->isKinematic){
            ++kinCount;
            continue; 
        };
        ++dynCount;

        // a = F / m 
        // Guard against invalid masses by treating <=0 as immovable
        if (physics->mass > 0.f){
            physics->acceleration = physics->netForce / physics->mass;
        }
        else{
            physics->acceleration = { 0.f, 0.f };
            physics->velocity = { 0.f, 0.f };
        }

        // Apply acceleration to velocity
        physics->velocity += physics->acceleration * dt;

        // Clamp velocity magnitude
        float speedSq = physics->velocity.LengthSquared();
        if (speedSq > maxSpeed * maxSpeed) {
            float scale = maxSpeed / std::sqrt(speedSq);
            physics->velocity *= scale;
        }

        // Apply velocity to position (account for parent transforms if present)
        auto* hierarchyComp = registry.GetComponent<HierarchyComponnent>(entity);
        TransformComponent* parentTransform = nullptr;
        if (hierarchyComp && hierarchyComp->parent != 0) {
            parentTransform = registry.GetComponent<TransformComponent>(hierarchyComp->parent);
        }
        transform->translate += TransformWorldDeltaToLocal(parentTransform, physics->velocity * dt);
    }

    // Reset single-step flag so it only triggers once
    if (singleStepTriggered){
        singleStepTriggered = false;
    }

    // --- DEBUG VISUALIZER START ---
    if (debugPrintEnabled){
        std::cout << "[Physics] dt=" << dt
            << " dyn=" << dynCount
            << " kin=" << kinCount
            << ")\n";
    }
    // --- DEBUG VISUALIZER END ---
}

