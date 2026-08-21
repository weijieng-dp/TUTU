/*!
@file       CameraControllerScript.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       21/01/2026

Implementations of the CameraControllerScript class, which controls
camera behavior in gameplay. This script handles smooth camera
following, dead-zone logic, input-based offsets, and transitions
between level floors based on player movement and trigger events.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "CameraControllerScript.h"
#include "../CoreLib/InputManager.h"
#include "../CoreLib/ScreenManager.h"
#include <algorithm>
#include <cmath>
#include <limits>


void CameraControllerScript::OnStart(Registry& r)
{
	// Cache the camera's current position from its transform component
	TransformComponent* camTransform = GetComponent<TransformComponent>(r);
	if (camTransform)
		cameraPos = camTransform->translate;

	// Find and cache reference to the player entity for quick access throughout gameplay
	EntityRegistry::Entity playerEntity = FindPlayer(r);
	if (playerEntity)
		player = GameObject(playerEntity);

	// Initialize state flags for the first frame of camera tracking
	currentTileParent = 0;
	isTransitioning = false;
	hasInitializedTracking = false;
}

void CameraControllerScript::OnUpdate(Registry& r, float dt, bool)
{
	//if (firstframe) {
	//	firstframe = false; //please don't ask why I have to do this, and please do not make any further changes to firstframe anywhere else in the whole codebase!!!
	//	return;
	//}

	// Attempt to re-find the player if the cached reference becomes invalid (e.g., player respawned)
	if (!player.IsValid())
	{
		EntityRegistry::Entity playerEntity = FindPlayer(r);
		if (playerEntity)
			player = GameObject(playerEntity);
	}

	// First frame initialization: determine which tile the player is in and align camera
	if (!hasInitializedTracking)
	{
		TransformComponent* playerTransform = player.GetComponent<TransformComponent>();
		if (!playerTransform)
			return;

		// Find the tile containing the player at startup
		currentTileParent = FindTileParentContainingPlayer(r, playerTransform->translate);
		// Position camera to follow player without smooth interpolation (instantaneous)
		TrackPosition(r, 0.f);
		if (TransformComponent* camTransform = GetComponent<TransformComponent>(r))
			camTransform->translate = cameraPos;

		hasInitializedTracking = true;
		return;
	}

	// Normal frame: update camera position to follow player
	TrackPosition(r, dt);
}


void CameraControllerScript::OnFixedUpdate(Registry& r, float dt, bool firstframe) {
	(void)dt; (void)firstframe;
	// Synchronize camera transform with computed camera position each fixed timestep
	if (TransformComponent* camTransform = GetComponent<TransformComponent>(r))
		camTransform->translate = cameraPos;
}

EntityRegistry::Entity CameraControllerScript::FindPlayer(Registry& registry)
{
	// Search for an entity with both a NameComponent and TransformComponent
	auto entities = registry.GetEntitiesWithComponents<NameComponent, TransformComponent>();
	for (auto candidate : entities)
	{
		NameComponent* name = registry.GetComponent<NameComponent>(candidate);
		// Match entity with name "Player" (must be exact match)
		if (name && name->name == "Player")
			return candidate;
	}
	return 0;
}

EntityRegistry::Entity CameraControllerScript::FindTileParentContainingPlayer(Registry& registry, Vec2 playerPos)
{
	constexpr float tileContainEpsilon = 0.01f;
	
	// Lambda to check if player position is within a tile's playable area (excluding blocked regions)
	auto isInsideTilePlayableArea = [&](Vec2 minBounds, Vec2 maxBounds, bool hasBlocked, Vec2 blockedMin, Vec2 blockedMax) -> bool
		{
			// Check if player is within tile bounds (with small epsilon tolerance for floating point)
			bool insideTile = playerPos.x >= (minBounds.x - tileContainEpsilon) && playerPos.x <= (maxBounds.x + tileContainEpsilon) &&
				playerPos.y >= (minBounds.y - tileContainEpsilon) && playerPos.y <= (maxBounds.y + tileContainEpsilon);
			if (!insideTile)
				return false;

			// If no blocked square, player is in valid playable area
			if (!hasBlocked)
				return true;

			// Check if player is inside the blocked region (if one exists)
			bool insideBlocked = playerPos.x >= (blockedMin.x - tileContainEpsilon) && playerPos.x <= (blockedMax.x + tileContainEpsilon) &&
				playerPos.y >= (blockedMin.y - tileContainEpsilon) && playerPos.y <= (blockedMax.y + tileContainEpsilon);
			// Return true only if player is NOT in the blocked area
			return !insideBlocked;
		};

	// Lambda to quickly check if a specific tile contains the player
	auto containsPlayer = [&](EntityRegistry::Entity tileParent) -> bool
		{
			Vec2 minBounds{}, maxBounds{}, blockedMin{}, blockedMax{};
			bool hasBlocked = false;
			if (!TryGetTileBounds(registry, tileParent, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax))
				return false;

			return isInsideTilePlayableArea(minBounds, maxBounds, hasBlocked, blockedMin, blockedMax);
		};

	// Fast path: if player is still in the current tile, return immediately
	if (currentTileParent && containsPlayer(currentTileParent))
		return currentTileParent;

	// Slow path: search all tiles to find the best match
	auto tiles = registry.GetEntitiesWithComponents<TileCameraBoundsComponent>();
	EntityRegistry::Entity bestTile = 0;
	float bestScore = -std::numeric_limits<float>::max();

	// Iterate through all tiles and rank them by how "centered" the player is within each
	for (auto tileParent : tiles)
	{
		Vec2 minBounds{}, maxBounds{}, blockedMin{}, blockedMax{};
		bool hasBlocked = false;
		if (!TryGetTileBounds(registry, tileParent, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax))
			continue;

		// Skip tiles that don't contain the player
		if (!isInsideTilePlayableArea(minBounds, maxBounds, hasBlocked, blockedMin, blockedMax))
			continue;

		// Calculate how far the player is from the nearest edge
		// Higher score = player is more centered in the tile
		float edgeDistanceX = std::min(playerPos.x - minBounds.x, maxBounds.x - playerPos.x);
		float edgeDistanceY = std::min(playerPos.y - minBounds.y, maxBounds.y - playerPos.y);
		float score = std::min(edgeDistanceX, edgeDistanceY);

		// Keep the tile with the highest score (most centered player position)
		// Tie-breaker: prefer lower entity ID for consistency
		if (score > bestScore || (std::abs(score - bestScore) <= tileContainEpsilon && tileParent < bestTile))
		{
			bestScore = score;
			bestTile = tileParent;
		}
	}

	return bestTile;
}


bool CameraControllerScript::TryGetTileBounds(Registry& registry, EntityRegistry::Entity tileParent, Vec2& outMin, Vec2& outMax,
	bool& outHasBlockedSquare, Vec2& outBlockedMin, Vec2& outBlockedMax) const
{
	TileCameraBoundsComponent* bounds = registry.GetComponent<TileCameraBoundsComponent>(tileParent);
	if (!bounds)
		return false;

	// Bounds in TileCameraBoundsComponent are authored relative to the tile parent.
	// Convert them to runtime world-space by offsetting with the tile parent's world translation.
	Vec2 tileWorldPos = GetWorldPosition(registry, tileParent);

	// Normalize min/max to ensure min is actually less than max (handles swapped values)
	Vec2 localMin = Vec2(std::min(bounds->min.x, bounds->max.x), std::min(bounds->min.y, bounds->max.y));
	Vec2 localMax = Vec2(std::max(bounds->min.x, bounds->max.x), std::max(bounds->min.y, bounds->max.y));
	outMin = localMin + tileWorldPos;
	outMax = localMax + tileWorldPos;

	outHasBlockedSquare = bounds->hasBlockedSquare;

	// Same normalization for blocked region
	Vec2 localBlockedMin = Vec2(std::min(bounds->blockedMin.x, bounds->blockedMax.x), std::min(bounds->blockedMin.y, bounds->blockedMax.y));
	Vec2 localBlockedMax = Vec2(std::max(bounds->blockedMin.x, bounds->blockedMax.x), std::max(bounds->blockedMin.y, bounds->blockedMax.y));
	outBlockedMin = localBlockedMin + tileWorldPos;
	outBlockedMax = localBlockedMax + tileWorldPos;

	return true;
}

CameraControllerScript::DoorSide CameraControllerScript::GetDoorSideFromName(std::string const& name) const
{
	// Map door entity names to their directional side (used for level transitions)
	if (name == "LeftDoor") return DoorSide::Left;
	if (name == "RightDoor") return DoorSide::Right;
	if (name == "TopDoor") return DoorSide::Top;
	if (name == "BottomDoor") return DoorSide::Bottom;
	return DoorSide::None;
}

std::string CameraControllerScript::OppositeDoorName(DoorSide side) const
{
	// Get the opposite door name for level transitions
	// If player exits through "RightDoor", look for "LeftDoor" in the adjacent level
	switch (side)
	{
	case DoorSide::Left: return "RightDoor";
	case DoorSide::Right: return "LeftDoor";
	case DoorSide::Top: return "BottomDoor";
	case DoorSide::Bottom: return "TopDoor";
	default: return {};
	}
}

EntityRegistry::Entity CameraControllerScript::FindAncestorDoorEntity(Registry& registry, EntityRegistry::Entity e) const
{
	// Walk up the hierarchy from a collider entity to find its parent door entity
	EntityRegistry::Entity curr = e;
	while (curr)
	{
		if (NameComponent* name = registry.GetComponent<NameComponent>(curr))
		{
			// Check if this entity's name matches a door (LeftDoor, RightDoor, etc.)
			if (GetDoorSideFromName(name->name) != DoorSide::None)
				return curr;
		}
		HierarchyComponnent* h = registry.GetComponent<HierarchyComponnent>(curr);
		curr = h ? h->parent : 0;
	}
	return 0;
}

EntityRegistry::Entity CameraControllerScript::FindTileParentForEntity(Registry& registry, EntityRegistry::Entity e) const
{
	// Walk up the hierarchy to find the nearest ancestor with a TileCameraBoundsComponent
	// This identifies which tile/level this entity belongs to
	EntityRegistry::Entity curr = e;
	while (curr)
	{
		if (registry.GetComponent<TileCameraBoundsComponent>(curr))
			return curr;
		HierarchyComponnent* h = registry.GetComponent<HierarchyComponnent>(curr);
		curr = h ? h->parent : 0;
	}
	return 0;
}

Vec2 CameraControllerScript::GetWorldPosition(Registry& registry, EntityRegistry::Entity e) const
{
	// Calculate absolute world position by accumulating all parent transforms up the hierarchy
	Vec2 world{};
	EntityRegistry::Entity curr = e;
	while (curr)
	{
		TransformComponent* transform = registry.GetComponent<TransformComponent>(curr);
		if (transform)
			world += transform->translate;

		HierarchyComponnent* hierarchy = registry.GetComponent<HierarchyComponnent>(curr);
		curr = hierarchy ? hierarchy->parent : 0;
	}
	return world;
}

EntityRegistry::Entity CameraControllerScript::FindNearestDoorInOtherTile(Registry& registry, EntityRegistry::Entity fromDoorEntity,
	DoorSide oppositeSide, EntityRegistry::Entity sourceTileParent) const
{
	// When transitioning between tiles, find the door in the adjacent tile that matches the direction
	Vec2 fromDoorWorld = GetWorldPosition(registry, fromDoorEntity);

	// Determine which door name to search for (e.g., exiting Right -> find LeftDoor in next tile)
	std::string targetDoorName = OppositeDoorName(oppositeSide);
	auto entities = registry.GetEntitiesWithComponents<NameComponent, TransformComponent>();

	// Search for the nearest matching door in a different tile
	EntityRegistry::Entity best = 0;
	float bestDistSq = std::numeric_limits<float>::max();
	for (auto e : entities)
	{
		NameComponent* name = registry.GetComponent<NameComponent>(e);
		if (!name || name->name != targetDoorName)
			continue;

		// Ensure the door is in a different tile than the source tile
		EntityRegistry::Entity tileParent = FindTileParentForEntity(registry, e);
		if (!tileParent || tileParent == sourceTileParent)
			continue;

		// Track the nearest door by distance
		Vec2 targetDoorWorld = GetWorldPosition(registry, e);
		float distSq = (targetDoorWorld - fromDoorWorld).LengthSquared();

		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			best = e;
		}
	}

	return best;
}

Vec2 CameraControllerScript::ClampToTileBounds(Vec2 position, Vec2 viewportHalf, Vec2 minBounds, Vec2 maxBounds,
	bool hasBlockedSquare, Vec2 blockedMin, Vec2 blockedMax) const
{
	// Ensure camera doesn't pan beyond the tile boundaries
	// viewportHalf is half the viewport size, used to prevent edge overscroll
	Vec2 minLimit = minBounds + viewportHalf;
	Vec2 maxLimit = maxBounds - viewportHalf;

	// Clamp X axis (handle case where tile is too narrow for viewport)
	if (minLimit.x > maxLimit.x) position.x = (minBounds.x + maxBounds.x) * 0.5f;
	else position.x = std::clamp(position.x, minLimit.x, maxLimit.x);

	// Clamp Y axis (handle case where tile is too short for viewport)
	if (minLimit.y > maxLimit.y) position.y = (minBounds.y + maxBounds.y) * 0.5f;
	else position.y = std::clamp(position.y, minLimit.y, maxLimit.y);

	// If no blocked region exists, return the clamped position
	if (!hasBlockedSquare)
		return position;

	// Check if camera viewport overlaps with the blocked square
	Vec2 camMin = position - viewportHalf;
	Vec2 camMax = position + viewportHalf;
	bool overlap = camMin.x < blockedMax.x && camMax.x > blockedMin.x &&
		camMin.y < blockedMax.y && camMax.y > blockedMin.y;
	if (!overlap)
		return position;

	// Camera overlaps blocked region: push camera away from blocked square using minimal movement
	// Calculate distance to each edge of the blocked square
	float toLeft = std::abs(position.x - (blockedMin.x - viewportHalf.x));
	float toRight = std::abs(position.x - (blockedMax.x + viewportHalf.x));
	float toBottom = std::abs(position.y - (blockedMin.y - viewportHalf.y));
	float toTop = std::abs(position.y - (blockedMax.y + viewportHalf.y));

	// Choose the direction with minimum push distance
	float minPush = std::min(std::min(toLeft, toRight), std::min(toBottom, toTop));
	if (minPush == toLeft) position.x = blockedMin.x - viewportHalf.x;
	else if (minPush == toRight) position.x = blockedMax.x + viewportHalf.x;
	else if (minPush == toBottom) position.y = blockedMin.y - viewportHalf.y;
	else position.y = blockedMax.y + viewportHalf.y;

	// Re-clamp to tile bounds after pushing away from blocked region
	if (minLimit.x <= maxLimit.x) position.x = std::clamp(position.x, minLimit.x, maxLimit.x);
	if (minLimit.y <= maxLimit.y) position.y = std::clamp(position.y, minLimit.y, maxLimit.y);
	return position;
}

void CameraControllerScript::OnPlayerDoorExit(Registry& registry, EntityRegistry::Entity doorColliderEntity)
{
	// Called when player exits a door trigger; initiates level transition animation
	
	CameraComponent* camera = GetComponent<CameraComponent>(registry);
	if (!camera)
		return;

	TransformComponent* playerTransform = player.GetComponent<TransformComponent>();
	if (!playerTransform)
		return;

	// Trace back from the collider to find the actual door entity
	EntityRegistry::Entity exitedDoor = FindAncestorDoorEntity(registry, doorColliderEntity);
	if (!exitedDoor)
		return;

	// Get the door's directional side (Left, Right, Top, Bottom)
	NameComponent* doorName = registry.GetComponent<NameComponent>(exitedDoor);
	if (!doorName)
		return;

	DoorSide side = GetDoorSideFromName(doorName->name);
	if (side == DoorSide::None)
		return;

	// Verify the door is part of the current tile before allowing transition
	EntityRegistry::Entity sourceTileParent = FindTileParentForEntity(registry, exitedDoor);
	if (!sourceTileParent || sourceTileParent != currentTileParent)
		return;

	// Determine which tile the player is moving into
	EntityRegistry::Entity playerCurrentTile = FindTileParentContainingPlayer(registry, playerTransform->translate);
	if (playerCurrentTile == sourceTileParent)
		return;

	// Find the next tile parent (either player's current tile or connected via door)
	EntityRegistry::Entity nextTileParent = 0;
	if (playerCurrentTile && playerCurrentTile != sourceTileParent)
	{
		// Player is already in a new tile; use that tile
		nextTileParent = playerCurrentTile;
	}
	else
	{
		// Find the connected tile by locating the opposite door in another tile
		EntityRegistry::Entity nextDoor = FindNearestDoorInOtherTile(registry, exitedDoor, side, sourceTileParent);
		if (!nextDoor)
			return;
#ifdef _DEBUG
		LOGE("nextDoor location: %f , %f", registry.GetComponent<TransformComponent>(nextDoor)->translate.x, registry.GetComponent<TransformComponent>(nextDoor)->translate.y);
#endif

		nextTileParent = FindTileParentForEntity(registry, nextDoor);
		if (!nextTileParent)
			return;
	}

#ifdef _DEBUG
	//LOGE("Source tile parent name: %s", registry.GetComponent<NameComponent>(sourceTileParent)->name.c_str());
	//LOGE("Next tile parent name: %s", registry.GetComponent<NameComponent>(nextTileParent)->name.c_str());
	//LOGE("Player current tile parent name: %s", registry.GetComponent<NameComponent>(playerCurrentTile)->name.c_str());
#endif

	Vec2 delta{};
	if (side == DoorSide::Right) delta.x = camera->viewportSize.x;
	else if (side == DoorSide::Left) delta.x = -camera->viewportSize.x;
	else if (side == DoorSide::Top) delta.y = camera->viewportSize.y;
	else if (side == DoorSide::Bottom) delta.y = -camera->viewportSize.y;

	Vec2 minBounds{}, maxBounds{}, blockedMin{}, blockedMax{};
	bool hasBlocked = false;
	Vec2 viewportHalf = camera->viewportSize * 0.5f;
	Vec2 target = cameraPos + delta;

	// Clamp target to the new tile's bounds
	if (TryGetTileBounds(registry, nextTileParent, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax))
	{
		target = ClampToTileBounds(target, viewportHalf, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax);
	}

	// Initiate smooth camera transition to the new tile
	currentTileParent = nextTileParent;
	transitionTarget = target;
	isTransitioning = true;
}

void CameraControllerScript::TrackPosition(Registry& r, float dt)
{
	// Main camera following logic: compute target position and smoothly move toward it
	
	if (!player.IsValid())
		return;

	TransformComponent* playerTransform = player.GetComponent<TransformComponent>();
	if (!playerTransform)
		return;

	CameraComponent* camera = GetComponent<CameraComponent>(r);
	if (!camera)
		return;

	// Initialize current tile if not already set
	Vec2 playerPos = playerTransform->translate;
	if (!currentTileParent)
		currentTileParent = FindTileParentContainingPlayer(r, playerPos);
	//LOGE("Player current tile parent name: %s", r.GetComponent<NameComponent>(currentTileParent)->name.c_str());

	Vec2 targetPosition = playerPos;
	Vec2 offset{};
	TransformComponent* camTransform = GetComponent<TransformComponent>(r);
	Vec2 cameraCenter = camTransform ? camTransform->translate : cameraPos;
	Vec2 viewportHalf = camera->viewportSize * 0.5f;

// Platform-specific input handling: PC uses cursor, mobile uses joystick
#ifdef PLATFORM_WINDOWS
	if (!isTransitioning)
	{
		// PC: Camera offset is based on cursor position relative to player
		if (!Input::IsGamepadConnected(0))
		{
			float screenWidth = camera->viewportSize.x;
			float screenHeight = camera->viewportSize.y;
			Vec2 cursorOffsetScreen{ Input::GetGameX(), Input::GetGameY() };
			Vec2 cursorOffsetWorld = {};
			if (screenWidth > 0.f && screenHeight > 0.f)
			{
				// Convert screen-space cursor offset to world-space offset
				cursorOffsetWorld.x = cursorOffsetScreen.x * (camera->viewportSize.x / screenWidth);
				cursorOffsetWorld.y = cursorOffsetScreen.y * (camera->viewportSize.y / screenHeight);
			}
			Vec2 cursorWorld = cameraCenter + cursorOffsetWorld;
			Vec2 delta = cursorWorld - playerPos;
			float distance = delta.Length();
			if (distance > 0.f)
			{
				// Apply dead zone: small cursor movements don't affect camera
				if (useDeadZone && distance <= deadZoneRadius) delta = {};
				else if (useDeadZone) delta *= (distance - deadZoneRadius) / distance;

				// Scale the offset and clamp to maximum allowed camera pan distance
				delta *= pcOffsetScale;
				float maxOffsetX = pcMaxOffset.x > 0.f ? pcMaxOffset.x : viewportHalf.x * pcMaxOffsetScale;
				float maxOffsetY = pcMaxOffset.y > 0.f ? pcMaxOffset.y : viewportHalf.y * pcMaxOffsetScale;
				delta.x = std::clamp(delta.x, -maxOffsetX, maxOffsetX);
				delta.y = std::clamp(delta.y, -maxOffsetY, maxOffsetY);
				offset = delta;
			}
			targetPosition = playerPos + offset;
		}
		else
		{
			Vec2 joystick = Input::GetRightStick();
			Vec2 maxOffset = Vec2(viewportHalf.x * mobileOffsetScale, viewportHalf.y * mobileOffsetScale);
			offset = Vec2(joystick.x * maxOffset.x * 0.5f, joystick.y * maxOffset.y * 0.5f);
			targetPosition = playerPos + offset;
		}
	}
#else
	if (!isTransitioning)
	{
		// Mobile: Camera offset is based on joystick input
		Vec2 joystick = Input::GetControllers()[1].GetJoystickValue();
		Vec2 maxOffset = Vec2(viewportHalf.x * mobileOffsetScale, viewportHalf.y * mobileOffsetScale);
		offset = Vec2(joystick.x * maxOffset.x * 0.5f, joystick.y * maxOffset.y * 0.5f);
		targetPosition = playerPos + offset;
	}
#endif

	// During tile transition, override target to smoothly move toward transition target
	if (isTransitioning)
	{
		targetPosition = transitionTarget;
		// Check if we've reached the transition target within epsilon tolerance
		if ((cameraPos - transitionTarget).Length() <= transitionEpsilon)
			isTransitioning = false;
	}

	// Clamp final target position to current tile's boundaries
	Vec2 minBounds{}, maxBounds{}, blockedMin{}, blockedMax{};
	bool hasBlocked = false;
	if (TryGetTileBounds(r, currentTileParent, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax))
	{
		targetPosition = ClampToTileBounds(targetPosition, viewportHalf, minBounds, maxBounds, hasBlocked, blockedMin, blockedMax);
	}

	// Smooth camera movement using exponential interpolation
	float smoothing = followSmoothing;
	if (dt <= 0.f || smoothing <= 0.f)
	{
		// No smoothing: snap instantly to target
		cameraPos = targetPosition;
		return;
	}

	// Exponential interpolation provides frame-rate-independent smooth following
	float t = 1.f - std::exp(-smoothing * dt);
	cameraPos = Lerp(cameraPos, targetPosition, t);
}
