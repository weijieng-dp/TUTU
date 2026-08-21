/**___________________________________________________________________________/
@file          UpdateStackManager.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple manager to skip updates on entities that are on a "lower" stack

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "CEO.h"
#include "Registry.h"
#include "Components.h"
//#include <functional>

class UpdateStackManager {
    int CurrStack{ 0 }; // tracks the current "layer"  

    Registry* registry;
    ComponentStorage<ActiveComponent>* activeCompStorage;
    ComponentStorage<UpdateStackComponent>* updCompStorage;
public:

    void UpdateStorages();

    /*!
    * \brief
    *    Returns if an entity should update in this cycle
    *
    * \param [EntityRegistry::Entity] the entity
    * 
    * \return [bool] yes or no
    */
    bool ShouldUpdate(EntityRegistry::Entity e) const;

    /*!
    * \brief
    *    Returns if an entity should update in this cycle
    *
    * \param [EntityRegistry::Entity] the entity
    *
    * \return [bool] yes or no
    */
    bool ShouldUpdate(UpdateStackComponent const* usc) const;

    /*!
    * \brief
    *    Returns if an entity should NOT update in this cycle
    *
    * \param [EntityRegistry::Entity] the entity
    * 
    * \return [bool] yes or no
    */
    bool ShouldNotUpdate(EntityRegistry::Entity e) const { return !ShouldUpdate(e); }

    bool IsActive(EntityRegistry::Entity e) const;

    /*!
    * \brief
    *    Get the current "layer"
    */
    int GetStack() const { return CurrStack; }
    /*!
    * \brief
    *    Set the current "layer"
    */
    void SetStack(int i) { CurrStack = i; } 

    void IncrementStack() { CurrStack++; }
    void DecrementStack() { if (CurrStack > 0) CurrStack--; }
};