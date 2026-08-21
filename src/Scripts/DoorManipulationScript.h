/*!
@file       DoorManipulationScript.h
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Script that handles door behaviour (like opening and closing
            when player moves into a tile). Also updates the position
            for minimap camera.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class DoorManipulationScript: public ScriptInstance {
public:
	void BindFrom() { 
        GetComponent<DoorManipulationScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<DoorManipulationScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<DoorManipulationScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *	Opens the door that this script is attached to when
    *   combat is done and is attached to another tile.
    * \param[in] registry - The registry to get the entities for drawing.
    */
    void OpenDoor(Registry& registry);

    /*!
    * \brief Closes all doors.
    * \param[in] registry - The registry to get the entities for drawing.
    */
    void CloseDoor(Registry& registry);

    /*!
    * \brief Plays dialogue for first entry on a specific tile shape.
    */
    void EnableDialogue();
    
    /*!
    * \brief 
    *   Collision on trigger exit call back. If room isn't cleared,
    *   it will close doors and call function for spawning enemies.
    *   Also updates minimap position and disables minimap if toggle
    *   for combat is on.
    * \param[in] other - The entity that exited the door's collider.
    */
    void OnTriggerExit(const Collider& other);

    /*!
    * \brief
    *   Collision on trigger enter call back. Check if it's the first time
    *   the player is entering a specific tile shape, plays dialogue if yes.
    * \param[in] other - The entity that exited the door's collider.
    */
    void OnTriggerEnter(const Collider& other);

    // Direction: 0 is NORTH | 1 == SOUTH | 2 == EAST | WEST == 3
    int direction{};                // which direction is this door blocker on
    int tileInstanceIndex{ -1 };    // index into MapManager::tileInstance
    int localGridPosX{ 0 };         // relative to tile's anchor (bottom most left)
    int localGridPosY{ 0 };         // relative to tile's anchor (bottom most left)
   
    REFLECTABLE_PROPERTIES;
private:
    EntityRegistry::Entity enemySpawner{};      // the entity with the enemy spawner script
    EntityRegistry::Entity doorSpriteEnt{};     // the entity with the door sprite
    EntityRegistry::Entity postProcessEnt{};    // the entity with the post process
    EntityRegistry::Entity minimapEnt{};        // the entity with the minimap script
    EntityRegistry::Entity dialogueEnt{};       // the entity with the dialogue script
    EntityRegistry::Entity playerEnt{};         // the player entity
    bool doorOpened{ false };                   // whether door has been opened
};
REFL_AUTO(
    type(DoorManipulationScript),
    field(direction),
    field(tileInstanceIndex),
    field(localGridPosX),
    field(localGridPosY)
)