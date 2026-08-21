/*!
@file       ItemManager.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Implements the ItemManager class, which loads item definitions from
            a JSON configuration file, manages item pools, handles random item
            selection, applies item effects to the player, and tracks active
            items. Also provides utility functions for item exhaustion and
            evolution.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#include "pch.h"
#include "ItemManager.h"
#include "CEO.h"
#include "Json.h"

#undef GetObject

// Default constructor; creates an invalid Item.
Item::Item() : name(), description(), iconPath(), type(), pool(), rarity(), effectList(), isStackable(), evolvesInto(), isEvolution(), itemsToRemove(),
isValid(false) {
};

// Parameterised constructor; creates a fully defined Item.
Item::Item(
    std::string const& __name,
    std::string const& __desc,
    std::string const& __iconPath,
    std::string const& __type,
    std::vector<std::string> const& __pool,
    int                                __rarity,
    std::vector<Effect> const& __effectList,
    bool                               __isStackable,
    std::string const& __evolvesInto,
    bool                               __isEvolution,
    std::vector<std::string> const& __itemsToRemove) :
    name(__name),
    description(__desc),
    iconPath(__iconPath),
    type(__type),
    pool(__pool),
    rarity(__rarity),
    effectList(__effectList),
    isStackable(__isStackable),
    evolvesInto(__evolvesInto),
    isEvolution(__isEvolution),
    isValid(true),
    itemsToRemove(__itemsToRemove)
{
};

// Returns whether the item contains valid data.
bool Item::IsValid() const {
    return isValid;
}

// Default constructor.
ItemManager::ItemManager() {

}

// Reads item configuration from JSON and populates item list and pools.
void ItemManager::Init()
{
    // Load config.json to get the path to the item data file.
#ifdef PLATFORM_ANDROID
    json jsonDoc("config.json");
#else
    json jsonDoc("Assets/config.json");
#endif
    // Validate that the ItemDataConfiguration path exists in config.json.
    if (!jsonDoc.GetValue("ItemDataConfiguration") ||
        !jsonDoc.GetValue("ItemDataConfiguration")->GetObj() ||
        !jsonDoc.GetValue("ItemDataConfiguration")->GetObj()->GetValue("path") ||
        !jsonDoc.GetValue("ItemDataConfiguration")->GetObj()->GetValue("path")->GetString()
        )
    {
        LOGE("No item config data in config.json");
        return;
    }
    std::string path = *jsonDoc.GetValue("ItemDataConfiguration")->GetObj()->GetValue("path")->GetString();

    // Open the item data file (ItemData.json).
    rapidjson::Document doc;
#ifdef PLATFORM_WINDOWS
    std::ifstream ifs("Assets/" + path);
#else
    std::stringstream ifs = CEO::Instance().GetManager<FileManager>()->ReadFile(path, false, std::ios_base::binary | std::ios_base::in);
#endif
    if (!ifs) {
        LOGE("ItemData.json cannot be opened");
        return;
    }

    // Read the entire file into a buffer and parse JSON.
    std::stringstream buffer;
    buffer << ifs.rdbuf();
#ifdef PLATFORM_WINDOWS
    ifs.close();
#endif
    doc.Parse(buffer.str());

    // Check for the required "ItemList" array.
    if (!doc.HasMember("ItemList") || !doc["ItemList"].IsArray()) {
        LOGE("ItemData.json is corrupted");
        return;
    };

    // Iterate over each item in the list.
    for (auto const& itemRaw : doc["ItemList"].GetArray()) {
        // Validate that all required fields are present and of correct type.
        if (itemRaw.HasMember("ItemName") && itemRaw["ItemName"].IsString() &&
            itemRaw.HasMember("ItemDescription") && itemRaw["ItemDescription"].IsString() &&
            itemRaw.HasMember("ItemIconPath") && itemRaw["ItemIconPath"].IsString() &&
            itemRaw.HasMember("ItemType") && itemRaw["ItemType"].IsString() &&
            itemRaw.HasMember("ItemPool") && itemRaw["ItemPool"].IsArray() &&
            itemRaw.HasMember("ItemRarity") && itemRaw["ItemRarity"].IsInt() &&
            itemRaw.HasMember("ItemEffects") && itemRaw["ItemEffects"].IsArray() &&
            itemRaw.HasMember("IsStackable") && itemRaw["IsStackable"].IsBool() &&
            itemRaw.HasMember("EvolvesInto") && itemRaw["EvolvesInto"].IsString() &&
            itemRaw.HasMember("Evolution") && itemRaw["Evolution"].IsBool() &&
            itemRaw.HasMember("ItemsToRemove") && itemRaw["ItemsToRemove"].IsArray()
            ) {
            // Create a temporary Item with basic fields (pools and effects will be filled next).
            Item temp{
                    itemRaw["ItemName"].GetString(),
                    itemRaw["ItemDescription"].GetString(),
                    itemRaw["ItemIconPath"].GetString(),
                    itemRaw["ItemType"].GetString(),
                    std::vector <std::string> {},          // empty pool for now
                    itemRaw["ItemRarity"].GetInt(),
                    std::vector<Item::Effect>{},           // empty effect list for now
                    itemRaw["IsStackable"].GetBool(),
                    itemRaw["EvolvesInto"].GetString(),
                    itemRaw["Evolution"].GetBool(),
                    std::vector<std::string>{}             // empty itemsToRemove for now
            };

            // Fill the pool vector from the JSON array.
            for (auto const& poolName : itemRaw["ItemPool"].GetArray()) {
                if (poolName.IsString()) {
                    temp.pool.push_back(poolName.GetString());
                }
                else {
                    LOGE("%s item has an invalid pool name datatype", temp.name.c_str());
                }
            }

            // Fill the effect list from the JSON array.
            for (auto const& effectRaw : itemRaw["ItemEffects"].GetArray()) {
                // Validate each effect object.
                if (effectRaw.IsObject() &&
                    effectRaw.HasMember("Effect") && effectRaw["Effect"].IsString() &&
                    effectRaw.HasMember("IsMultiplicative") && effectRaw["IsMultiplicative"].IsBool() &&
                    effectRaw.HasMember("EffectValue") && effectRaw["EffectValue"].IsNumber()) {
                    // Handle EffectValue as either double or int.
                    if (effectRaw["EffectValue"].IsDouble())
                        temp.effectList.push_back(
                            Item::Effect{
                                    effectRaw["Effect"].GetString(),
                                    effectRaw["IsMultiplicative"].GetBool(),
                                    effectRaw["EffectValue"].GetDouble()
                            }
                        );
                    else {
                        temp.effectList.push_back(
                            Item::Effect{
                                    effectRaw["Effect"].GetString(),
                                    effectRaw["IsMultiplicative"].GetBool(),
                                    static_cast<double>(effectRaw["EffectValue"].GetInt())
                            }
                        );
                    }
                }
                else {
                    LOGE("%s item has an invalid effect in item file", temp.name.c_str());
                }
            }

            // Fill the itemsToRemove vector from the JSON array.
            for (auto const& removeItemRaw : itemRaw["ItemsToRemove"].GetArray()) {
                if (removeItemRaw.IsString()) {
                    temp.itemsToRemove.push_back(removeItemRaw.GetString());
                }
                else {
                    LOGE("%s item has an invalid item to remove datatype", temp.name.c_str());
                }
            }

            // Insert the completed item into the master item list.
            itemList.emplace(temp.name, temp);

            // Ensure that each pool name exists in itemPools (create empty map if not).
            for (std::string const& poolName : temp.pool) {
                if (itemPools.find(poolName) == itemPools.end()) {
                    itemPools[poolName] = std::unordered_map<std::string, Item>{};
                };
            }
        }
        else if (itemRaw.HasMember("ItemName") && itemRaw["ItemName"].IsString()) {
            // Item has a name but other fields are corrupted.
            LOGE("%s item data in item file is corrupted", itemRaw["ItemName"].GetString());
        }
        else {
            // Completely malformed item entry.
            LOGE("Unknown item in item file is corrupted");
        }
    };
    // After loading all items, initialise the pools (fill them with non?evolution items).
    InitItemPool();
}

// Resets item pools and clears active items.
void ItemManager::ResetItems() {
    InitItemPool();      // Rebuild pools from master list
    activeItems.clear(); // Clear any active items
}

// Rebuilds item pools from the master item list (excluding evolution items).
void ItemManager::InitItemPool() {
    // Clear all existing pools.
    for (auto& pool : itemPools) {
        pool.second.clear();
    }
    // Iterate over all items and add only non?evolution items to their respective pools.
    for (auto& [name, item] : itemList) {
        if (!item.isEvolution) {
            for (auto const& poolName : item.pool) {
                itemPools[poolName].emplace(item.name, item);
            }
        }
    }
}

// Returns a random item from a specific pool, optionally filtered by rarity.
Item ItemManager::GetRandomItem(std::string const& poolFilter, int rarityFilter) {
    std::vector<Item> filteredItems;
    // Find the requested pool.
    if (auto filteredPool = itemPools.find(poolFilter);
        filteredPool != itemPools.end()) {
        // Iterate through items in that pool and apply rarity filter.
        for (auto& [name, item] : filteredPool->second) {
            if (rarityFilter == -1 || item.rarity == rarityFilter) {
                filteredItems.push_back(item);
            }
        }
    }
    // If no items match, log error and return invalid Item.
    if (filteredItems.empty()) {
        if (rarityFilter != -1) {
            LOGE("No items found matching pool %s and rarity %d", poolFilter.c_str(), rarityFilter);
        }
        else {
            LOGE("No items found matching pool %s", poolFilter.c_str());
        }
        return Item{};
    }

    // Select a random item from the filtered list.
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, static_cast<int>(filteredItems.size() - 1));
    return filteredItems[dis(gen)];
}

// Returns a random item from the entire item list, optionally filtered by rarity.
Item ItemManager::GetRandomItem(int rarityFilter) {
    std::vector<Item> filteredItems;
    // Iterate through all items and apply rarity filter.
    for (auto& [name, item] : itemList) {
        if (rarityFilter == -1 || item.rarity == rarityFilter) {
            filteredItems.push_back(item);
        }
    }
    // If no items match, log error and return invalid Item.
    if (filteredItems.empty()) {
        if (rarityFilter != -1) {
            LOGE("No items found matching rarity %d", rarityFilter);
        }
        else {
            LOGE("No items found in item list");
        }
        return Item{};
    }

    // Select a random item.
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, static_cast<int>(filteredItems.size() - 1));
    return filteredItems[dis(gen)];
}

// Adds an item to the player, applies its effects, and handles exhaustion/evolution/removal.
void ItemManager::AddItem(Item const& itemInst, StatsManager& playerStats) {
    // Verify that the item is valid and exists in the master list.
    if (itemInst.IsValid() && itemList.find(itemInst.name) != itemList.end()) {
        // Apply the item's effects to the player's stats.
        ApplyItemEffects(playerStats, itemInst);
        // Add to active items list.
        activeItems.push_back(itemInst);
        // If item is not stackable, remove it from all pools (exhaust).
        if (!itemInst.isStackable) {
            ExhaustItem(itemInst);
        }
        // If the item evolves into another, add the evolved form to pools.
        if (!itemInst.evolvesInto.empty()) {
            AddItemEvolution(itemInst);
        }
        // Remove any items specified in itemsToRemove from pools.
        for (std::string const& itemToRemove : itemInst.itemsToRemove) {
            ExhaustItem(itemToRemove);
        }
    }
    else {
        LOGE("No item %s exists in item list", itemInst.name.c_str());
        return;
    }
}

// Removes the given item from all pools (exhausts it).
void ItemManager::ExhaustItem(Item const& item) {
    // For each pool the item belongs to, erase it.
    for (std::string const& poolName : item.pool) {
        if (auto pool = itemPools.find(poolName);
            pool != itemPools.end()) {
            pool->second.erase(item.name);
        }
        else {
            LOGE("Item %s is to be exhausted but is not found in item pool %s, is this intentional?", item.name.c_str(), poolName.c_str());
        }
    }
}

// Overload: removes the named item from all pools.
void ItemManager::ExhaustItem(std::string const& item) {
    // Iterate through all pools and erase the item by name.
    for (auto& [poolName, pool] : itemPools) {
        if (!pool.erase(item)) {
            LOGE("Item %s is to be exhausted but is not found in item pool %s, is this intentional?", item.c_str(), poolName.c_str());
        }
    }
}

// When an item has a non?empty evolvesInto, adds the evolved form to all pools of the original.
void ItemManager::AddItemEvolution(Item const& item) {
    // Find the evolved item in the master list.
    if (auto const search = itemList.find(item.evolvesInto);
        search != itemList.end()) {
        // For each pool of the original item, insert the evolved item.
        for (std::string const& poolName : item.pool) {
            if (auto pool = itemPools.find(poolName);
                pool != itemPools.end()) {
                pool->second.insert(*search);
            }
            else {
                LOGE("Item pool %s in item %s could not be found and was not added to that pool", poolName.c_str(), item.name.c_str());
            }
        }

    }
    else {
        LOGE("Item %s is to evolve into %s but the evolved form is not found in item list, is this intentional?", item.name.c_str(), item.evolvesInto.c_str());
        return;
    }
}

// Removes and returns the first active item with the given name.
Item ItemManager::PopItem(std::string const& itemName) {
    auto it = activeItems.begin();
    while (it != activeItems.end()) {
        if (it->name == itemName) {
            Item ret = *it;
            activeItems.erase(it);
            return ret;
        }
        std::advance(it, 1);
    }
    return Item{}; // Not found
}

// Overload: removes and returns the first active item matching the provided item.
Item ItemManager::PopItem(Item const& item) {
    auto it = activeItems.begin();
    while (it != activeItems.end()) {
        if (it->name == item.name) {
            Item ret = *it;
            activeItems.erase(it);
            return ret;
        }
        std::advance(it, 1);
    }
    return Item{};
}

// Returns a const reference to the list of active items.
std::list <Item> const& ItemManager::GetActiveItems() const {
    return activeItems;
}

// Returns a const reference to the master item list.
std::unordered_map<std::string, Item> const& ItemManager::GetItemList() const {
    return itemList;
}

// Clears the active items list.
void ItemManager::ClearActiveItems() {
    activeItems.clear();
}

// Applies all effects of the given item to the StatsManager.
StatsManager& ItemManager::ApplyItemEffects(StatsManager& stats, Item const& item) {
    for (Item::Effect const& effect : item.effectList) {
        ApplyEffect(effect, stats);
    }
    return stats;
}

// Applies a single effect using a cached handler; creates handler if not yet cached.
void ItemManager::ApplyEffect(Item::Effect const& effect, StatsManager& stats) {
    // If this effect name hasn't been seen before, create a handler lambda.
    if (effectLookup.find(effect.name) == effectLookup.end()) {
        if (effect.name == "maxHealth") {
            // Special handling for maxHealth: also adjust current health.
            effectLookup[effect.name] = [&stats](bool isMultiplicative, double effectValue) {
                int hpBefore = stats.maxHealth.Get();
                if (isMultiplicative) {
                    stats.maxHealth.AddMultiplier(static_cast<float>(effectValue));
                }
                else {
                    stats.maxHealth.AddOffset(static_cast<int>(effectValue));
                }
                // Increase current health by the same amount maxHealth increased.
                if (hpBefore > stats.maxHealth.Get()) {
                    if(stats.health.GetNetValue() > stats.maxHealth.Get()){
                        stats.health.Curr(stats.maxHealth.Get());
					}
                }
                else {
                    stats.health += stats.maxHealth.Get() - hpBefore;
                }
                };
        }
        else if (stats.GetByName(effect.name)) {
            // Generic handler for stats that exist in StatsManager.
            effectLookup[effect.name] = [&stats, name = effect.name](bool isMultiplicative, double effectValue) {
                if (isMultiplicative) {
                    *stats.GetByName(name) *= static_cast<float>(effectValue);
                }
                else {
                    *stats.GetByName(name) += static_cast<float>(effectValue);
                }
                };
        }
        else {
            // No matching stat; log error and create a no?op handler.
            effectLookup[effect.name] = [name = effect.name](bool, double) {
                LOGE("Effect %s does not have a corresponding stat in StatsManager, this effect will be ignored", name.c_str());
                };
        }
    }
    // Execute the cached handler.
    effectLookup[effect.name](effect.isMultiplicative, effect.effectValue);
}