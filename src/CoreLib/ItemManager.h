/*!
@file       ItemManager.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Declares the Item and ItemManager classes. Item represents a
            gameplay item with attributes, effects, and evolution data.
            ItemManager is responsible for loading, storing, and distributing
            items, as well as applying their effects to the player's stats.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#pragma once
#include <random>
#include "StatsManager.h"
#include <unordered_map>
#include <list>
struct Item {
    std::string name;
    std::string description;
    std::string iconPath;
    std::string type;
    std::vector<std::string> pool;
    int rarity;

    struct Effect {
        std::string name;
        bool isMultiplicative;
        double effectValue;
    };

    std::vector<Effect> effectList;

    bool isStackable;
    std::string evolvesInto;
    bool isEvolution;

    std::vector<std::string> itemsToRemove;
    /*!
    * \brief
    *    Constructs an invalid item
    */
    Item();

    /*!
    * \brief
    *    Constructs a fully defined item.
    *
    * \param __name          Item name.
    * \param __desc          Item description.
    * \param __iconPath      Path to the item's icon.
    * \param __type          Item type (e.g., "Projectile", "Passive").
    * \param __pool          List of pool names this item belongs to.
    * \param __rarity        Rarity value (higher = rarer).
    * \param __effectList    List of effects this item grants.
    * \param __isStackable   Whether multiple copies can be active.
    * \param __evolvesInto   Name of the item this evolves into.
    * \param __isEvolution   Whether this item is an evolution of another.
    * \param __itemsToRemove List of items to be removed when this is picked.
    */
    Item(std::string const& __name,
        std::string const& __desc,
        std::string const& __iconPath,
        std::string const& __type,
        std::vector<std::string> const& __pool,
        int                                __rarity,
        std::vector<Effect> const& __effectList,
        bool                               __isStackable,
        std::string const& __evolvesInto,
        bool                               __isEvolution,
        std::vector<std::string> const& __itemsToRemove);

    /*!
    * \brief
    *    Checks whether the item contains valid data.
    *
    * \return
    *    True if the item was successfully loaded and is usable.
    */
    bool IsValid() const;

private:
    bool isValid;
};


class ItemManager {
public:


    ItemManager();
    /*!
    * \brief
    *    Initialises the manager by loading item data from JSON.
    */
    void Init();
    /*!
    * \brief
    *    Resets all item pools and clears active items.
    */
    void ResetItems();
    /*!
    * \brief
    *    (Re)builds the item pools from the master item list.
    */
    void InitItemPool();
    /*!
    * \brief
    *    Returns a random item, optionally filtered by rarity.
    */
    Item GetRandomItem(int rarityFilter = -1);
    /*!
    * \brief
    *    Returns a random item from a specific pool, optionally filtered by rarity.
    */
    Item GetRandomItem(std::string const& poolFilter, int rarityFilter = -1);
    /*!
    * \brief
    *    Adds an item to the player and applies its effects.
    */
    void AddItem(Item const& itemName, StatsManager& playerStats);
    /*!
    * \brief
    *    Removes an item from the active list by name.
    */
    Item PopItem(std::string const& itemName);
    /*!
    * \brief
    *    Removes an item from the active list by item.
    */
    Item PopItem(Item const& item);
    /*!
    * \brief
    *    Returns a const reference to the master item list.
    */
    std::unordered_map<std::string, Item> const& GetItemList() const;
    /*!
    * \brief
    *    Returns a const reference to the active items list.
    */
    std::list<Item> const& GetActiveItems() const;

    /*!
    * \brief
    *    Clears all active items.
    */
    void ClearActiveItems();


private:
    /*!
    * \brief
    *    Applies all effects of the given item to the provided StatsManager.
    *    Iterates through the item's effect list and calls ApplyEffect for each.
    *
    * \param playerStats
    *    Reference to the player's StatsManager that will be modified.
    * \param item
    *    The item whose effects are to be applied.
    *
    * \return
    *    Reference to the modified StatsManager (for chaining).
    */
    StatsManager& ApplyItemEffects(StatsManager& playerStats, Item const& item);

    /*!
    * \brief
    *    Applies a single effect to the StatsManager. Uses a cached lookup of
    *    effect handlers; if the effect name is not recognised, logs an error
    *    and does nothing.
    *
    * \param effect
    *    The effect to apply, containing a name, a multiplicative flag, and a value.
    * \param stats
    *    Reference to the player's StatsManager to modify.
    */
    void ApplyEffect(Item::Effect const& effect, StatsManager& stats);

    /*!
    * \brief
    *    Removes the given item from all item pools it belongs to, preventing it
    *    from being offered again in future selections. This is used for non‑stackable
    *    items or items that should be exhausted after being picked.
    *
    * \param item
    *    The item to exhaust.
    */
    void ExhaustItem(Item const& item);

    /*!
    * \brief
    *    Overload of ExhaustItem that takes an item name. Removes the named item
    *    from all pools.
    *
    * \param item
    *    The name of the item to exhaust.
    */
    void ExhaustItem(std::string const& item);

    /*!
    * \brief
    *    When an item has a non‑empty evolvesInto field, this function adds the
    *    evolved form to all pools that the original item belonged to. This makes
    *    the evolved item available for selection after the prerequisite is taken.
    *
    * \param item
    *    The item that triggers the evolution (must have a valid evolvesInto name).
    */
    void AddItemEvolution(Item const& item);


    std::random_device rd;

    std::unordered_map<std::string, std::function<void(bool, double)>> effectLookup;

    std::unordered_map<std::string, Item> itemList;

    std::unordered_map<std::string, std::unordered_map<std::string, Item>> itemPools;

    std::list<Item> activeItems;
};