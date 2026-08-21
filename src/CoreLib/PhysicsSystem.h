/*!
@file       PhysicsSystem.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file declares the [PhysicsSystem] class, which manages the simulation
of physics for entities in the game world. It provides functionality for
updating velocity, acceleration, and position each frame, as well as
controls for pausing, stepping, and debugging physics updates.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "Components.h" 

/*!
* \brief
*    The [PhysicsSystem] class manages the simulation of physics in the game world.
*    It updates positions and velocities of entities with physics components and
*    provides debugging controls such as pausing and single-step simulation.
*    This class is implemented as a singleton, ensuring a single shared instance
*    throughout the application.
*
* \brief
*    Usage:
* \brief
*    - Retrieve the singleton instance using `PhysicsSystem::Instance()`.
* \brief
*    - Call `Update(registry, dt)` every frame to process all physics-enabled entities.
* \brief
*    - Use `SetPause()` to toggle the pause state, freezing or resuming physics updates.
* \brief
*    - Call `TriggerSingleStep()` to advance the simulation by one frame when paused.
* \brief
*    - Use `ShouldStep()` to check if physics should update in the current frame.
*
* \return
*    [PhysicsSystem&] Reference to the singleton instance, used to access
*    and control physics updates globally.
*/
class PhysicsSystem {
public:

    /*!
    * \brief
    *    Retrieves the singleton instance of the [PhysicsSystem] class.
    *
    * \param
    *    [None]
    *
    * \return
    *    [PhysicsSystem&] Reference to the singleton instance.
    */
    static PhysicsSystem& Instance();

    // ===============================
    // Core Physics Update
    // ===============================    

    /*!
    * \brief
    *    Updates all physics-enabled entities by applying velocity and acceleration
    *    over the given time step.
    *
    * \param
    *    [Registry&] registry - The registry containing entities and components.
    * \param
    *    [float] dt - The delta time (time step) to advance the simulation.
    *
    * \return
    *    [void]
    */
    void Update(Registry& registry, float dt);

    // ===============================
    // Helper Utilities
    // ===============================

    /*!
    * \brief
    *    Applies an external force to a specified entity’s physics component.
    *    The resulting acceleration is determined by the entity’s mass and
    *    contributes to velocity updates during simulation.
    *
    * \param
    *    [EntityRegistry::Entity] e - The target entity to apply the force to.
    * \param
    *    [const Vec2&] force - The external force vector to apply.
    * \param
    *    [Registry&] registry - The registry containing entity components.
    *
    * \return
    *    [void]
    */
    void ApplyForce(EntityRegistry::Entity e, const Vec2& force, Registry& registry);

    /*!
    * \brief
    *    Enables or disables debug printing for physics-related operations.
    *    When enabled, the system may print entity state data each frame.
    *
    * \param
    *    [bool] b - True to enable debug print; false to disable it.
    *
    * \return
    *    [void]
    */
    inline void SetDebugPrint(bool b) { debugPrintEnabled = b; }
    
    /*!
    * \brief
    *    Provides modifiable access to the pause state of the physics system.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool&] Reference to the pause state.
    *           - True: Physics updates are frozen.
    *           - False: Physics updates run normally.
    */
    bool& SetPause() { return paused; }

    /*!
    * \brief
    *    Provides read-only access to the pause state of the physics system.
    *
    * \param
    *    [None]
    *
    * \return
    *    [const bool&] Constant reference to the pause state.
    */
    const bool& SetPause() const { return paused; }

    /*!
    * \brief
    *    Provides modifiable access to the single-step state flag.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool&] Reference to the single-step state flag.
    *           - True: Advance exactly one frame while paused.
    *           - False: Disabled.
    */
    bool& SetSingleStep() { return singleStepTriggered; }

    /*!
    * \brief
    *    Provides read-only access to the single-step state flag.
    *
    * \param
    *    [None]
    *
    * \return
    *    [const bool&] Constant reference to the single-step state flag.
    */
    const bool& SetSingleStep() const { return singleStepTriggered; }

    /*!
    * \brief
    *    Explicitly triggers a single-step update while the physics system is paused.
    *    This allows developers to advance the simulation by exactly one frame.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void TriggerSingleStep();

    /*!
    * \brief
    *    Checks whether the physics system should perform an update this frame.
    *    Updates occur if the system is not paused or if a single-step was triggered.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool]
    *           - True: Update should occur this frame.
    *           - False: Skip physics updates.
    */
    inline bool ShouldStep() const {
        return !paused || singleStepTriggered;
    }

private:

    //Debug Control Flags
    /*!
    * \brief
    *    Flag controlling whether physics updates are currently paused.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if updates are paused; false otherwise.
    */
    bool paused = false;

    /*!
    * \brief
    *    Flag used to advance the simulation by exactly one step while paused.
    *    Automatically resets after the frame is processed.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if a single-step update was triggered.
    */
    bool singleStepTriggered = false;

    /*!
    * \brief
    *    Controls whether physics debug information (entity positions, velocities)
    *    is printed to the console for developer inspection.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if debug printing is enabled; false otherwise.
    */
    bool debugPrintEnabled = false;
};
