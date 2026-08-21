/*!
@file       ItemMenuManager.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Implements the ItemMenuManager class, which orchestrates the item
			selection menu. It configures the item, heal, and luck cards, and
			controls their staggered appearance over time.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#include "ItemMenuManager.h"
#include "ItemCardScript.h"
#include "LuckItemScript.h"

// Initialises item cards and luck button with correct textures and delays.
void ItemMenuManager::OnStart(Registry& r)
{
	// Initialise the item and heal cards based on current player rarity.
	InitItems(CEO::Get<StatsManager>()->GetRarity());

	// Configure the luck button if it exists.
	if (luckButton.IsValid()) {
		LuckItemScript* luck = luckButton.GetComponent<LuckItemScript>();
		if (luck) {
			// Set click delay and texture paths (luck card uses generic non?item textures).
			luck->clickDelay = popupTime;
			luck->cardBackgroundPath = cardNon;
			luck->hoverTexPath = cardNonHover;
			luck->UpdateScriptParams(r);
		}
	}
	// Reset the timer used for staggered activation.
	timeElapsed = 0.f;
};

// Per?frame update: enables cards in sequence based on popupTime.
void ItemMenuManager::OnUpdate(Registry&, float dt, bool)
{
	// Enable luck card after full popupTime.
	if (luckButton.IsValid() && timeElapsed > popupTime) {
		if (ActiveComponent* comp = luckButton.GetComponent<ActiveComponent>();
			comp) {
			comp->isActiveSelf = true;
		}
	}
	// Enable heal card after 2/3 of popupTime.
	if (healButton.IsValid() && timeElapsed > popupTime / 3 * 2) {
		if (ActiveComponent* comp = healButton.GetComponent<ActiveComponent>();
			comp) {
			comp->isActiveSelf = true;
		}
	}
	// Enable item card after 1/3 of popupTime.
	if (itemButton.IsValid() && timeElapsed > popupTime / 3) {
		if (ActiveComponent* comp = itemButton.GetComponent<ActiveComponent>();
			comp) {
			comp->isActiveSelf = true;
		}
	}
	// Accumulate elapsed time.
	timeElapsed += dt;
};

// Fixed update; no logic needed.
void ItemMenuManager::OnFixedUpdate(Registry&, float, bool)
{
};

// Configures the item and heal cards with appropriate textures based on rarity.
void ItemMenuManager::InitItems(int rarity) {
	// Configure the main item button.
	if (itemButton.IsValid()) {
		ItemCardScript* item = itemButton.GetComponent<ItemCardScript>();
		if (item) {
			// Set common properties.
			item->clickDelay = popupTime;
			item->rarity = rarity;
			// Choose textures based on rarity.
			switch (rarity) {
			case 1:
				item->cardBackgroundPath = card1;
				item->hoverTexPath = card1Hover;
				break;
			case 2:
				item->cardBackgroundPath = card2;
				item->hoverTexPath = card2Hover;
				break;
			case 3:
				item->cardBackgroundPath = card3;
				item->hoverTexPath = card3Hover;
				break;
			case 4:
				item->cardBackgroundPath = card4;
				item->hoverTexPath = card4Hover;
				break;
			default:
				// Fallback to generic non?item card if rarity out of range.
				item->cardBackgroundPath = cardNon;
				item->hoverTexPath = cardNonHover;
			}
			// Apply the new parameters (load textures and pick a random item).
			item->UpdateScriptParams(*CEO::Get<Registry>());
		}
	}

	// Configure the heal button (if present).
	if (healButton.IsValid()) {
		ItemCardScript* heal = healButton.GetComponent<ItemCardScript>();
		if (heal) {
			heal->clickDelay = popupTime;
			// Heal rarity is clamped between 1 and 3.
			heal->rarity = std::clamp(rarity, 1, 3);
			switch (heal->rarity) {
			case 1:
				heal->cardBackgroundPath = card1;
				heal->hoverTexPath = card1Hover;
				break;
			case 2:
				heal->cardBackgroundPath = card2;
				heal->hoverTexPath = card2Hover;
				break;
			case 3:
				heal->cardBackgroundPath = card3;
				heal->hoverTexPath = card3Hover;
				break;
			default:
				heal->cardBackgroundPath = cardNon;
				heal->hoverTexPath = cardNonHover;
			}
			heal->UpdateScriptParams(*CEO::Get<Registry>());
		}
	}
}