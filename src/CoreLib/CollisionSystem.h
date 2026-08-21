/*!
@file       CollisionSystem.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file declares the [CollisionSystem] class, which handles detection
and resolution of collisions between game entities. It provides utilities
for shape-based collision checks (AABB and Circle) and response handling
for static and dynamic entities.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/


#pragma once
#include "Registry.h"
#include "Components.h"
#include "BroadPhaseGrid.h" 
#include <unordered_map>
#include <unordered_set>

/*!
* \brief
*    The [CollisionSystem] class is responsible for detecting and resolving
*    collisions between entities in the game world. It supports various
*    collision shapes such as axis-aligned bounding boxes (AABBs) and circles,
*    and provides utility functions for calculating collision-related properties
*    like scaled extents, world-space positions, and effective radii.
*
* \brief
*    Usage:
* \brief
*    - Retrieve the singleton instance using `CollisionSystem::Instance()`.
* \brief
*    - Call `Update(registry)` every frame to automatically detect and resolve
*      collisions between entities registered with collision components.
* \brief
*    - Use `CheckAABB()`, `CheckCircleCircle()`, or `CheckCircleAABB()` to
*      perform shape-specific collision checks.
* \brief
*    - Utilize helper functions like `GetWorldCenter()` and
*      `GetFinalHalfExtents()` to compute collision-related metrics for custom
*      logic or debugging.
*
* \return
*    [CollisionSystem&] Reference to the singleton instance for managing
*    collision detection and resolution globally.
*/
class CollisionSystem {
public:

    /*!
    * \brief
    *    Initializes the singleton instance of the [CollisionSystem] class.
    */
    void Init();

    /*!
    * \brief
    *    Frees resources used by the singleton instance of the [CollisionSystem] class.
    */
    void Free();

    /*!
    * \brief
    *    Retrieves the singleton instance of the [CollisionSystem] class.
    *
    * \param
    *    [None]
    *
    * \return
    *    [CollisionSystem&] Reference to the singleton instance.
    */
    static CollisionSystem& Instance();

    // ===============================
    // Core Collision Update
    // ===============================    

    /*!
    * \brief
    *    Updates the collision system by detecting and resolving collisions
    *    between entities in the registry.
    *
    * \param
    *    [Registry&] registry - The registry containing entities and their components.
    *
    * \return
    *    [void]
    */
    void Update(Registry& registry);

    // ===============================
    // Collision Detection
    // ===============================

    /*!
    * \brief
    *    Checks for a collision between two axis-aligned bounding boxes (AABBs).
    *
    * \param
    *    [const CollisionComponent&] c1 - Collision data for the first entity.
    * \param
    *    [const TransformComponent&] t1 - Transform data for the first entity.
    * \param
    *    [const CollisionComponent&] c2 - Collision data for the second entity.
    * \param
    *    [const TransformComponent&] t2 - Transform data for the second entity.
    *
    * \return
    *    [bool] True if the two boxes are colliding, otherwise false.
    */
    bool CheckAABB(const CollisionComponent& c1, const TransformComponent& t1,
                   const CollisionComponent& c2, const TransformComponent& t2);

    /*!
    * \brief
    *    Checks for a collision between two circles.
    *
    * \param
    *    [const CollisionComponent&] c1 - Collision data for the first circle.
    * \param
    *    [const TransformComponent&] t1 - Transform data for the first circle.
    * \param
    *    [const CollisionComponent&] c2 - Collision data for the second circle.
    * \param
    *    [const TransformComponent&] t2 - Transform data for the second circle.
    *
    * \return
    *    [bool] True if the circles are colliding, otherwise false.
    */
    bool CheckCircleCircle(const CollisionComponent& c1, const TransformComponent& t1,
                           const CollisionComponent& c2, const TransformComponent& t2);

    /*!
    * \brief
    *    Checks for a collision between a circle and an axis-aligned bounding box.
    *
    * \param
    *    [const CollisionComponent&] circle - Collision data for the circle.
    * \param
    *    [const TransformComponent&] circleTransform - Transform data for the circle.
    * \param
    *    [const CollisionComponent&] box - Collision data for the box.
    * \param
    *    [const TransformComponent&] boxTransform - Transform data for the box.
    *
    * \return
    *    [bool] True if the circle and box are colliding, otherwise false.
    */
    bool CheckCircleAABB(const CollisionComponent& circle, const TransformComponent& circleTransform,
                         const CollisionComponent& box, const TransformComponent& boxTransform);

    /*!
    * \brief
    *    Checks for a collision between a capsule and a circle.
    *
    * \param
    *    [const CollisionComponent&] capsule - Collision data for the capsule.
    * \param
    *    [const TransformComponent&] capsuleTransform - Transform data for the capsule.
    * \param
    *    [const CollisionComponent&] circle - Collision data for the circle.
    * \param
    *    [const TransformComponent&] circleTransform - Transform data for the circle.
    *
    * \return
    *    [bool] True if the capsule and circle are colliding, otherwise false.
    */
    bool CheckCapsuleCircle(const CollisionComponent& capsule, const TransformComponent& capsuleTransform,
        const CollisionComponent& circle, const TransformComponent& circleTransform);
    
    /*!
    * \brief
    *    Checks for a collision between a capsule and an axis-aligned bounding box.
    *
    * \param
    *    [const CollisionComponent&] capsule - Collision data for the capsule.
    * \param
    *    [const TransformComponent&] capsuleTransform - Transform data for the capsule.
    * \param
    *    [const CollisionComponent&] box - Collision data for the box.
    * \param
    *    [const TransformComponent&] boxTransform - Transform data for the box.
    *
    * \return
    *    [bool] True if the capsule and box are colliding, otherwise false.
    */
    bool CheckCapsuleAABB(const CollisionComponent& capsule, const TransformComponent& capsuleTransform,
        const CollisionComponent& box, const TransformComponent& boxTransform);
    
    /*!
    * \brief
    *    Checks for a collision between two capsules.
    *
    * \param
    *    [const CollisionComponent&] c1 - Collision data for the first capsule.
    * \param
    *    [const TransformComponent&] t1 - Transform data for the first capsule.
    * \param
    *    [const CollisionComponent&] c2 - Collision data for the second capsule.
    * \param
    *    [const TransformComponent&] t2 - Transform data for the second capsule.
    *
    * \return
    *    [bool] True if the two capsules are colliding, otherwise false.
    */
    bool CheckCapsuleCapsule(const CollisionComponent& c1, const TransformComponent& t1,
        const CollisionComponent& c2, const TransformComponent& t2);

    // ===============================
    // Helper Utilities
    // ===============================

    /*!
    * \brief
    *    Calculates the final scaled half-extents of a box collider, factoring in
    *    both the transform scale and the collider's scale.
    *
    * \param
    *    [const CollisionComponent&] c - The box's collision data.
    * \param
    *    [const TransformComponent&] t - The box's transform data.
    *
    * \return
    *    [Vec2] The computed half-extents of the box in world space.
    */
    static Vec2 GetFinalHalfExtents(const CollisionComponent& c, const TransformComponent& t);

    /*!
    * \brief
    *    Calculates the world-space center of an entity based on its transform
    *    and collision offset.
    *
    * \param
    *    [const TransformComponent&] t - The entity's transform data.
    * \param
    *    [const CollisionComponent&] c - The entity's collision data.
    *
    * \return
    *    [Vec2] The world-space position of the collider's center.
    */
    static Vec2 GetWorldCenter(const TransformComponent& t, const CollisionComponent& c);

    /*!
    * \brief
    *    Calculates the effective radius of a circle collider, factoring in both
    *    transform scaling and collider scaling.
    *
    * \param
    *    [const CollisionComponent&] c - The circle's collision data.
    * \param
    *    [const TransformComponent&] t - The circle's transform data.
    *
    * \return
    *    [float] The scaled radius of the circle collider.
    */
    static float GetEffectiveRadius(const CollisionComponent& c, const TransformComponent& t); 

    /*!
    * \brief
    *    Enables or disables debug printing for broad-phase collision checks.
    *
    * \param
    *    [bool] enabled - True to enable debug printing, false to disable it.
    *
    * \return
    *    [void]
    */
    inline void SetBroadPhaseDebugPrint(bool enabled) { debugPrintEnabled = enabled; }

    /*!
    * \brief
    *    Checks whether broad-phase debug printing is currently enabled.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if debug printing is enabled, false otherwise.
    */
    inline bool IsBroadPhaseDebugPrintEnabled() const { return debugPrintEnabled; }

    /*!
    * \brief
    *    Automatically fits an entity's collider to the opaque (non-transparent)
    *    pixel region of its assigned sprite texture, updating the collision
    *    bounds to tightly match the visible sprite area.
    *
    * \param
    *    [Registry&] registry - The ECS registry used to access the entity's
    *    sprite and collision components.
    *
    * \param
    *    [Registry::Entity] entity - The entity whose collider should be
    *    recalculated based on its sprite's opaque pixel data.
    *
    * \return
    *    [bool] True if the collider was successfully updated, false if fitting
    *    failed due to missing texture data or required components.
    */
    bool AutoFitToSpriteOpaque(Registry& registry, Registry::Entity entity);

private:
    // ===============================
    // Collision Responses
    // ===============================

    /*!
    * \brief
    *    Resolves a collision between a dynamic entity and a static entity by
    *    correcting positions and removing velocity along the collision normal.
    *
    * \param
    *    [TransformComponent&] dynT - Transform of the dynamic entity.
    * \param
    *    [PhysicsComponent&] dynP - Physics component of the dynamic entity.
    * \param
    *    [const CollisionComponent&] dynC - Collision data of the dynamic entity.
    * \param
    *    [const TransformComponent&] staT - Transform of the static entity.
    * \param
    *    [const CollisionComponent&] staC - Collision data of the static entity.
    *
    * \return
    *    [void]
    */
    void ResolveStaticDynamic(TransformComponent& dynT, const TransformComponent& dynWorldT, PhysicsComponent& dynP, const CollisionComponent& dynC,
        const TransformComponent& staWorldT, const CollisionComponent& staC,
        const TransformComponent* dynParentTransform);


    /*!
    * \brief
    *    Resolves a collision between two dynamic entities by reflecting their
    *    velocities and adjusting positions to remove overlap.
    *
    * \param
    *    [TransformComponent&] t1 - Transform of the first dynamic entity.
    * \param
    *    [PhysicsComponent&] p1 - Physics component of the first dynamic entity.
    * \param
    *    [const CollisionComponent&] c1 - Collision data of the first dynamic entity.
    * \param
    *    [TransformComponent&] t2 - Transform of the second dynamic entity.
    * \param
    *    [PhysicsComponent&] p2 - Physics component of the second dynamic entity.
    * \param
    *    [const CollisionComponent&] c2 - Collision data of the second dynamic entity.
    *
    * \return
    *    [void]
    */
    void ResolveDynamicDynamic(TransformComponent& t1, const TransformComponent& worldT1, PhysicsComponent& p1, const CollisionComponent& c1,
        TransformComponent& t2, const TransformComponent& worldT2, PhysicsComponent& p2, const CollisionComponent& c2,
        const TransformComponent* parentTransform1, const TransformComponent* parentTransform2);


    /*!
    * \brief
    *    Broad-phase grid used to optimize collision detection by dividing
    *    the world space into spatial partitions.
    *
    * \param
    *    [None]
    *
    * \return
    *    [BroadPhaseGrid] Broad-phase spatial partitioning system.
    */
    BroadPhaseGrid broadPhase;

    /*!
    * \brief
    *    Flag that determines whether to print broad-phase debug information
    *    during collision checks.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if debug printing is enabled, false otherwise.
    */
    bool debugPrintEnabled = false;

    /*!
    * \brief
    *    Stores the event listener identifier used to register and manage
    *    collision-related callbacks or notifications within the system.
    */
    size_t eventListenerID{};

    using PairKey = uint64_t;

    /*!
    * \brief
    *    Generates a unique, order-independent key representing a pair of entities.
    *    This key is used to efficiently track collision or trigger relationships
    *    between two entities across frames.
    *
    * \param
    *    [Registry::Entity] a - The first entity in the pair.
    * \param
    *    [Registry::Entity] b - The second entity in the pair.
    *
    * \return
    *    [PairKey] A 64-bit key uniquely identifying the entity pair.
    */
    [[nodiscard]] PairKey MakePairKey(Registry::Entity a, Registry::Entity b) const;

    /*!
    * \brief
    *    Decodes a previously generated pair key back into the original entity IDs.
    *    This allows collision or trigger pairs to be inspected or processed
    *    after being stored in hashed containers.
    *
    * \param
    *    [PairKey] key - The encoded key representing an entity pair.
    *
    * \return
    *    [std::pair<Registry::Entity, Registry::Entity>] The decoded entity pair.
    */
    [[nodiscard]] std::pair<Registry::Entity, Registry::Entity> DecodePairKey(PairKey key) const;

    /*!
    * \brief
    *    Stores all collision pairs detected during the current update frame.
    *    Used to determine collision enter, stay, and exit events.
    */
    std::unordered_set<PairKey> currentCollisionPairs;

    /*!
    * \brief
    *    Stores collision pairs detected during the previous update frame.
    *    Compared against the current collision set to identify state changes.
    */
    std::unordered_set<PairKey> previousCollisionPairs;

    /*!
    * \brief
    *    Stores all trigger overlap pairs detected during the current update frame.
    *    Trigger pairs do not resolve physically but may generate gameplay events.
    */
    std::unordered_set<PairKey> currentTriggerPairs;

    /*!
    * \brief
    *    Stores trigger overlap pairs from the previous update frame.
    *    Used to detect trigger enter and exit events.
    */
    std::unordered_set<PairKey> previousTriggerPairs;
};
