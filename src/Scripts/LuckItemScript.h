/*!
@file       LuckItemScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Declares the LuckItemScript class, a UI script for a luck‑granting
            item card. It handles hover/click interactions, visual feedback,
            and updates the player's luck stat.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/GameObjects.h"

class LuckItemScript : public ScriptInstance
{
private:
    ButtonComponent* buttonComp{ nullptr };               // Cached button component for event binding
    SpriteRendererComponent* spriteComp{ nullptr };       // Cached sprite component for texture changes

    TextureObj* cardBackgroundTex{ nullptr };             // Normal background texture
    TextureObj* hoverTex{ nullptr };                       // Hover state texture

    /*!
    * \brief
    *    Private method called when the mouse hovers over the card. Plays a hover
    *    sound, changes texture, and scales up the UI element.
    */
    void onHover();

    /*!
    * \brief
    *    Private method called when the mouse exits the card. Restores the original
    *    texture and size.
    */
    void onHoverExit();

    /*!
    * \brief
    *    Private method called when the card is clicked. Plays a pickup sound,
    *    updates the sprite, deposits luck into the player's stats, and closes the menu.
    */
    void onClick();

    float timeElapsed{ 0 };                                // Timer used for click delay

public:
    /*!
    * \brief
    *    Binds this script instance from the component stored in the registry.
    *    Copies the entity ID and then overwrites this instance with the registry's data.
    */
    void BindFrom() {
        GetComponent<LuckItemScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<LuckItemScript>(*CEO::Get<Registry>());
    };

    /*!
    * \brief
    *    Binds this script instance back to the component in the registry,
    *    overwriting the registry's component with the current local data.
    */
    void BindTo() { *GetComponent<LuckItemScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *    Initialises the script by retrieving components and setting up hover callbacks.
    *
    * \param registry
    *    Reference to the ECS registry.
    */
    void OnStart(Registry& registry);

    /*!
    * \brief
    *    Per‑frame update; empty as no per‑frame logic is needed.
    *
    * \param registry
    *    Reference to the ECS registry.
    * \param dt
    *    Delta time.
    * \param firstframe
    *    Flag indicating first frame.
    */
    void OnUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    Fixed‑step update. Accumulates time and enables the click callback once
    *    the click delay has passed.
    *
    * \param registry
    *    Reference to the ECS registry.
    * \param dt
    *    Fixed delta time.
    * \param firstframe
    *    Flag indicating first fixed update.
    */
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    Loads the card background and hover textures from the resource manager
    *    and assigns the background texture to the sprite.
    *
    * \param r
    *    Reference to the ECS registry.
    */
    void UpdateScriptParams(Registry& r);

    GameObject DescObject;                                 // Child GameObject for description (unused in this script)

    std::string cardBackgroundPath;                        // Path to the normal card background texture (set in inspector)
    std::string hoverTexPath;                              // Path to the hover state texture (set in inspector)

    int luckAdded{ 1 };                                    // Amount of luck added when card is clicked
    float clickDelay{ 0 };                                  // Delay before the card becomes clickable

    REFLECTABLE_PROPERTIES;
};

REFL_AUTO(
    type(LuckItemScript),
    field(DescObject),

    field(cardBackgroundPath),
    field(hoverTexPath),

    field(luckAdded)
)