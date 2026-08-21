/*!
@file       HealthbarScript.h
@author     Ng Wei Jie (weijie.ng) (100%)
@date       5/2/2026
@brief
    Implements the HealthbarScript, which is responsible for visually
    representing an entity's health using heart icons in the UI.

    The script spawns, positions, and updates heart GameObjects based on
    the target entity's current and maximum health. It supports full,
    half, and empty heart states.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*/
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"
#include "../CoreLib/Texture.h"


class HealthbarScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<HealthbarScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<HealthbarScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<HealthbarScript>(*CEO::Get<Registry>()) = *this; };

    std::vector<GameObject> HealthGameObjects;
    GameObject HealthPrefab{};
    GameObject TargetGameobject{};
    float HealthPadding{};
    TextureObj* EmptyHeartTexture{};
    TextureObj* FullHeartTexture{};
    TextureObj* HalfHeartTexture{};

    /*!
* \brief
*   Initializes the health bar on startup by calculating the required
*   number of heart icons from the target entity's maximum health.
*   Spawns missing heart GameObjects and caches heart textures.
*/
    void OnStart(Registry& registry);

    /*!
* \brief
*   Frame-based update callback.
*
*   Currently unused as the health bar is updated explicitly through
*   UpdateHealthbar().
*/
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
* \brief
*   Fixed timestep update callback.
*
*   Currently unused. Health bar updates are event-driven rather than
*   physics-based.
*/
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    /*!
* \brief
*   Instantiates a new heart UI GameObject and attaches it as a child
*   of the health bar entity.
*
*   The heart is positioned horizontally based on its index to ensure
*   consistent spacing within the health bar.
*/
    void AddHealthGameobject();

    /*!
* \brief
*   Updates the visual state of the health bar based on current and
*   maximum health values.
*
* \param
*   currHealth - The current health value of the target entity.
* \param
*   maxHealth  - The maximum health value of the target entity.
*
*   Converts health values into full, half, and empty heart icons and
*   updates the associated sprite textures accordingly.
*/
    void UpdateHealthbar(int currHealth, int maxHealth);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(HealthbarScript),
    field(HealthPrefab),
    field(HealthPadding)

)