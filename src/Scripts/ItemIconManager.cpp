/*!
@file       ItemIconScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Implements the ItemIconManager class. Handles UI display for
			displaying items collected in the game

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/
#include "ItemIconManager.h"
#include "../CoreLib/ItemManager.h"
void ItemIconManager::OnStart(Registry& )
{
};

// This function dynamically creates and positions icon GameObjects for newly active items in a 4-column grid layout.
void ItemIconManager::OnUpdate(Registry&, float, bool)
{
	auto& itemList = CEO::Get<ItemManager>()->GetActiveItems();
    // Only create new icon GameObjects for items that have been added since the last update.
    // itemGameObjects is a member vector that stores the icon for each active item.
    for (size_t i = itemGameObjects.size(); i < itemList.size(); i++) {
        // Instantiate a new icon from the prefab.
        GameObject itemObj = itemIconPrefab.Instantiate();
        itemGameObjects.push_back(itemObj);

        // Attach the new icon as a child of this manager's entity (for UI hierarchy/transform inheritance).
        GameObject{ this->entity }.SetChild(itemObj.GetEntityID());

        // Position the icon in a grid layout.
        UITransformComponent* transform = itemObj.GetComponent<UITransformComponent>();
        // 4 columns: each new icon shifts to the right by 1.15 times its width per column.
        transform->relativePos.x += (i % 4) * (transform->size.x * 1.15f);
        // Move upward (negative Y) for each full row of 4 items.
        transform->relativePos.y -= (i / 4) * (transform->size.y * 1.15f);

        // Retrieve the corresponding item data from the list.
        auto itemIt = itemList.begin();
        std::advance(itemIt, i);

        // Set the sprite texture based on the item's icon path.
        itemObj.GetComponent<SpriteRendererComponent>()->texture =
            &CEO::Get<ResourceManager>()->GetTexture("items/" + itemIt->iconPath + ".png");
    }
};

void ItemIconManager::OnFixedUpdate(Registry&, float, bool)
{
};
