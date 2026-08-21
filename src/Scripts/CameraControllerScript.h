/*!
@file       CameraControllerScript.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       21/01/2026

Declaration of the CameraControllerScript class, which controls
camera behavior in gameplay. This script handles smooth camera
following, dead-zone logic, input-based offsets, and transitions
between level floors based on player movement and trigger events.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "PlayerControllerScript.h"
#include "../CoreLib/Gameobjects.h"

/*!
* \brief
*    The [CameraControllerScript] class controls camera movement and behavior
*    during gameplay. It is responsible for following the player smoothly,
*    applying dead-zone logic, handling camera offsets for different platforms,
*    and transitioning between level floors when the player moves across
*    boundaries.
*
* \brief
*    Usage:
* \brief
*    - Automatically tracks the player entity once found in the registry.
* \brief
*    - Applies smoothing, offsets, and dead-zone constraints each update.
* \brief
*    - Responds to trigger interactions to initiate floor transitions.
*
* \return
*    [CameraControllerScript] Script instance managing camera behavior.
*/
class CameraControllerScript : public ScriptInstance
{
public:
    /*!
    * \brief
    *    Binds script data from the registry component to this local script instance.
    *    Retrieves the CameraControllerScript component from the registry and copies
    *    its state into this object, ensuring the script has the latest configuration
    *    values. Called by the scripting system before OnUpdate to sync external changes.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void BindFrom() {
        GetComponent<CameraControllerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<CameraControllerScript>(*CEO::Get<Registry>());
    };

    /*!
    * \brief
    *    Binds this script's local data back to the registry component.
    *    Copies the current state of this object into the CameraControllerScript
    *    component stored in the registry, synchronizing any changes made during
    *    script execution. Called by the scripting system after OnUpdate to persist
    *    local changes.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void BindTo() { *GetComponent<CameraControllerScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *    Cached reference to the player entity for quick access during each frame.
    *    Valid if player.IsValid() returns true.
    *
    * \return
    *    [GameObject] The player entity handle.
    */
    GameObject player;

    /*!
    * \brief
    *    Current camera position in world space (center of viewport).
    *
    * \return
    *    [Vec2] The camera's world position.
    */
    Vec2 cameraPos{};

    /*!
    * \brief
    *    Smoothing factor for exponential interpolation of camera position.
    *    Higher values = faster convergence to target. Range: 0+ (0 = instant snap).
    *    Default: 8.0 provides smooth, responsive following.
    *
    * \return
    *    [float] The smoothing coefficient.
    */
    float followSmoothing = 8.f;

    /*!
    * \brief
    *    Scale factor for PC cursor offset influence on camera pan.
    *    Controls how strongly mouse position pulls the camera away from player.
    *    Range: 0 to 1. Default: 0.5 provides moderate look-ahead offset.
    *
    * \return
    *    [float] The PC offset scale multiplier.
    */
    float pcOffsetScale = 0.5f;

    /*!
    * \brief
    *    Scale factor for maximum PC camera offset as a fraction of viewport half-width.
    *    When pcMaxOffset is zero, this scales the viewport to determine max pan distance.
    *    Range: 0 to 1. Default: 0.5 allows camera pan up to 50% of viewport width.
    *
    * \return
    *    [float] The PC maximum offset scale.
    */
    float pcMaxOffsetScale = 0.5f;

    /*!
    * \brief
    *    Absolute maximum camera pan distance on PC in world units.
    *    When x or y components are > 0, override pcMaxOffsetScale for that axis.
    *    When 0, use pcMaxOffsetScale calculation instead.
    *
    * \return
    *    [Vec2] The maximum offset in (x, y) world units.
    */
    Vec2 pcMaxOffset = {};

    /*!
    * \brief
    *    Enable/disable dead zone for mouse cursor offset on PC.
    *    When true, small cursor movements don't affect camera (prevents jitter).
    *    When false, cursor position directly influences camera position.
    *
    * \return
    *    [bool] true to enable dead zone, false for direct cursor control.
    */
    bool useDeadZone = true;

    /*!
    * \brief
    *    Radius of the dead zone around the player on PC in world units.
    *    Cursor movements within this radius do not affect camera offset.
    *    Beyond this radius, offset smoothly increases with distance.
    *
    * \return
    *    [float] The dead zone radius in world units.
    */
    float deadZoneRadius = 50.f;

    /*!
    * \brief
    *    Scale factor for mobile joystick offset influence on camera pan.
    *    Controls how strongly joystick position pulls camera away from player.
    *    Range: 0 to 1. Default: 0.5 provides moderate pan on mobile.
    *
    * \return
    *    [float] The mobile offset scale multiplier.
    */
    float mobileOffsetScale = 0.5f;

    /*!
    * \brief
    *    Entity ID of the current tile/level the player is in.
    *    Used to determine camera bounds and blocked regions.
    *    Updated by FindTileParentContainingPlayer().
    *
    * \return
    *    [EntityRegistry::Entity] The tile parent entity ID.
    */
    EntityRegistry::Entity currentTileParent = 0;

    /*!
    * \brief
    *    Target camera position during floor transition animation.
    *    Only meaningful when isTransitioning is true.
    *
    * \return
    *    [Vec2] The target position in world space.
    */
    Vec2 transitionTarget = {};

    /*!
    * \brief
    *    Flag indicating whether a floor transition animation is active.
    *    When true, camera smoothly interpolates from current to transition target.
    *
    * \return
    *    [bool] true if transition is in progress, false otherwise.
    */
    bool isTransitioning = false;

    /*!
    * \brief
    *    Distance threshold for completing transition animation.
    *    When camera gets within this distance of transitionTarget, transition ends.
    *
    * \return
    *    [float] The completion threshold in world units.
    */
    float transitionEpsilon = 1.f;

    /*!
    * \brief
    *    Flag indicating whether initial camera setup is complete.
    *    Set to true after first frame to prevent repeated initialization.
    *
    * \return
    *    [bool] true if tracking has been initialized, false on first frame.
    */
    bool hasInitializedTracking = false;

    /*!
    * \brief
    *    Called once when the script is initialized. Caches the camera's initial position,
    *    finds the player entity, determines the starting tile, and positions the camera.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    *
    * \return
    *    [void]
    */
    void OnStart(Registry& registry);

    /*!
    * \brief
    *    Called every frame during gameplay. Updates camera position to follow the player,
    *    applies smoothing and dead-zone logic, and handles platform-specific input offsets.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    dt - delta time in seconds since the last frame.
    * \param
    *    firstframe - true if this is the first update after script initialization.
    *
    * \return
    *    [void]
    */
    void OnUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    Called every fixed timestep for deterministic physics or logic updates.
    *    Synchronizes the camera's transform component with the current computed position.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    dt - fixed timestep delta in seconds.
    * \param
    *    firstframe - true if this is the first fixed update after script initialization.
    *
    * \return
    *    [void]
    */
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    Main camera tracking logic. Computes the target position based on player location,
    *    applies smoothing, offset constraints, and tile boundary clamping. Called each frame
    *    by OnUpdate to update the internal cameraPos field.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    dt - delta time in seconds since the last frame.
    *
    * \return
    *    [void]
    */
    void TrackPosition(Registry& registry, float dt);

    /*!
    * \brief
    *    Searches the registry for an entity with the name "Player".
    *    Used to initialize and maintain the player reference.
    *
    * \param
    *    registry - reference to the entity registry to search.
    *
    * \return
    *    [EntityRegistry::Entity] The entity ID of the player, or 0 if not found.
    */
    EntityRegistry::Entity FindPlayer(Registry& registry);

    /*!
    * \brief
    *    Determines which tile the player is currently within based on position.
    *    Scores tiles by how centered the player is and returns the best match.
    *    Accounts for blocked regions within tiles.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    playerPos - the player's current world position.
    *
    * \return
    *    [EntityRegistry::Entity] The entity ID of the tile containing the player,
    *    or 0 if player is not in any valid tile.
    */
    EntityRegistry::Entity FindTileParentContainingPlayer(Registry& registry, Vec2 playerPos);

    /*!
    * \brief
    *    Retrieves the world-space bounds of a tile entity, accounting for the tile's
    *    position and any blocked regions. Converts local bounds to world space using
    *    the tile parent's transform.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    tileParent - the entity ID of the tile to query.
    * \param
    *    outMin - (output) minimum corner of tile bounds in world space.
    * \param
    *    outMax - (output) maximum corner of tile bounds in world space.
    * \param
    *    outHasBlockedSquare - (output) whether a blocked region exists in this tile.
    * \param
    *    outBlockedMin - (output) minimum corner of blocked region in world space.
    * \param
    *    outBlockedMax - (output) maximum corner of blocked region in world space.
    *
    * \return
    *    [bool] true if tile bounds were successfully retrieved, false if component missing.
    */
    bool TryGetTileBounds(Registry& registry, EntityRegistry::Entity tileParent, Vec2& outMin, Vec2& outMax,
        bool& outHasBlockedSquare, Vec2& outBlockedMin, Vec2& outBlockedMax) const;

    /*!
    * \brief
    *    Called when the player exits a door trigger. Initiates a smooth camera transition
    *    to the adjacent tile/level. Determines the transition target and sets up animation.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    doorColliderEntity - the collider entity that detected the player exit.
    *
    * \return
    *    [void]
    */
    void OnPlayerDoorExit(Registry& registry, EntityRegistry::Entity doorColliderEntity);

    REFLECTABLE_PROPERTIES;

private:
    /*!
    * \brief
    *    Enumeration representing which side of a tile a door is on.
    *    Used for transition logic and opposite door matching.
    */
    enum class DoorSide { None, Left, Right, Top, Bottom };

    /*!
    * \brief
    *    Converts a door entity's name to its directional side.
    *    Names: "LeftDoor", "RightDoor", "TopDoor", "BottomDoor".
    *
    * \param
    *    name - the name to parse (typically from NameComponent).
    *
    * \return
    *    [DoorSide] The corresponding side enum, or DoorSide::None if name doesn't match.
    */
    DoorSide GetDoorSideFromName(std::string const& name) const;

    /*!
    * \brief
    *    Gets the opposite door side for level transitions.
    *    Example: Left -> Right (exiting left door means look for right door in next tile).
    *
    * \param
    *    side - the exit side to find the opposite of.
    *
    * \return
    *    [std::string] The opposite door name, or empty string if side is None.
    */
    std::string OppositeDoorName(DoorSide side) const;

    /*!
    * \brief
    *    Walks up the entity hierarchy to find the ancestor door entity.
    *    Used to identify which door triggered a level transition.
    *
    * \param
    *    registry - reference to the entity registry for hierarchy access.
    * \param
    *    e - the starting entity (typically a collider child of a door).
    *
    * \return
    *    [EntityRegistry::Entity] The ancestor door entity ID, or 0 if not found.
    */
    EntityRegistry::Entity FindAncestorDoorEntity(Registry& registry, EntityRegistry::Entity e) const;

    /*!
    * \brief
    *    Walks up the entity hierarchy to find the nearest ancestor with a
    *    TileCameraBoundsComponent, which identifies the tile parent.
    *
    * \param
    *    registry - reference to the entity registry for hierarchy access.
    * \param
    *    e - the starting entity (typically a door or door collider).
    *
    * \return
    *    [EntityRegistry::Entity] The tile parent entity ID, or 0 if not found.
    */
    EntityRegistry::Entity FindTileParentForEntity(Registry& registry, EntityRegistry::Entity e) const;

    /*!
    * \brief
    *    Calculates the absolute world position of an entity by accumulating
    *    transforms up the entity hierarchy (all parent transforms).
    *
    * \param
    *    registry - reference to the entity registry for hierarchy access.
    * \param
    *    e - the entity to query.
    *
    * \return
    *    [Vec2] The world position of the entity.
    */
    Vec2 GetWorldPosition(Registry& registry, EntityRegistry::Entity e) const;

    /*!
    * \brief
    *    Finds the nearest door in a different tile that matches a specified direction.
    *    Used during level transitions to locate the destination door entity.
    *
    * \param
    *    registry - reference to the entity registry for searching.
    * \param
    *    fromDoorEntity - the exit door the player just left.
    * \param
    *    oppositeSide - the direction to search (opposite of exit side).
    * \param
    *    sourceTileParent - the tile being exited (to exclude from search).
    *
    * \return
    *    [EntityRegistry::Entity] The nearest matching door in another tile, or 0 if none found.
    */
    EntityRegistry::Entity FindNearestDoorInOtherTile(Registry& registry, EntityRegistry::Entity fromDoorEntity,
        DoorSide oppositeSide, EntityRegistry::Entity sourceTileParent) const;

    /*!
    * \brief
    *    Constrains a camera position to remain within tile boundaries while accounting
    *    for viewport dimensions and blocked regions. Ensures the camera doesn't pan
    *    outside the playable area and avoids blocked squares by pushing the camera away.
    *
    * \param
    *    position - the position to clamp.
    * \param
    *    viewportHalf - half the viewport dimensions (used for edge clamping).
    * \param
    *    minBounds - minimum corner of tile bounds in world space.
    * \param
    *    maxBounds - maximum corner of tile bounds in world space.
    * \param
    *    hasBlockedSquare - whether a blocked region exists in the tile.
    * \param
    *    blockedMin - minimum corner of blocked region in world space.
    * \param
    *    blockedMax - maximum corner of blocked region in world space.
    *
    * \return
    *    [Vec2] The clamped camera position within bounds and outside blocked regions.
    */
    Vec2 ClampToTileBounds(Vec2 position, Vec2 viewportHalf, Vec2 minBounds, Vec2 maxBounds,
        bool hasBlockedSquare, Vec2 blockedMin, Vec2 blockedMax) const;

};
REFL_AUTO(
    type(CameraControllerScript),
    field(followSmoothing),
    field(pcOffsetScale),
    field(pcMaxOffsetScale),
    field(pcMaxOffset),
    field(useDeadZone),
    field(deadZoneRadius),
    field(mobileOffsetScale),
    field(cameraPos),
    field(player)
)
