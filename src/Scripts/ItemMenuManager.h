/*!
@file       ItemMenuManager.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Declares the ItemMenuManager class, which controls the item selection
            menu. It creates and configures the item, heal, and luck cards,
            and manages their staggered appearance using a popup timer.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/StatsManager.h"

class ItemMenuManager : public ScriptInstance
{
public:
    /*!
    * \brief
    *    Binds this script instance from the component stored in the registry.
    *    Copies the entity ID and then overwrites this instance with the registry's data.
    */
    void BindFrom() {
        GetComponent<ItemMenuManager>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ItemMenuManager>(*CEO::Get<Registry>());
    };

    /*!
    * \brief
    *    Binds this script instance back to the component in the registry,
    *    overwriting the registry's component with the current local data.
    */
    void BindTo() { *GetComponent<ItemMenuManager>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *    On start, initialises the item cards (item, heal, luck) by setting their
    *    parameters based on the player's current rarity and the configured popup time.
    *
    * \param registry
    *    Reference to the ECS registry.
    */
    void OnStart(Registry& registry);

    /*!
    * \brief
    *    Per‑frame update: enables the cards one by one according to the popupTime
    *    progression and accumulates elapsed time.
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
    *    Fixed update; empty implementation.
    *
    * \param registry
    *    Reference to the ECS registry.
    * \param dt
    *    Fixed delta time.
    * \param firstframe
    *    Flag indicating first fixed update.
    */
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    // GameObject references for the three card buttons (set in inspector)
    GameObject itemButton;   // Main item card
    GameObject healButton;   // Healing item card
    GameObject luckButton;   // Luck‑granting card

    float popupTime{};       // Total time over which cards appear (staggered)

    REFLECTABLE_PROPERTIES;

private:
    /*!
    * \brief
    *    Private helper that configures the itemButton and healButton with the
    *    appropriate card background and hover textures based on the given rarity.
    *    Also sets the click delay and calls UpdateScriptParams on each card.
    *
    * \param rarity
    *    The current rarity value from StatsManager.
    */
    void InitItems(int rarity);

    float timeElapsed{};     // Timer used for staggered activation

    // Texture paths for different rarities (set in code)
    std::string card1{ "items\\item_1.png" };
    std::string card1Hover{ "items\\item_1_hover.png" };

    std::string card2{ "items\\item_2.png" };
    std::string card2Hover{ "items\\item_2_hover.png" };

    std::string card3{ "items\\item_3.png" };
    std::string card3Hover{ "items\\item_3_hover.png" };

    std::string card4{ "items\\item_4.png" };
    std::string card4Hover{ "items\\item_4_hover.png" };

    // Generic non‑item card textures (for luck and fallback)
    std::string cardNon{ "items\\item_luck.png" };
    std::string cardNonHover{ "items\\item_luck_hover.png" };
};

REFL_AUTO(
    type(ItemMenuManager),
    field(itemButton),
    field(healButton),
    field(luckButton),
    field(popupTime)
)