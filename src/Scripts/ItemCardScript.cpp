/*!
@file       ItemCardScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Implements the ItemCardScript class. Handles UI interactions for
			item cards, including hover effects, click handling, item application,
			and visual updates of the item icon and description.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#include "ItemCardScript.h"
#include "HealthbarScript.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/AchievementManager.h"
#include "DialogueScript.h"

// Retrieves components, sets hover callbacks, resets timer, and loads item data.
void ItemCardScript::OnStart(Registry& r)
{
	// Get the button and sprite components attached to this entity
	buttonComp = this->GetComponent<ButtonComponent>(r);
	spriteComp = this->GetComponent<SpriteRendererComponent>(r);

	// If a button component exists, bind hover enter/exit callbacks
	if (buttonComp) {
		// Use std::bind to capture the current script instance (retrieved again via GetComponent)
		buttonComp->onHoverEnter = std::bind(&ItemCardScript::onHover, this->GetComponent<ItemCardScript>(r));
		buttonComp->onHoverExit = std::bind(&ItemCardScript::onHoverExit, this->GetComponent<ItemCardScript>(r));
	}
	// Reset the timer used for click delay
	timeElapsed = 0;
	// Load textures and set up the item display
	UpdateScriptParams(r);
};

// Per‑frame update; empty as no per‑frame logic needed.
void ItemCardScript::OnUpdate(Registry&, float, bool)
{
	// Intentionally empty
};

// Accumulates time and enables click callback after delay.
void ItemCardScript::OnFixedUpdate(Registry& r, float dt, bool)
{
	// Increase timer
	timeElapsed += dt;
	// If the delay has passed and we haven't already set the click callback
	if (timeElapsed > clickDelay) {
		// Bind the onClick method to the button's click event
		if (buttonComp) buttonComp->onClick = std::bind(&ItemCardScript::onClick, this->GetComponent<ItemCardScript>(r));
		// Reset clickDelay to zero so this block won't run again
		clickDelay = 0;
	}
};

// Handles card click: plays sound, adds item, updates stats, closes menu, refreshes healthbars.
void ItemCardScript::onClick() {
	// Play item pickup sound effect
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\ItemPicked.wav").Play(0.5f, 1.0f, 1.0f);

	// If the item is of type "Projectile", update the achievement tracker for item collection
	if (item.type == "Projectile")
	{
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ITEM");
	}

	// Get references to the stats manager and registry
	StatsManager* stats = CEO::Get<StatsManager>();
	Registry* reg = CEO::Get<Registry>();

	// Event to call Dialogue
	for (Registry::Entity ent : reg->GetEntitiesWithComponent<DialogueScript>()) {
		reg->GetComponent<DialogueScript>(ent)->Trigger("ITEM", item.name);
	}

	// Change the sprite to the hover texture (visual feedback)
	if (spriteComp && hoverTex) {
		spriteComp->texture = hoverTex;
	}

	// Add the item to the player's inventory and apply its effects
	CEO::Get<ItemManager>()->AddItem(item, *stats);

	// Calculate overflow: difference between current rarity and the item's rarity
	int overflow = 0;
	if (stats->GetRarity() != rarity) {
		overflow = stats->GetRarity() - rarity;
	}

	// Reset the player's luck and deposit any overflow (extra luck from rarity difference)
	stats->ResetLuck();
	stats->DepositLuck(overflow);

	// Debug output of current stats (presumably for development)
	stats->Debug();

	// Close the current item menu scene (pop it from the scene stack)
	CEO::Get<SceneManager>()->QueueSceneAction("", SceneManager::POP);

	// Update all health bars in the scene to reflect the player's new health values
	for (Registry::Entity ent : reg->GetEntitiesWithComponent<HealthbarScript>()) {
		reg->GetComponent<HealthbarScript>(ent)->UpdateHealthbar(stats->health.Curr(), stats->maxHealth.Get());
	}
}

// Handles hover enter: plays sound, changes texture, scales up.
void ItemCardScript::onHover() {
	// Play UI hover sound
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav").Play();

	// Change texture to hover version if available
	if (spriteComp && hoverTex) {
		spriteComp->texture = hoverTex;

		// Get the UI transform component and increase its size for a "pop" effect
		if (UITransformComponent* comp = GetComponent<UITransformComponent>(*CEO::Get<Registry>());
			comp) {
			comp->size.x += 25.f;
			comp->size.y += 25.f;
		}
	}
}

// Handles hover exit: restores texture and scale.
void ItemCardScript::onHoverExit() {
	// Restore the normal background texture if available
	if (spriteComp && cardBackgroundTex) {
		spriteComp->texture = cardBackgroundTex;

		// Get the UI transform component and reduce its size back to original
		if (UITransformComponent* comp = GetComponent<UITransformComponent>(*CEO::Get<Registry>());
			comp) {
			comp->size.x -= 25.f;
			comp->size.y -= 25.f;
		}
	}
}

// Loads textures, sets background, retrieves random item, updates icon and description.
void ItemCardScript::UpdateScriptParams(Registry& r) {
	// Load textures from resource manager using the paths set in the inspector
	cardBackgroundTex = &CEO::Get<ResourceManager>()->GetTexture(cardBackgroundPath);
	hoverTex = &CEO::Get<ResourceManager>()->GetTexture(hoverTexPath);

	// Set the sprite to the background texture (default state)
	if (spriteComp) {
		spriteComp->texture = cardBackgroundTex;
	}

	// Get a random item from the item pool matching the configured rarity and pool
	item = CEO::Get<ItemManager>()->GetRandomItem(itemPool, rarity);

	// If a valid item was retrieved, update the child objects to display its icon and description
	if (item.IsValid()) {
		// Update the icon sprite on the itemObject
		if (SpriteRendererComponent* comp = r.GetComponent<SpriteRendererComponent>(itemObject.GetEntityID());
			comp) {
			// Construct the full texture path using the item's iconPath
			comp->texture = &CEO::Get<ResourceManager>()->GetTexture("items/" + item.iconPath + ".png");
		}

		// Update the description text on the itemDescObject
		if (TextRendererComponent* comp = r.GetComponent<TextRendererComponent>(itemDescObject.GetEntityID());
			comp) {
			comp->text = item.description;
		}
	}
}