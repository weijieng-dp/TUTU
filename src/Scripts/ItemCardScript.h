/*!
@file       ItemCardScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Defines the ItemCardScript class, a script instance that manages an
            interactive item card in the UI. It displays item information,
            handles hover and click events, and applies the corresponding item
            effects when selected. The script also supports delayed activation
            and dynamic texture updates.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/GameObjects.h"

class ItemCardScript : public ScriptInstance
{
private:
    Item item;                          // The item data associated with this card

    ButtonComponent* buttonComp{ nullptr };               // Cached button component for event binding
    SpriteRendererComponent* spriteComp{ nullptr };       // Cached sprite component for texture changes

    TextureObj* cardBackgroundTex{ nullptr };             // Normal background texture
    TextureObj* hoverTex{ nullptr };                       // Hover state texture

    float timeElapsed{ 0 };                                // Timer used for click delay

public:
    /*!
    * \brief
    *    Binds this script instance from the component stored in the registry.
    *    Copies the entity ID and then overwrites this instance with the registry's data.
    */
    void BindFrom() {
        GetComponent<ItemCardScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ItemCardScript>(*CEO::Get<Registry>());
    };

    /*!
    * \brief
    *    Binds this script instance back to the component in the registry,
    *    overwriting the registry's component with the current local data.
    */
    void BindTo() { *GetComponent<ItemCardScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *    Initialises the script: obtains button and sprite components, sets hover
    *    callbacks, resets timer, and calls UpdateScriptParams to load textures and
    *    set up the item display.
    *
    * \param registry
    *    Reference to the ECS registry.
    */
    void OnStart(Registry& registry);

    /*!
    * \brief
    *    Per‑frame update; empty implementation because no per‑frame logic is needed.
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
    *    Loads the card background and hover textures from the resource manager,
    *    assigns the background to the sprite, and retrieves a random item from
    *    the item pool based on the configured rarity and pool. Updates the child
    *    GameObjects (item icon and description) with the item's data.
    *
    * \param r
    *    Reference to the ECS registry.
    */
    void UpdateScriptParams(Registry& r);

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
    *    updates the sprite, adds the item to the player via ItemManager, handles
    *    rarity overflow and luck reset, and closes the menu.
    */
    void onClick();

    // Child GameObject that displays the item's icon
    GameObject itemObject;

    // Child GameObject that displays the item's description text
    GameObject itemDescObject;

    // Path to the normal card background texture (set in inspector)
    std::string cardBackgroundPath;

    // Path to the hover state texture (set in inspector)
    std::string hoverTexPath;

    // Rarity of the item to be displayed (used for filtering)
    int rarity{ -1 };

    // Name of the item pool from which to draw (e.g., "Common", "Boss")
    std::string itemPool;

    // Delay before the card becomes clickable (used for staggered appearance)
    float clickDelay{};

    REFLECTABLE_PROPERTIES;
};

REFL_AUTO(
    type(ItemCardScript),
    field(itemObject),
    field(itemDescObject),

    field(cardBackgroundPath),
    field(hoverTexPath),

    field(rarity),
    field(itemPool)
)