/*!
@file       CollisionSystem.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file implements the [CollisionSystem] class, which manages broad-phase
and narrow-phase collision detection and applies appropriate responses to
overlapping entities. It handles both dynamic-dynamic and dynamic-static
collision resolution using shape-specific intersection logic.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "CollisionSystem.h"
#include "UpdateStackManager.h"

// Global singleton instance (defined once and accessed via Instance())
static CollisionSystem instance;

// Restitution coefficient: controls bounce/elasticity of collisions
// e = 1.0 = perfectly elastic (100% energy preserved, maximum bounce)
// e = 0.0 = perfectly inelastic (no bounce, stick together)
// Values 0-1 represent partially elastic collisions
constexpr float RESTITUTION_COEFF = 1.f;

// PENETRATION_CORRECTION: Portion of penetration depth to resolve each frame
// This prevents "sinking" through surfaces; typical values are 0.2-0.8
// Lower values = gradual separation (smooth but slower); higher values = aggressive correction
constexpr float PENETRATION_CORRECTION = 1.0f;

// Tangential friction coefficient (disabled in this implementation)
// When enabled, this would reduce sliding velocity along collision surfaces
// Typical range: 0.0 (no friction) to 1.0 (high friction)
constexpr float FRICTION_COEFF = 0.f;

namespace {

    // Lightweight world-space representation of a vertical capsule.
    // a/b are the segment endpoints of the capsule spine, center is collider center,
    // and radius is the cap radius after all scales are applied.
    struct CapsuleGeom {
        Vec2 a;
        Vec2 b;
        Vec2 center;
        float radius;
    };


    // Combines a local transform with parent transform hierarchy to compute world space position/rotation/scale
    // This is essential for entities that have parent-child relationships in the scene tree
    TransformComponent BuildWorldTransform(const TransformComponent& local, const TransformComponent* parentTransform) {
        TransformComponent world = local;

        // Build local transformation matrix: T * R * S order
        // Translation applied last so it's in global coordinates (not affected by rotation/scale)
        Mat3 localMat = Mat3::Translation(local.translate.x, local.translate.y)
            * Mat3::Rotation(local.rotation)
            * Mat3::Scale(local.scale.x, local.scale.y);

        // If entity has a parent, multiply by parent's world transform
        // Otherwise, local transform = world transform
        Mat3 worldMat = parentTransform ? parentTransform->transform * localMat : localMat;

        // Extract components from combined transformation matrix
        // worldMat.m[6,7] contains the translation (homogeneous coordinates)
        world.translate = Vec2(worldMat.m[6], worldMat.m[7]);

        // Extract scale by computing length of transform basis vectors
        // sqrt(m[0]^2 + m[1]^2) = x-axis scale; sqrt(m[3]^2 + m[4]^2) = y-axis scale
        world.scale = Vec2(
            std::sqrt(worldMat.m[0] * worldMat.m[0] + worldMat.m[1] * worldMat.m[1]),
            std::sqrt(worldMat.m[3] * worldMat.m[3] + worldMat.m[4] * worldMat.m[4])
        );

        // Extract rotation from transformation matrix using atan2
        // atan2(m[3], m[0]) = angle of first basis vector from x-axis
        world.rotation = atan2f(worldMat.m[3], worldMat.m[0]);
        world.transform = worldMat;

        return world;
    }

    // Converts a delta vector from world space to local space
    // This is used when we want to apply world-space corrections to local transforms
    Vec2 TransformWorldDeltaToLocal(const TransformComponent* parentTransform, const Vec2& worldDelta) {
        if (!parentTransform) {
            // No parent = local space = world space
            return worldDelta;
        }

        // Create rotation-only matrix (remove translation component)
        Mat3 parentNoTranslation = parentTransform->transform;
        parentNoTranslation.m[6] = 0.f;  // Clear translation x
        parentNoTranslation.m[7] = 0.f;  // Clear translation y

        // Invert the rotation matrix to convert world delta to local delta
        // Note: for rotation matrices, inverse = transpose, so this efficiently reverses the transform
        Mat3 invParent = parentNoTranslation.Inversed();
        return invParent.TransformPoint(worldDelta);
    }

    // Applies a world-space position correction to a local transform
    // When entities have parents, collision resolution happens in world space
    // but we need to store the correction in local space for the transform component
    void ApplyWorldTranslationDelta(TransformComponent& localTransform,
        const TransformComponent* parentTransform,
        const Vec2& worldDelta) {
        // Convert the world-space collision correction to local space, then apply to local position
        localTransform.translate += TransformWorldDeltaToLocal(parentTransform, worldDelta);
        const Mat3 localMat = Mat3::Translation(localTransform.translate.x, localTransform.translate.y)
            * Mat3::Rotation(localTransform.rotation)
            * Mat3::Scale(localTransform.scale.x, localTransform.scale.y);
        localTransform.transform = parentTransform ? (parentTransform->transform * localMat) : localMat;
    }

    // Determines which entity should receive collision response.
    // Colliders may exist on child entities, but physics (velocity, movement)
    // is usually stored on a parent.
    //
    // So when a collision occurs:
    //   child collider -> does NOT move itself
    //   instead -> we walk up hierarchy to find a PhysicsComponent
    //
    // Example:
    //   Player (Physics)
    //     |_ Weapon (Collider)
    //
    // Weapon hits -> Player moves, NOT Weapon.
    // This ensures one rigid body per hierarchy,
    // so no desync between child & parent transforms
    Registry::Entity FindPhysicsOwner(Registry& registry, Registry::Entity entity) {
        Registry::Entity current = entity;

        while (current != 0) {
            if (registry.GetComponent<PhysicsComponent>(current) != nullptr) {
                return current;
            }

            HierarchyComponnent* hierarchy = registry.GetComponent<HierarchyComponnent>(current);
            if (!hierarchy || hierarchy->parent == 0) {
                break;
            }

            current = hierarchy->parent;
        }

        return entity;
    }

    // Builds world-space capsule geometry from CollisionComponent + TransformComponent.
    // Capsule is treated as axis-aligned vertical:
    // - half extents come from GetFinalHalfExtents (includes transform and collider scale)
    // - radius is min(half.x, half.y)
    // - spine half-length is max(0, half.y - radius)
    // This allows degenerate capsules (segment length = 0) to behave as circles.
    CapsuleGeom GetCapsuleGeom(const CollisionComponent& capsule, const TransformComponent& t) {
        const Vec2 half = CollisionSystem::GetFinalHalfExtents(capsule, t);
        CapsuleGeom g{};
        g.center = CollisionSystem::GetWorldCenter(t, capsule);
        g.radius = std::min(half.x, half.y);
        const float halfSeg = std::max(0.f, half.y - g.radius);
        g.a = Vec2{ g.center.x, g.center.y - halfSeg };
        g.b = Vec2{ g.center.x, g.center.y + halfSeg };
        return g;
    }

    // Returns the closest point on segment AB to point P using clamped projection.
    // Handles degenerate segments by returning A.
    Vec2 ClosestPointOnSegment(const Vec2& p, const Vec2& a, const Vec2& b) {
        const Vec2 ab = b - a;
        const float len2 = ab.LengthSquared();
        if (len2 <= 1e-8f) return a;
        const float t = std::clamp(Vec2DotProduct(p - a, ab) / len2, 0.f, 1.f);
        return a + ab * t;
    }

    // Returns the closest point on an axis-aligned box [min,max] to point P
    // by clamping each coordinate independently.
    Vec2 ClosestPointOnAABB(const Vec2& p, const Vec2& min, const Vec2& max) {
        return Vec2{
            std::clamp(p.x, min.x, max.x),
            std::clamp(p.y, min.y, max.y)
        };
    }

    // Approximates the closest point on segment AB to an AABB by alternating
    // projections between segment and box for a small fixed number of iterations.
    // This is used by capsule-vs-box checks where capsule spine is a segment.
    Vec2 ClosestPointSegmentToAABB(const Vec2& a, const Vec2& b, const Vec2& boxMin, const Vec2& boxMax) {
        // Small iterative projection between segment and box.
        Vec2 qSeg = ClosestPointOnSegment((boxMin + boxMax) * 0.5f, a, b);
        Vec2 qBox = ClosestPointOnAABB(qSeg, boxMin, boxMax);
        for (int i = 0; i < 2; ++i) {
            qSeg = ClosestPointOnSegment(qBox, a, b);
            qBox = ClosestPointOnAABB(qSeg, boxMin, boxMax);
        }
        return qSeg;
    }

    // Computes closest points c1/c2 between two finite segments [p1,q1] and [p2,q2].
    // Robustly handles parallel and degenerate (point-like) segments.
    // Used as the core distance primitive for capsule-capsule collision.
    void ClosestPointsSegmentSegment(const Vec2& p1, const Vec2& q1, const Vec2& p2, const Vec2& q2,
        Vec2& c1, Vec2& c2) {
        const Vec2 d1 = q1 - p1;
        const Vec2 d2 = q2 - p2;
        const Vec2 r = p1 - p2;
        const float a = d1.LengthSquared();
        const float e = d2.LengthSquared();
        const float f = Vec2DotProduct(d2, r);
        float s = 0.f;
        float t = 0.f;

        if (a <= 1e-8f && e <= 1e-8f) {
            c1 = p1;
            c2 = p2;
            return;
        }
        if (a <= 1e-8f) {
            s = 0.f;
            t = std::clamp(f / e, 0.f, 1.f);
        }
        else {
            const float c = Vec2DotProduct(d1, r);
            if (e <= 1e-8f) {
                t = 0.f;
                s = std::clamp(-c / a, 0.f, 1.f);
            }
            else {
                const float b = Vec2DotProduct(d1, d2);
                const float denom = a * e - b * b;
                if (std::fabs(denom) > 1e-8f) {
                    s = std::clamp((b * f - c * e) / denom, 0.f, 1.f);
                }
                else {
                    s = 0.f;
                }
                const float tNom = b * s + f;
                if (tNom <= 0.f) {
                    t = 0.f;
                    s = std::clamp(-c / a, 0.f, 1.f);
                }
                else if (tNom >= e) {
                    t = 1.f;
                    s = std::clamp((b - c) / a, 0.f, 1.f);
                }
                else {
                    t = tNom / e;
                }
            }
        }
        c1 = p1 + d1 * s;
        c2 = p2 + d2 * t;
    }
}

///////////////////////////////////////////////////////////////////////////////
// PAIR KEY HELPERS
// These encode/decode entity pairs into unique 64-bit keys for collision tracking
///////////////////////////////////////////////////////////////////////////////

// Combines two entity IDs into a single unique key
// Uses min/max ordering to ensure (A,B) and (B,A) produce the same key
// Bit layout: [first (32 bits) | second (32 bits)]
CollisionSystem::PairKey CollisionSystem::MakePairKey(Registry::Entity a, Registry::Entity b) const {
    const auto first = std::min(a, b);   // Smaller ID goes in upper bits
    const auto second = std::max(a, b);  // Larger ID goes in lower bits
    return (static_cast<PairKey>(first) << 32) | static_cast<PairKey>(second);
}

// Extracts two entity IDs from a pair key
// Reverses the encoding done by MakePairKey()
std::pair<Registry::Entity, Registry::Entity> CollisionSystem::DecodePairKey(PairKey key) const {
    const Registry::Entity first = static_cast<Registry::Entity>(key >> 32);      // Extract upper 32 bits
    const Registry::Entity second = static_cast<Registry::Entity>(key & 0xFFFFFFFF); // Extract lower 32 bits
    return { first, second };
}

///////////////////////////////////////////////////////////////////////////////
// INITIALIZATION AND LIFECYCLE
///////////////////////////////////////////////////////////////////////////////

// Initializes the collision system and subscribes to scene change events
void CollisionSystem::Init() {
    // When scene changes, clear all collision data to avoid stale references
    eventListenerID = CEO::Get<EventsDispatcher>()->Subscribe<Events::SceneChanged>([this](const Events::SceneChanged&) {
        broadPhase.Clear();              // Clear spatial partitioning structure
        currentCollisionPairs.clear();   // Clear active collisions this frame
        previousCollisionPairs.clear();  // Clear collisions from previous frame
        currentTriggerPairs.clear();     // Clear active trigger overlaps
        previousTriggerPairs.clear();    // Clear trigger overlaps from previous frame
        });
}

// Cleans up event subscriptions
void CollisionSystem::Free() {
    CEO::Get<EventsDispatcher>()->Unsubscribe<Events::SceneChanged>(eventListenerID);
}

// Returns the global singleton instance
CollisionSystem& CollisionSystem::Instance() { return instance; }

///////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS - Shape Measurement
///////////////////////////////////////////////////////////////////////////////

// Computes final half-extents (distance from center to edge) for an AABB
// Takes into account: base collider size, entity scale, and custom collider scale
Vec2 CollisionSystem::GetFinalHalfExtents(const CollisionComponent& c, const TransformComponent& t) {
    return {
        c.baseHalfExtents.x * std::fabs(t.scale.x) * c.colliderScale.x,
        c.baseHalfExtents.y * std::fabs(t.scale.y) * c.colliderScale.y
    };
}

// Computes effective radius for a circle collider
// Uses maximum of x/y scales (takes the larger scale to ensure full coverage)
// This prevents the circle from "slipping through" on the scaled side
float CollisionSystem::GetEffectiveRadius(const CollisionComponent& c, const TransformComponent& t) {
    const float scaledX = std::fabs(t.scale.x) * c.colliderScale.x;
    const float scaledY = std::fabs(t.scale.y) * c.colliderScale.y;
    const float finalScale = (scaledX > scaledY) ? scaledX : scaledY;  // Use maximum scale
    return c.radius * finalScale;
}

// Computes world-space center of a collider
// Applies scaling to the offset (offset moves more with larger scales)
Vec2 CollisionSystem::GetWorldCenter(const TransformComponent& t, const CollisionComponent& c) {
    const Vec2 scaledOffset{
        c.offset.x * std::fabs(t.scale.x) * c.colliderScale.x,
        c.offset.y * std::fabs(t.scale.y) * c.colliderScale.y
    };
    return { t.translate.x + scaledOffset.x, t.translate.y + scaledOffset.y };
}

///////////////////////////////////////////////////////////////////////////////
// MTV (MINIMUM TRANSLATION VECTOR) COMPUTATION
// These functions calculate the normal and depth needed to separate colliding shapes
///////////////////////////////////////////////////////////////////////////////

// Computes MTV for two axis-aligned bounding boxes
// Uses separating axis theorem (SAT) on axis-aligned planes
static void ComputeMTV_BoxBox(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2,
    Vec2& normal, float& penetration) {

    const Vec2 h1 = CollisionSystem::GetFinalHalfExtents(c1, t1);
    const Vec2 h2 = CollisionSystem::GetFinalHalfExtents(c2, t2);
    const Vec2 p1 = CollisionSystem::GetWorldCenter(t1, c1);
    const Vec2 p2 = CollisionSystem::GetWorldCenter(t2, c2);

    // Calculate the vector from box1 center to box2 center
    const Vec2 distanceVectorP1P2 = { p2.x - p1.x, p2.y - p1.y };

    // MTV on X-axis: sum of half-widths minus the actual distance between centers
    // If positive, boxes overlap on X-axis
    const float penetrationX = (h1.x + h2.x) - std::fabs(distanceVectorP1P2.x);

    // MTV on Y-axis: sum of half-heights minus the actual distance between centers
    // If positive, boxes overlap on Y-axis
    const float penetrationY = (h1.y + h2.y) - std::fabs(distanceVectorP1P2.y);

    // Choose the axis with SMALLEST penetration (easiest to separate along)
    // This ensures we push objects apart along the path of least resistance
    if (penetrationX < penetrationY) {
        // Separate along X-axis; direction depends on which box is to the left/right
        normal = { (distanceVectorP1P2.x < 0.f) ? -1.f : 1.f, 0.f };
        penetration = penetrationX;
    }
    else {
        // Separate along Y-axis; direction depends on which box is above/below
        normal = { 0.f, (distanceVectorP1P2.y < 0.f) ? -1.f : 1.f };
        penetration = penetrationY;
    }
}

// Computes MTV for two circles
// Circles overlap if distance between centers < sum of radii
static void ComputeMTV_CircleCircle(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2,
    Vec2& normal, float& penetration) {

    const Vec2 circleCtr1 = CollisionSystem::GetWorldCenter(t1, c1);
    const Vec2 circleCtr2 = CollisionSystem::GetWorldCenter(t2, c2);
    const float radiusSum = CollisionSystem::GetEffectiveRadius(c1, t1) + CollisionSystem::GetEffectiveRadius(c2, t2);

    // Vector from circle1 to circle2
    Vec2 distanceBtwCircles = { circleCtr2 - circleCtr1 };
    float distanceSquared = distanceBtwCircles.LengthSquared();

    if (distanceSquared > 0.f) {
        // Normal always points from circle1 toward circle2
        normal = distanceBtwCircles / distanceBtwCircles.Length();
        // Penetration = how much they overlap (radiusSum - actualDistance)
        penetration = radiusSum - distanceBtwCircles.Length();
    }
    else {
        // Circles are exactly concentric (centers overlap)
        // Use arbitrary direction; depth = sum of radii
        normal = { 1.f, 0.f };
        penetration = radiusSum;
    }
}

// Computes MTV for circle vs axis-aligned box (mixed shapes)
// This is the most complex test; uses closest-point-on-AABB method
static void ComputeMTV_CircleAABB(const CollisionComponent& circle, const TransformComponent& tCircle,
    const CollisionComponent& box, const TransformComponent& tBox,
    Vec2& normal, float& penetration) {

    const Vec2 halfExtents = CollisionSystem::GetFinalHalfExtents(box, tBox);
    const Vec2 circleCtr = CollisionSystem::GetWorldCenter(tCircle, circle);
    const Vec2 boxCtr = CollisionSystem::GetWorldCenter(tBox, box);
    const float circleRadius = CollisionSystem::GetEffectiveRadius(circle, tCircle);

    // Compute AABB bounds
    const Vec2 boxMin = boxCtr - halfExtents;
    const Vec2 boxMax = boxCtr + halfExtents;

    // Find the point on the AABB closest to the circle center
    // This is done by clamping the circle center to the box bounds
    const float closestX = std::max(boxMin.x, std::min(circleCtr.x, boxMax.x));
    const float closestY = std::max(boxMin.y, std::min(circleCtr.y, boxMax.y));
    const Vec2 closestPt{ closestX, closestY };

    // Vector from closest point on box to circle center
    Vec2 distanceVec = circleCtr - closestPt;
    const float distanceSquared = distanceVec.LengthSquared();

    // CASE 1: Circle center is outside the box
    if (distanceSquared > 0.f) {
        // Normal points from box surface toward circle center
        normal = distanceVec / distanceVec.Length();
        // Penetration = how much circle overlaps the box surface
        penetration = circleRadius - distanceVec.Length();
    }
    else {
        // CASE 2: Circle center is INSIDE the box (very deep penetration)
        // Find the nearest box face and push toward it
        const Vec2 toCenter = circleCtr - boxCtr;

        // Distance from box center to circle center, compared to box half-extent
        // The smaller distance indicates the nearest face
        const float distanceX = halfExtents.x - std::fabs(toCenter.x);  // Distance to left/right face
        const float distanceY = halfExtents.y - std::fabs(toCenter.y);  // Distance to top/bottom face

        if (distanceX < distanceY) {
            // Nearest face is left or right; push away horizontally
            normal = { (toCenter.x < 0.f) ? -1.f : 1.f, 0.f };
            penetration = circleRadius + distanceX;  // Circle plus distance to face
        }
        else {
            // Nearest face is top or bottom; push away vertically
            normal = { 0.f, (toCenter.y < 0.f) ? -1.f : 1.f };
            penetration = circleRadius + distanceY;  // Circle plus distance to face
        }
    }

    // Sanity check: ignore tiny or negative penetrations (numerical noise guard)
    if (penetration <= 0.f) {
        normal = { 0.f, 0.f };
    }
}

// Computes collision normal + penetration depth for capsule vs circle.
// Normal points from capsule toward circle (shape1 -> shape2 convention).
// Uses closest point from circle center to capsule spine, then compares
// distance against sum of radii (capsule cap radius + circle radius).
static void ComputeMTV_CapsuleCircle(const CollisionComponent& capsule, const TransformComponent& tCapsule,
    const CollisionComponent& circle, const TransformComponent& tCircle,
    Vec2& normal, float& penetration) {
    const CapsuleGeom cap = GetCapsuleGeom(capsule, tCapsule);
    const Vec2 circleCtr = CollisionSystem::GetWorldCenter(tCircle, circle);
    const float radiusSum = cap.radius + CollisionSystem::GetEffectiveRadius(circle, tCircle);
    const Vec2 closest = ClosestPointOnSegment(circleCtr, cap.a, cap.b);
    Vec2 d = circleCtr - closest;
    const float distSq = d.LengthSquared();

    if (distSq > 1e-8f) {
        const float dist = std::sqrt(distSq);
        normal = d / dist; // capsule -> circle
        penetration = radiusSum - dist;
    }
    else {
        Vec2 fallback = circleCtr - cap.center;
        if (fallback.LengthSquared() <= 1e-8f) fallback = Vec2{ 0.f, 1.f };
        normal = fallback.Normalised();
        penetration = radiusSum;
    }
}

// Computes collision normal + penetration depth for capsule vs capsule.
// Normal points from capsule1 toward capsule2 (shape1 -> shape2 convention).
// Uses closest points between both spines, then compares against sum of radii.
static void ComputeMTV_CapsuleCapsule(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2,
    Vec2& normal, float& penetration) {
    const CapsuleGeom cap1 = GetCapsuleGeom(c1, t1);
    const CapsuleGeom cap2 = GetCapsuleGeom(c2, t2);
    Vec2 p1{}, p2{};
    ClosestPointsSegmentSegment(cap1.a, cap1.b, cap2.a, cap2.b, p1, p2);
    Vec2 d = p2 - p1; // capsule1 -> capsule2
    const float radiusSum = cap1.radius + cap2.radius;
    const float distSq = d.LengthSquared();

    if (distSq > 1e-8f) {
        const float dist = std::sqrt(distSq);
        normal = d / dist;
        penetration = radiusSum - dist;
    }
    else {
        Vec2 fallback = cap2.center - cap1.center;
        if (fallback.LengthSquared() <= 1e-8f) fallback = Vec2{ 0.f, 1.f };
        normal = fallback.Normalised();
        penetration = radiusSum;
    }
}

// Computes collision normal + penetration depth for capsule vs axis-aligned box.
// Normal points from box toward capsule for compatibility with existing resolver
// call sites. Uses segment-to-box closest points; if segment point is inside box,
// falls back to nearest-face axis resolution.
static void ComputeMTV_CapsuleAABB(const CollisionComponent& capsule, const TransformComponent& tCapsule,
    const CollisionComponent& box, const TransformComponent& tBox,
    Vec2& normal, float& penetration) {
    const CapsuleGeom cap = GetCapsuleGeom(capsule, tCapsule);
    const Vec2 halfExtents = CollisionSystem::GetFinalHalfExtents(box, tBox);
    const Vec2 boxCtr = CollisionSystem::GetWorldCenter(tBox, box);
    const Vec2 boxMin = boxCtr - halfExtents;
    const Vec2 boxMax = boxCtr + halfExtents;

    const Vec2 segClosest = ClosestPointSegmentToAABB(cap.a, cap.b, boxMin, boxMax);
    const Vec2 boxClosest = ClosestPointOnAABB(segClosest, boxMin, boxMax);
    Vec2 d = segClosest - boxClosest; // box -> capsule
    const float distSq = d.LengthSquared();

    if (distSq > 1e-8f) {
        const float dist = std::sqrt(distSq);
        normal = d / dist;
        penetration = cap.radius - dist;
    }
    else {
        const Vec2 toCenter = segClosest - boxCtr;
        const float distanceX = halfExtents.x - std::fabs(toCenter.x);
        const float distanceY = halfExtents.y - std::fabs(toCenter.y);
        if (distanceX < distanceY) {
            normal = Vec2{ (toCenter.x < 0.f) ? -1.f : 1.f, 0.f };
            penetration = cap.radius + distanceX;
        }
        else {
            normal = Vec2{ 0.f, (toCenter.y < 0.f) ? -1.f : 1.f };
            penetration = cap.radius + distanceY;
        }
    }

    if (penetration <= 0.f) {
        normal = Vec2{ 0.f, 0.f };
    }
}


///////////////////////////////////////////////////////////////////////////////
// COLLISION DETECTION (Broad-phase / Narrow-phase test functions)
///////////////////////////////////////////////////////////////////////////////

// Axis-aligned bounding box test (the simplest and fastest collision check)
// Returns true if two AABBs overlap
bool CollisionSystem::CheckAABB(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2) {

    const Vec2 halfExtents1 = GetFinalHalfExtents(c1, t1);
    const Vec2 halfExtents2 = GetFinalHalfExtents(c2, t2);
    const Vec2 boxCtr1 = GetWorldCenter(t1, c1);
    const Vec2 boxCtr2 = GetWorldCenter(t2, c2);

    // Compute AABB bounds in world space
    const Vec2 boxMin1 = boxCtr1 - halfExtents1, boxMax1 = boxCtr1 + halfExtents1;
    const Vec2 boxMin2 = boxCtr2 - halfExtents2, boxMax2 = boxCtr2 + halfExtents2;

    // Check overlap on both axes: if they DON'T overlap on any axis, there's no collision
    return (boxMin1.x <= boxMax2.x && boxMax1.x >= boxMin2.x &&
        boxMin1.y <= boxMax2.y && boxMax1.y >= boxMin2.y);
}

// Circle-to-circle collision test
// Returns true if distance between centers <= sum of radii
bool CollisionSystem::CheckCircleCircle(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2) {

    const Vec2 circleCtr1 = GetWorldCenter(t1, c1);
    const Vec2 circleCtr2 = GetWorldCenter(t2, c2);
    const float radiusSum = GetEffectiveRadius(c1, t1) + GetEffectiveRadius(c2, t2);

    // Compare squared distances to avoid expensive sqrt()
    return Vec2SquareDistance(circleCtr1, circleCtr2) <= (radiusSum * radiusSum);
}

// Circle-to-AABB collision test
// Uses closest-point method: find closest point on box to circle center
bool CollisionSystem::CheckCircleAABB(const CollisionComponent& circle, const TransformComponent& tCircle,
    const CollisionComponent& box, const TransformComponent& tBox) {

    const Vec2 boxHalfExtents = GetFinalHalfExtents(box, tBox);
    const Vec2 circleCtr = GetWorldCenter(tCircle, circle);
    const Vec2 boxCtr = GetWorldCenter(tBox, box);
    const float circleRadius = GetEffectiveRadius(circle, tCircle);

    const Vec2 min = boxCtr - boxHalfExtents;
    const Vec2 max = boxCtr + boxHalfExtents;

    // Find closest point on box to circle center
    const float closestX = std::max(min.x, std::min(circleCtr.x, max.x));
    const float closestY = std::max(min.y, std::min(circleCtr.y, max.y));
    const float distanceX = circleCtr.x - closestX;
    const float distanceY = circleCtr.y - closestY;

    // Compare squared distances (avoids sqrt)
    return (distanceX * distanceX + distanceY * distanceY) <= (circleRadius * circleRadius);
}

// Narrow-phase overlap test for capsule vs circle.
// Collision if distance(closest point on capsule spine, circle center)
// <= (capsule radius + circle radius).
bool CollisionSystem::CheckCapsuleCircle(const CollisionComponent& capsule, const TransformComponent& tCapsule,
    const CollisionComponent& circle, const TransformComponent& tCircle) {
    const CapsuleGeom cap = GetCapsuleGeom(capsule, tCapsule);
    const Vec2 circleCtr = GetWorldCenter(tCircle, circle);
    const float radiusSum = cap.radius + GetEffectiveRadius(circle, tCircle);
    const Vec2 closest = ClosestPointOnSegment(circleCtr, cap.a, cap.b);
    return Vec2SquareDistance(circleCtr, closest) <= radiusSum * radiusSum;
}

// Narrow-phase overlap test for capsule vs axis-aligned box.
// Collision if distance(closest segment point, closest box point) <= capsule radius.
bool CollisionSystem::CheckCapsuleAABB(const CollisionComponent& capsule, const TransformComponent& tCapsule,
    const CollisionComponent& box, const TransformComponent& tBox) {
    const CapsuleGeom cap = GetCapsuleGeom(capsule, tCapsule);
    const Vec2 halfExtents = GetFinalHalfExtents(box, tBox);
    const Vec2 boxCtr = GetWorldCenter(tBox, box);
    const Vec2 boxMin = boxCtr - halfExtents;
    const Vec2 boxMax = boxCtr + halfExtents;
    const Vec2 segClosest = ClosestPointSegmentToAABB(cap.a, cap.b, boxMin, boxMax);
    const Vec2 boxClosest = ClosestPointOnAABB(segClosest, boxMin, boxMax);
    return Vec2SquareDistance(segClosest, boxClosest) <= cap.radius * cap.radius;
}

// Narrow-phase overlap test for capsule vs capsule.
// Collision if distance(closest points between spines) <= sum of capsule radii.
bool CollisionSystem::CheckCapsuleCapsule(const CollisionComponent& c1, const TransformComponent& t1,
    const CollisionComponent& c2, const TransformComponent& t2) {
    const CapsuleGeom cap1 = GetCapsuleGeom(c1, t1);
    const CapsuleGeom cap2 = GetCapsuleGeom(c2, t2);
    Vec2 p1{}, p2{};
    ClosestPointsSegmentSegment(cap1.a, cap1.b, cap2.a, cap2.b, p1, p2);
    const float radiusSum = cap1.radius + cap2.radius;
    return Vec2SquareDistance(p1, p2) <= radiusSum * radiusSum;
}


///////////////////////////////////////////////////////////////////////////////
// COLLISION RESPONSE - DYNAMIC VS STATIC
// Resolves collisions where one body is dynamic and one is static (immovable)
///////////////////////////////////////////////////////////////////////////////

void CollisionSystem::ResolveStaticDynamic(
    TransformComponent& dynT,                    // Local transform of dynamic entity
    const TransformComponent& dynWorldT,         // World transform of dynamic entity
    PhysicsComponent& dynP,                      // Physics data of dynamic entity
    const CollisionComponent& dynC,              // Collision shape of dynamic entity
    const TransformComponent& staWorldT,         // World transform of static entity
    const CollisionComponent& staC,              // Collision shape of static entity
    const TransformComponent* dynParentTransform) // Parent transform (for hierarchy)
{
    Vec2 normal{ 0.f, 0.f };
    float pen = 0.f;

    // Compute MTV based on shape pair
    if (dynC.shape == Shape::Box && staC.shape == Shape::Box) {
        ComputeMTV_BoxBox(staC, staWorldT, dynC, dynWorldT, normal, pen);
    }
    else if (dynC.shape == Shape::Circle && staC.shape == Shape::Circle) {
        ComputeMTV_CircleCircle(staC, staWorldT, dynC, dynWorldT, normal, pen);
    }
    else if (dynC.shape == Shape::Capsule && staC.shape == Shape::Capsule) {
        ComputeMTV_CapsuleCapsule(staC, staWorldT, dynC, dynWorldT, normal, pen);
    }
    else {
        // Mixed shapes
        if (dynC.shape == Shape::Circle && staC.shape == Shape::Box) {
            ComputeMTV_CircleAABB(dynC, dynWorldT, staC, staWorldT, normal, pen);
        }
        else if (dynC.shape == Shape::Box && staC.shape == Shape::Circle) {
            ComputeMTV_CircleAABB(staC, staWorldT, dynC, dynWorldT, normal, pen);
            normal = -normal; // Flip to point static -> dynamic
        }
        else if (dynC.shape == Shape::Capsule && staC.shape == Shape::Circle) {
            ComputeMTV_CapsuleCircle(dynC, dynWorldT, staC, staWorldT, normal, pen);
            normal = -normal; // static(circle) -> dynamic(capsule)
        }
        else if (dynC.shape == Shape::Circle && staC.shape == Shape::Capsule) {
            ComputeMTV_CapsuleCircle(staC, staWorldT, dynC, dynWorldT, normal, pen);
        }
        else if (dynC.shape == Shape::Capsule && staC.shape == Shape::Box) {
            ComputeMTV_CapsuleAABB(dynC, dynWorldT, staC, staWorldT, normal, pen);
        }
        else if (dynC.shape == Shape::Box && staC.shape == Shape::Capsule) {
            ComputeMTV_CapsuleAABB(staC, staWorldT, dynC, dynWorldT, normal, pen);
            normal = -normal; // static(capsule) -> dynamic(box)
        }
    }

    // Early exit: no actual penetration or degenerate normal
    if (pen <= 0.f || (normal.x == 0.f && normal.y == 0.f)) {
        return;
    }

    // Ensure normal is unit length (important for consistent impulse calculations)
    normal = normal.Normalised();

    //////////////////////////////////////////////////////////////////////////
    // POSITIONAL CORRECTION
    // Separates overlapping bodies by moving the dynamic body along the normal
    //////////////////////////////////////////////////////////////////////////
    // Apply MTV as a position correction
    // Only apply a portion (PENETRATION_CORRECTION) to avoid sudden snapping
    // Also convert to local space if entity has a parent in the hierarchy
    ApplyWorldTranslationDelta(dynT, dynParentTransform, normal * pen * PENETRATION_CORRECTION);

    //////////////////////////////////////////////////////////////////////////
    // FORCE CANCELLATION
    // Prevents forces from pushing the dynamic body further into the wall
    //////////////////////////////////////////////////////////////////////////
    // Check if any applied forces are pointing into the collision surface
    float forceAlongNormal = Vec2DotProduct(dynP.netForce, normal);
    if (forceAlongNormal < 0.f) {
        // Remove the component of force pressing into the wall
        // This prevents gravity/external forces from causing sustained penetration
        dynP.netForce -= normal * forceAlongNormal;
    }

    //////////////////////////////////////////////////////////////////////////
    // VELOCITY RESPONSE
    // Removes velocity component perpendicular to the surface (into the wall)
    // This is a simplified collision response (no bounce/restitution for static)
    //////////////////////////////////////////////////////////////////////////
    float velAlongNormal = Vec2DotProduct(dynP.velocity, normal);
    if (velAlongNormal < 0.f) {
        // Entity is moving into the wall; remove that velocity component
        // This leaves tangential (sliding) velocity intact
        dynP.velocity -= normal * velAlongNormal;
    }

    //////////////////////////////////////////////////////////////////////////
    // FRICTION (OPTIONAL - CURRENTLY DISABLED)
    // Reduces sliding velocity along surfaces
    //////////////////////////////////////////////////////////////////////////
    // Decompose velocity into normal and tangential components
    // tangent = velocity - (normal * dot(velocity, normal))
    Vec2 tangent = dynP.velocity - (normal * Vec2DotProduct(dynP.velocity, normal));

    // Only apply friction if there's meaningful tangential velocity
    if (tangent.LengthSquared() > 0.0001f) {
        tangent = tangent.Normalised();
    }

    // Impulse magnitude for friction (currently multiplied by FRICTION_COEFF = 0.0)
    float jt = -Vec2DotProduct(dynP.velocity, tangent);
    jt /= (1.f / dynP.mass);  // Divide by inverse mass (equivalent to multiply by mass^-1)
    Vec2 frictionImpulse = tangent * jt * FRICTION_COEFF;  // Scaled by friction coefficient (0.0)
    dynP.velocity += frictionImpulse / dynP.mass;
}

///////////////////////////////////////////////////////////////////////////////
// COLLISION RESPONSE - DYNAMIC VS DYNAMIC
// Resolves collisions between two moving bodies using impulse-based dynamics
///////////////////////////////////////////////////////////////////////////////

void CollisionSystem::ResolveDynamicDynamic(
    TransformComponent& t1, const TransformComponent& worldT1,
    PhysicsComponent& p1, const CollisionComponent& c1,
    TransformComponent& t2, const TransformComponent& worldT2,
    PhysicsComponent& p2, const CollisionComponent& c2,
    const TransformComponent* parentTransform1, const TransformComponent* parentTransform2)
{
    Vec2 normal{ 0.f, 0.f };
    float pen = 0.f;

    // Compute MTV based on shape pair (same logic as static-dynamic)
    if (c1.shape == Shape::Box && c2.shape == Shape::Box) {
        ComputeMTV_BoxBox(c1, worldT1, c2, worldT2, normal, pen);
    }
    else if (c1.shape == Shape::Circle && c2.shape == Shape::Circle) {
        ComputeMTV_CircleCircle(c1, worldT1, c2, worldT2, normal, pen);
    }
    else if (c1.shape == Shape::Capsule && c2.shape == Shape::Capsule) {
        ComputeMTV_CapsuleCapsule(c1, worldT1, c2, worldT2, normal, pen);
    }
    else {
        if (c1.shape == Shape::Circle && c2.shape == Shape::Box) {
            ComputeMTV_CircleAABB(c1, worldT1, c2, worldT2, normal, pen);
            normal = -normal;  // Flip to point from 1 toward 2
        }
        else if (c1.shape == Shape::Box && c2.shape == Shape::Circle) {
            ComputeMTV_CircleAABB(c2, worldT2, c1, worldT1, normal, pen);
        }
        else if (c1.shape == Shape::Capsule && c2.shape == Shape::Circle) {
            ComputeMTV_CapsuleCircle(c1, worldT1, c2, worldT2, normal, pen);
        }
        else if (c1.shape == Shape::Circle && c2.shape == Shape::Capsule) {
            ComputeMTV_CapsuleCircle(c2, worldT2, c1, worldT1, normal, pen);
            normal = -normal;
        }
        else if (c1.shape == Shape::Capsule && c2.shape == Shape::Box) {
            ComputeMTV_CapsuleAABB(c1, worldT1, c2, worldT2, normal, pen);
        }
        else if (c1.shape == Shape::Box && c2.shape == Shape::Capsule) {
            ComputeMTV_CapsuleAABB(c2, worldT2, c1, worldT1, normal, pen);
            normal = -normal;
        }
    }

    // Early exit: no penetration
    if (pen <= 0.f) {
        return;
    }

    // Ensure unit-length normal
    normal = normal.Normalised();

    //////////////////////////////////////////////////////////////////////////
    // POSITIONAL CORRECTION (SPLIT EQUALLY)
    // Both bodies move half the penetration depth away from each other
    // This distributes the correction fairly based on mass (could be weighted)
    //////////////////////////////////////////////////////////////////////////
    Vec2 correction = normal * (pen * PENETRATION_CORRECTION * 0.5f);
    // Push entity 1 in negative normal direction (away from entity 2)
    ApplyWorldTranslationDelta(t1, parentTransform1, -correction);
    // Push entity 2 in positive normal direction (away from entity 1)
    ApplyWorldTranslationDelta(t2, parentTransform2, correction);


    //////////////////////////////////////////////////////////////////////////
    // FORCE CANCELLATION
    // Prevent applied forces from causing sustained penetration
    //////////////////////////////////////////////////////////////////////////
    // Check if entity 1's forces push it into entity 2 (along normal)
    float f1n = Vec2DotProduct(p1.netForce, normal);
    // Check if entity 2's forces push it into entity 1 (opposite normal)
    float f2n = Vec2DotProduct(p2.netForce, -normal);

    // Remove pressing forces
    if (f1n > 0.f) {
        p1.netForce -= normal * f1n;  // Remove component pushing entity 1 into entity 2
    }
    if (f2n > 0.f) {
        p2.netForce += normal * f2n;  // Remove component pushing entity 2 into entity 1
    }

    //////////////////////////////////////////////////////////////////////////
    // IMPULSE-BASED COLLISION RESPONSE
    // Uses conservation of momentum and coefficient of restitution
    // 
    // Physics Background:
    // - Impulse = change in momentum = force * time (or instant velocity change)
    // - Relative velocity = v2 - v1 (how fast they're approaching along normal)
    // - If relative velocity > 0, they're separating (no collision response needed)
    // - If relative velocity < 0, they're approaching (need impulse to separate)
    // - Coefficient of restitution (e) controls bounce: 0=stick, 1=elastic bounce
    //////////////////////////////////////////////////////////////////////////

    // Calculate relative velocity of entity 2 with respect to entity 1
    Vec2 relativeVel = p2.velocity - p1.velocity;

    // Project relative velocity onto collision normal
    // This gives us the approach speed along the normal
    // Negative = approaching, Positive = separating
    float velAlongNormal = Vec2DotProduct(relativeVel, normal);

    // If already separating, don't apply impulse (they'll naturally separate)
    if (velAlongNormal > 0.f) {
        return;
    }

    // Compute inverse masses (0 if infinite mass / kinematic)
    // Inverse mass allows treating all masses uniformly in the impulse equation
    float invMass1 = (p1.mass > 0.f) ? 1.f / p1.mass : 0.f;
    float invMass2 = (p2.mass > 0.f) ? 1.f / p2.mass : 0.f;

    //////////////////////////////////////////////////////////////////////////
    // IMPULSE CALCULATION
    // Formula: j = -(1 + e) * velAlongNormal / (invMass1 + invMass2)
    //
    // Derivation:
    // 1. Conservation of momentum: m1*v1 + m2*v2 = m1*v1' + m2*v2'
    // 2. Coefficient of restitution: e = -(v1' - v2') / (v1 - v2)
    //    (negative of relative velocity ratio after/before)
    // 3. With impulse J = m1*(v1' - v1) = m2*(v2' - v2):
    //    j = -(1 + e) * velAlongNormal / (invMass1 + invMass2)
    //
    // Simplified: assumes impulse acts only along collision normal
    //////////////////////////////////////////////////////////////////////////

    // Magnitude of impulse to apply
    // (1 + e) factor: e.g., e=1 (elastic) amplifies the impulse for bouncing
    //                  e=0 (plastic) uses minimum impulse just to stop penetration
    float j = -(1.f + RESTITUTION_COEFF) * velAlongNormal;

    // Normalize by total inverse mass (how "responsive" the system is)
    j /= (invMass1 + invMass2);

    // Convert impulse scalar to vector (along collision normal)
    Vec2 impulse = normal * j;

    // Apply impulse: change velocities in opposite directions
    // Entity 1 gets negative impulse (slows down or bounces back)
    p1.velocity -= impulse * invMass1;  // Scaling by invMass ensures correct mass ratios
    // Entity 2 gets positive impulse (speeds up in normal direction)
    p2.velocity += impulse * invMass2;

    //////////////////////////////////////////////////////////////////////////
    // FRICTION (OPTIONAL - CURRENTLY DISABLED)
    // Applies tangential impulse to simulate friction/surface drag
    //////////////////////////////////////////////////////////////////////////

    // Decompose relative velocity: tangent = relativeVel - (normal * dot(relativeVel, normal))
    Vec2 tangent = relativeVel - (normal * Vec2DotProduct(relativeVel, normal));

    // Only apply friction if there's tangential motion
    if (tangent.LengthSquared() > 0.0001f) {
        tangent = tangent.Normalised();
    }

    // Tangential impulse magnitude (similar to normal impulse, but no restitution)
    float jt = -Vec2DotProduct(relativeVel, tangent);
    jt /= (invMass1 + invMass2);

    // Apply friction impulse (scaled by FRICTION_COEFF = 0.0, so currently no effect)
    Vec2 frictionImpulse = tangent * jt * FRICTION_COEFF;
    p1.velocity -= frictionImpulse * invMass1;
    p2.velocity += frictionImpulse * invMass2;
}

///////////////////////////////////////////////////////////////////////////////
// MAIN UPDATE FUNCTION
// Coordinates broad-phase detection, narrow-phase testing, and response
// 
// COLLISION PIPELINE:
//
// 1. Broad-phase (grid)
//    -> find possible pairs
//
// 2. Narrow-phase
//    -> exact shape checks
//
// 3. MTV computation
//    -> get normal + penetration
//
// 4. Resolution
//    -> apply correction to physics owners
///////////////////////////////////////////////////////////////////////////////

void CollisionSystem::Update(Registry& registry) {

    // Get all entities that have both Transform and Collision components
    auto entities = registry.GetEntitiesWithComponents<TransformComponent, CollisionComponent>();

    // Shift current pairs to previous (for enter/stay/exit tracking)
    previousCollisionPairs = std::move(currentCollisionPairs);
    currentCollisionPairs.clear();
    previousTriggerPairs = std::move(currentTriggerPairs);
    currentTriggerPairs.clear();

    // Lambda: Invokes C# script callbacks when collisions occur
    // Iterates through all native scripts on an entity and calls the appropriate handler
    auto invokeScriptCallbacks = [&registry](Registry::Entity target, Registry::Entity other, auto selector) {
        // Get the scripting component for this entity
        auto* nativeScripts = registry.GetComponent<NativeScriptingComponent>(target);
        if (!nativeScripts) return;

        // Create a Collider wrapper for the "other" entity
        const Collider otherCollider{ &registry, other, FindPhysicsOwner(registry, other) };

        // Iterate through all scripts attached to this entity
        for (auto& [name, entry] : nativeScripts->scripts) {
            (void)name;  // Unused

            // Skip if script hasn't started or has faulted
            if (!entry.isStarted) continue;
            if (entry.state == NativeScriptingComponent::ScriptState::Faulted) continue;

            // Get the callback function using the selector lambda
            auto fn = selector(entry);

            // Call the callback if it exists
            if (fn) {
                fn(entry.Instance, otherCollider);
            }
        }
        };

    //////////////////////////////////////////////////////////////////////////
    // BROAD-PHASE: Build spatial structure and update collision bounds
    //////////////////////////////////////////////////////////////////////////

    broadPhase.Clear();
    UpdateStackManager* usm = CEO::Instance().GetManager<UpdateStackManager>();
    TransformComponent worldTransform;

    // For each entity, compute world-space collision bounds
    for (auto entity : entities) {
        // Skip if in lower update stack or inactive
        if (usm->ShouldNotUpdate(entity)) continue;
        if (!usm->IsActive(entity)) continue;

        auto* transformComp = registry.GetComponent<TransformComponent>(entity);
        auto* collisionComp = registry.GetComponent<CollisionComponent>(entity);
        auto* hierarchyComp = registry.GetComponent<HierarchyComponnent>(entity);

        // Build world transform by combining local + parent transforms
        TransformComponent* parentTransform = nullptr;
        if (hierarchyComp && hierarchyComp->parent != 0) {
            parentTransform = registry.GetComponent<TransformComponent>(hierarchyComp->parent);
        }
        worldTransform = BuildWorldTransform(*transformComp, parentTransform);
        const Vec2 center = GetWorldCenter(worldTransform, *collisionComp);

        // Compute AABB bounds based on shape type
        if (collisionComp->shape == Shape::Box) {
            // Boxes: use half-extents to compute bounds
            const Vec2 halfExtent = GetFinalHalfExtents(*collisionComp, worldTransform);
            collisionComp->worldMin = center - halfExtent;
            collisionComp->worldMax = center + halfExtent;
        }
        else if (collisionComp->shape == Shape::Circle) {
            // Circles: form bounding box from radius
            const float r = GetEffectiveRadius(*collisionComp, worldTransform);
            collisionComp->worldMin = center - Vec2(r, r);
            collisionComp->worldMax = center + Vec2(r, r);
        }
        else if (collisionComp->shape == Shape::Capsule) {
            const Vec2 halfExtent = GetFinalHalfExtents(*collisionComp, worldTransform);
            collisionComp->worldMin = center - halfExtent;
            collisionComp->worldMax = center + halfExtent;
        }

        // Update broad-phase spatial partitioning (e.g., grid, quadtree)
        broadPhase.UpdateEntity(registry, entity);
    }

    //////////////////////////////////////////////////////////////////////////
    // BROAD-PHASE: Generate candidate pairs
    // Only these pairs will be tested for actual collision
    //////////////////////////////////////////////////////////////////////////

    std::vector<std::pair<EntityRegistry::Entity, EntityRegistry::Entity>> candidates;
    broadPhase.GetCandidatePairs(candidates);

    // --- DEBUG VISUALIZER START ---
    size_t totalEntities = entities.size();
    size_t totalBroadPairs = candidates.size();
    size_t confirmedPairs = 0;
    // --- DEBUG VISUALIZER END ---

    //////////////////////////////////////////////////////////////////////////
    // NARROW-PHASE: Detailed collision testing and response
    //////////////////////////////////////////////////////////////////////////

    LayerManager* layerManager = CEO::Instance().GetManager<LayerManager>();

    // Pre-fetch storage pointers (optimization: avoid repeated GetStorage calls)
    auto layerStorage = registry.GetStorage<LayerComponent>();
    auto transformStorage = registry.GetStorage<TransformComponent>();
    auto hierarchyStorage = registry.GetStorage<HierarchyComponnent>();
    auto physicsStorage = registry.GetStorage<PhysicsComponent>();
    auto collisionStorage = registry.GetStorage<CollisionComponent>();

    // For each candidate pair from broad-phase
    for (auto& [entity1, entity2] : candidates) {
        // Get layer components for both entities
        LayerComponent* layer1 = layerStorage->Get(entity1);
        LayerComponent* layer2 = layerStorage->Get(entity2);

        // Check if collision is enabled between these two layers
        if (!layerManager->CollisionEnabled(layer1->layer, layer2->layer)) continue;

        // Skip if either entity is in a lower update stack or inactive
        if (usm->ShouldNotUpdate(entity1)) continue;
        if (usm->ShouldNotUpdate(entity2)) continue;
        if (!usm->IsActive(entity1)) continue;
        if (!usm->IsActive(entity2)) continue;

        // Fetch all required components (may be null, especially physics)
        TransformComponent* transformCompEntity1 = transformStorage->Get(entity1);
        TransformComponent* transformCompEntity2 = transformStorage->Get(entity2);
        CollisionComponent* collisionCompEntity1 = collisionStorage->Get(entity1);
        CollisionComponent* collisionCompEntity2 = collisionStorage->Get(entity2);
        HierarchyComponnent* hierarchyCompEntity1 = hierarchyStorage->Get(entity1);
        HierarchyComponnent* hierarchyCompEntity2 = hierarchyStorage->Get(entity2);

        // Resolve the rigid-body owner for each collider. Child colliders can
        // now contribute to their parent's physical collision shape.
        const Registry::Entity ownerEntity1 = FindPhysicsOwner(registry, entity1);
        const Registry::Entity ownerEntity2 = FindPhysicsOwner(registry, entity2);

        // Ignore collisions within the same rigid-body owner (e.g. hurtbox vs
        // env colliders on the same player/enemy hierarchy).
        if (ownerEntity1 != 0 && ownerEntity1 == ownerEntity2) {
            continue;
        }

        TransformComponent* ownerTransformEntity1 = transformStorage->Get(ownerEntity1);
        TransformComponent* ownerTransformEntity2 = transformStorage->Get(ownerEntity2);
        PhysicsComponent* ownerPhysicsEntity1 = physicsStorage->Get(ownerEntity1);  // May be null (static)
        PhysicsComponent* ownerPhysicsEntity2 = physicsStorage->Get(ownerEntity2);  // May be null (static)

        HierarchyComponnent* ownerHierarchyEntity1 = hierarchyStorage->Get(ownerEntity1);
        HierarchyComponnent* ownerHierarchyEntity2 = hierarchyStorage->Get(ownerEntity2);

        // Find parent transforms for collider world-transform support
        TransformComponent* colliderParentTransformEntity1 = nullptr;
        TransformComponent* colliderParentTransformEntity2 = nullptr;
        if (hierarchyCompEntity1 && hierarchyCompEntity1->parent != 0) {
            colliderParentTransformEntity1 = transformStorage->Get(hierarchyCompEntity1->parent);
        }
        if (hierarchyCompEntity2 && hierarchyCompEntity2->parent != 0) {
            colliderParentTransformEntity2 = transformStorage->Get(hierarchyCompEntity2->parent);
        }

        // Find parent transforms for owner-space resolution support
        TransformComponent* ownerParentTransformEntity1 = nullptr;
        TransformComponent* ownerParentTransformEntity2 = nullptr;
        if (ownerHierarchyEntity1 && ownerHierarchyEntity1->parent != 0) {
            ownerParentTransformEntity1 = transformStorage->Get(ownerHierarchyEntity1->parent);
        }
        if (ownerHierarchyEntity2 && ownerHierarchyEntity2->parent != 0) {
            ownerParentTransformEntity2 = transformStorage->Get(ownerHierarchyEntity2->parent);
        }

        // Ensure required components exist
        if (!transformCompEntity1 || !transformCompEntity2 || !collisionCompEntity1 || !collisionCompEntity2
            || !ownerTransformEntity1 || !ownerTransformEntity2) {
            continue;
        }

        // Build world transforms for both entities
        TransformComponent worldTransformEntity1 = BuildWorldTransform(*transformCompEntity1, colliderParentTransformEntity1);
        TransformComponent worldTransformEntity2 = BuildWorldTransform(*transformCompEntity2, colliderParentTransformEntity2);


        //////////////////////////////////////////////////////////////////////////
        // NARROW-PHASE: Run appropriate collision test based on shape pair
        //////////////////////////////////////////////////////////////////////////

        bool hit = false;
        if (collisionCompEntity1->shape == Shape::Box && collisionCompEntity2->shape == Shape::Box) {
            // Box vs Box: AABB test
            hit = CheckAABB(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Circle && collisionCompEntity2->shape == Shape::Circle) {
            // Circle vs Circle: distance test
            hit = CheckCircleCircle(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Capsule && collisionCompEntity2->shape == Shape::Capsule) {
            hit = CheckCapsuleCapsule(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Circle && collisionCompEntity2->shape == Shape::Box) {
            hit = CheckCircleAABB(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Box && collisionCompEntity2->shape == Shape::Circle) {
            hit = CheckCircleAABB(*collisionCompEntity2, worldTransformEntity2,
                *collisionCompEntity1, worldTransformEntity1);
        }
        else if (collisionCompEntity1->shape == Shape::Capsule && collisionCompEntity2->shape == Shape::Circle) {
            hit = CheckCapsuleCircle(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Circle && collisionCompEntity2->shape == Shape::Capsule) {
            hit = CheckCapsuleCircle(*collisionCompEntity2, worldTransformEntity2,
                *collisionCompEntity1, worldTransformEntity1);
        }
        else if (collisionCompEntity1->shape == Shape::Capsule && collisionCompEntity2->shape == Shape::Box) {
            hit = CheckCapsuleAABB(*collisionCompEntity1, worldTransformEntity1,
                *collisionCompEntity2, worldTransformEntity2);
        }
        else if (collisionCompEntity1->shape == Shape::Box && collisionCompEntity2->shape == Shape::Capsule) {
            hit = CheckCapsuleAABB(*collisionCompEntity2, worldTransformEntity2,
                *collisionCompEntity1, worldTransformEntity1);
        }

        // If shapes don't actually collide, skip response
        if (!hit) continue;

        ++confirmedPairs;  // Increment collision counter for debug

        // Create unique pair key for tracking enter/stay/exit
        const PairKey pairKey = MakePairKey(entity1, entity2);

        //////////////////////////////////////////////////////////////////////////
        // TRIGGER HANDLING
        // Triggers are collision volumes that don't create physical responses
        // They just call enter/stay/exit callbacks in scripts
        //////////////////////////////////////////////////////////////////////////

        if (collisionCompEntity1->isTrigger || collisionCompEntity2->isTrigger) {
            currentTriggerPairs.insert(pairKey);

            // Check if this pair was already triggering last frame
            const bool wasTriggering = previousTriggerPairs.find(pairKey) != previousTriggerPairs.end();

            if (wasTriggering) {
                // OnTriggerStay: called every frame while overlapping
                invokeScriptCallbacks(entity1, entity2, [](auto& entry) { return entry.OnTriggerStayFunction; });
                invokeScriptCallbacks(entity2, entity1, [](auto& entry) { return entry.OnTriggerStayFunction; });
            }
            else {
                // OnTriggerEnter: called the first frame of overlap
                invokeScriptCallbacks(entity1, entity2, [](auto& entry) { return entry.OnTriggerEnterFunction; });
                invokeScriptCallbacks(entity2, entity1, [](auto& entry) { return entry.OnTriggerEnterFunction; });
            }
            continue;  // Skip physical response for triggers
        }

        //////////////////////////////////////////////////////////////////////////
        // PHYSICAL COLLISION HANDLING
        // These collisions create forces/impulses and alter motion
        //////////////////////////////////////////////////////////////////////////

        currentCollisionPairs.insert(pairKey);

        // Check if this pair was already colliding last frame
        if (previousCollisionPairs.find(pairKey) != previousCollisionPairs.end()) {
            // OnCollisionStay: called every frame while colliding
            invokeScriptCallbacks(entity1, entity2, [](auto& entry) { return entry.OnCollisionStayFunction; });
            invokeScriptCallbacks(entity2, entity1, [](auto& entry) { return entry.OnCollisionStayFunction; });
        }
        else {
            // OnCollisionEnter: called the first frame of collision
            invokeScriptCallbacks(entity1, entity2, [](auto& entry) { return entry.OnCollisionEnterFunction; });
            invokeScriptCallbacks(entity2, entity1, [](auto& entry) { return entry.OnCollisionEnterFunction; });
        }

        // Queue collision event for other systems
        CEO::Instance().GetManager<EventsDispatcher>()->QueueEvent(Events::CollisionEvent{ entity1, entity2 });

        //////////////////////////////////////////////////////////////////////////
        // CLASSIFY ENTITY TYPES
        // Static = no physics component, kinematic, or mass <= 0
        // Dynamic = has physics and mass > 0
        //////////////////////////////////////////////////////////////////////////

        // Check if each entity has physics
        const bool hasP1 = (ownerPhysicsEntity1 != nullptr);
        const bool hasP2 = (ownerPhysicsEntity2 != nullptr);

        // Classify as static (immovable) or dynamic (movable)
        // Static objects: no physics, marked as kinematic, or zero/negative mass
        const bool e1Static = (!hasP1) || ownerPhysicsEntity1->isKinematic || (ownerPhysicsEntity1->mass <= 0.f);
        const bool e2Static = (!hasP2) || ownerPhysicsEntity2->isKinematic || (ownerPhysicsEntity2->mass <= 0.f);

        //////////////////////////////////////////////////////////////////////////
        // DISPATCH TO APPROPRIATE RESOLUTION METHOD
        //////////////////////////////////////////////////////////////////////////

        if (!e1Static && e2Static && hasP1) {
            // Entity 1 dynamic, Entity 2 static
            ResolveStaticDynamic(*ownerTransformEntity1, worldTransformEntity1,
                *ownerPhysicsEntity1, *collisionCompEntity1,
                worldTransformEntity2, *collisionCompEntity2,
                ownerParentTransformEntity1);
        }
        else if (e1Static && !e2Static && hasP2) {
            // Entity 2 dynamic, Entity 1 static
            ResolveStaticDynamic(*ownerTransformEntity2, worldTransformEntity2,
                *ownerPhysicsEntity2, *collisionCompEntity2,
                worldTransformEntity1, *collisionCompEntity1,
                ownerParentTransformEntity2);
        }
        else if (!e1Static && !e2Static && hasP1 && hasP2) {
            // Both entities dynamic
            ResolveDynamicDynamic(*ownerTransformEntity1, worldTransformEntity1,
                *ownerPhysicsEntity1, *collisionCompEntity1,
                *ownerTransformEntity2, worldTransformEntity2,
                *ownerPhysicsEntity2, *collisionCompEntity2,
                ownerParentTransformEntity1, ownerParentTransformEntity2);
        }
        // If both static: do nothing (immovable objects don't interact)
    }

    //////////////////////////////////////////////////////////////////////////
    // COLLISION EXIT HANDLING
    // Check for collisions that ended (pairs in previous but not current)
    //////////////////////////////////////////////////////////////////////////

    for (const auto& key : previousCollisionPairs) {
        // If this pair is still colliding, skip
        if (currentCollisionPairs.find(key) != currentCollisionPairs.end()) continue;

        // Decode the pair and invoke OnCollisionExit on both entities
        auto [first, second] = DecodePairKey(key);
        invokeScriptCallbacks(first, second, [](auto& entry) { return entry.OnCollisionExitFunction; });
        invokeScriptCallbacks(second, first, [](auto& entry) { return entry.OnCollisionExitFunction; });
    }

    //////////////////////////////////////////////////////////////////////////
    // TRIGGER EXIT HANDLING
    // Check for trigger overlaps that ended
    //////////////////////////////////////////////////////////////////////////

    for (const auto& key : previousTriggerPairs) {
        // If this pair is still triggering, skip
        if (currentTriggerPairs.find(key) != currentTriggerPairs.end()) continue;

        // Decode the pair and invoke OnTriggerExit on both entities
        auto [first, second] = DecodePairKey(key);
        invokeScriptCallbacks(first, second, [](auto& entry) { return entry.OnTriggerExitFunction; });
        invokeScriptCallbacks(second, first, [](auto& entry) { return entry.OnTriggerExitFunction; });
    }

    //////////////////////////////////////////////////////////////////////////
    // DEBUG STATISTICS
    // Print broad-phase efficiency metrics
    //////////////////////////////////////////////////////////////////////////

    if (debugPrintEnabled) {
        if (totalBroadPairs > 0) {
            // Narrow-phase rejection rate: how many broad-phase pairs were rejected by narrow-phase
            float rejection = 100.f * (1.f - (float)confirmedPairs / (float)totalBroadPairs);

            // Compare against brute force: show broad-phase efficiency
            float brutePairs = (float)totalEntities * (totalEntities - 1) / 2.0f;
            float broadCulling = 100.f * (1.f - (float)totalBroadPairs / brutePairs);

            std::cout << "[BroadPhase] Entities: " << totalEntities
                << " | Pairs: " << totalBroadPairs
                << " | Collisions: " << confirmedPairs
                << " | Rejection: " << rejection << "%\n"
                << "[BroadPhase Efficiency] Culled " << broadCulling
                << "% of possible pairs (brute: " << brutePairs << ")\n";
        }
        else {
            std::cout << "[BroadPhase] No active pairs this frame ("
                << totalEntities << " entities)\n";
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // RECOMPUTE AABBs FOR DEBUG VISUALIZATION
    // Update collision bounds after all responses for accurate debug drawing
    //////////////////////////////////////////////////////////////////////////

    for (auto entity : entities) {
        auto* transformComp = registry.GetComponent<TransformComponent>(entity);
        auto* collisionComp = registry.GetComponent<CollisionComponent>(entity);
        if (!usm->IsActive(entity)) continue;

        // Build world transform
        auto* hierarchyComp = registry.GetComponent<HierarchyComponnent>(entity);
        TransformComponent* parentTransform = nullptr;
        if (hierarchyComp && hierarchyComp->parent != 0) {
            parentTransform = registry.GetComponent<TransformComponent>(hierarchyComp->parent);
        }
        worldTransform = BuildWorldTransform(*transformComp, parentTransform);
        const Vec2 center = GetWorldCenter(worldTransform, *collisionComp);

        // Update bounds for debug drawing
        if (collisionComp->shape == Shape::Box) {
            const Vec2 halfExtent = GetFinalHalfExtents(*collisionComp, worldTransform);
            collisionComp->worldMin = center - halfExtent;
            collisionComp->worldMax = center + halfExtent;
        }
        else if (collisionComp->shape == Shape::Circle) {
            const float r = GetEffectiveRadius(*collisionComp, worldTransform);
            collisionComp->worldMin = center - Vec2(r, r);
            collisionComp->worldMax = center + Vec2(r, r);
        }
        else if (collisionComp->shape == Shape::Capsule) {
            const Vec2 halfExtent = GetFinalHalfExtents(*collisionComp, worldTransform);
            collisionComp->worldMin = center - halfExtent;
            collisionComp->worldMax = center + halfExtent;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// AUTO-FIT COLLIDER TO SPRITE
// Analyzes sprite texture alpha channel to create tight-fitting collider bounds
///////////////////////////////////////////////////////////////////////////////

// Automatically sizes a collider to fit the opaque pixels of a sprite
// Useful for non-rectangular sprites or sprites with transparent regions
bool CollisionSystem::AutoFitToSpriteOpaque(Registry& registry, Registry::Entity entity) {

    auto* sprite = registry.GetComponent<SpriteRendererComponent>(entity);
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    auto* collider = registry.GetComponent<CollisionComponent>(entity);
    if (!sprite || !transform || !collider) return false;

    TextureObj* tex = sprite->texture;
    if (!tex || !tex->HasPixelData()) return false;

    // Get raw texture data
    const unsigned char* pixels = tex->PixelData();
    const int channels = tex->Channels();        // 3 = RGB, 4 = RGBA
    const int texW = tex->Width();
    const int texH = tex->Height();

    // Check if texture has alpha channel (4 channels = RGBA)
    const bool hasAlpha = (channels == 4);
    const int alphaOffset = hasAlpha ? 3 : -1;

    //////////////////////////////////////////////////////////////////////////
    // COMPUTE SPRITE FRAME REGION
    // Sprites are often packed in atlases, so we only scan the relevant region
    //////////////////////////////////////////////////////////////////////////

    // UV coordinates [0,1] defining which part of the texture this sprite uses
    Vec2 uv0 = sprite->start;     // Top-left corner
    Vec2 uvSize = sprite->size;   // Width and height in UV space

    // Clamp to valid range (prevent out-of-bounds access)
    uv0.x = std::clamp(uv0.x, 0.f, 1.f);
    uv0.y = std::clamp(uv0.y, 0.f, 1.f);
    uvSize.x = std::clamp(uvSize.x, 0.f, 1.f - uv0.x);
    uvSize.y = std::clamp(uvSize.y, 0.f, 1.f - uv0.y);

    // Convert UV coordinates to pixel coordinates
    const int px0 = static_cast<int>(uv0.x * texW);
    const int py0 = static_cast<int>(uv0.y * texH);
    const int fw = static_cast<int>(uvSize.x * texW);  // Frame width in pixels
    const int fh = static_cast<int>(uvSize.y * texH);  // Frame height in pixels

    if (fw <= 0 || fh <= 0) return false;

    // Clamp frame bounds to texture bounds
    const int px1 = std::min(px0 + fw, texW);
    const int py1 = std::min(py0 + fh, texH);

    //////////////////////////////////////////////////////////////////////////
    // SCAN FOR OPAQUE PIXELS
    // Find the bounding box of all non-transparent pixels
    //////////////////////////////////////////////////////////////////////////

    int minX = fw, minY = fh;  // Initialize to frame size
    int maxX = -1, maxY = -1;  // Initialize to invalid

    for (int y = py0; y < py1; ++y) {
        for (int x = px0; x < px1; ++x) {
            // Calculate linear index into pixel array
            const int idx = (y * texW + x) * channels;

            // Check if this pixel is opaque
            bool opaque = true;
            if (hasAlpha) {
                // For RGBA: opaque if alpha channel (idx + 3) != 0
                opaque = (pixels[idx + alphaOffset] != 0);
            }
            // If no alpha channel, consider all pixels opaque

            if (!opaque) continue;

            // Convert texture coords to frame-relative coords
            int lx = x - px0;
            int ly = y - py0;

            // Update bounding box
            if (lx < minX) minX = lx;
            if (lx > maxX) maxX = lx;
            if (ly < minY) minY = ly;
            if (ly > maxY) maxY = ly;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // FALLBACK FOR ALL-TRANSPARENT SPRITES
    // If no opaque pixels found, use entire frame as collider
    //////////////////////////////////////////////////////////////////////////

    if (maxX < 0 || maxY < 0) {
        // No opaque pixels found; use entire sprite frame
        minX = 0; minY = 0;
        maxX = fw - 1; maxY = fh - 1;
    }

    //////////////////////////////////////////////////////////////////////////
    // CONVERT PIXEL BOUNDS TO NORMALIZED COORDINATES
    // Normalize to [0, 1] range within the sprite frame
    // This makes the collider independent of texture resolution
    //////////////////////////////////////////////////////////////////////////

    const float nMinX = float(minX) / fw;
    const float nMaxX = float(maxX + 1) / fw;
    const float nMinY = float(minY) / fh;
    const float nMaxY = float(maxY + 1) / fh;

    const float nW = nMaxX - nMinX;  // Normalized width
    const float nH = nMaxY - nMinY;  // Normalized height

    //////////////////////////////////////////////////////////////////////////
    // COMPUTE COLLIDER OFFSET
    // Sprite frame origin is at center, so we compute offset from center
    //////////////////////////////////////////////////////////////////////////

    // Compute center of opaque region
    const float nCenterX = (nMinX + nMaxX) * 0.5f;
    const float nCenterY = (nMinY + nMaxY) * 0.5f;

    // Offset from frame center (frame center is at 0.5, 0.5)
    // Range: (-0.5 to 0.5) representing pixels relative to frame center
    const float localOffsetX = nCenterX - 0.5f;
    const float localOffsetY = nCenterY - 0.5f;

    //////////////////////////////////////////////////////////////////////////
    // UPDATE COLLIDER
    // Set size and offset based on opaque region
    //////////////////////////////////////////////////////////////////////////

    if (collider->shape == Shape::Box) {
        // For rectangles: half-extents are half of width/height
        collider->baseHalfExtents = Vec2{ nW * 0.5f, nH * 0.5f };
    }
    else if (collider->shape == Shape::Circle) {
        // For circles: radius is half the maximum dimension
        collider->radius = std::max(nW, nH) * 0.5f;
    }
    else { 
        // For capsules, auto-fit currently initializes half-extents from opaque bounds.
        // Radius/spine are then derived at runtime from these half-extents.
        collider->baseHalfExtents = Vec2{ nW * 0.5f, nH * 0.5f };
    }

    // Set the offset from the sprite's center point
    collider->offset = Vec2{ localOffsetX, localOffsetY };

    return true;
}