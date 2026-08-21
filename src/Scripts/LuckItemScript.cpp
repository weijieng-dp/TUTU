/*!
@file       LuckItemScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       11/03/2026

@brief      Implements the LuckItemScript class, which controls the behaviour of a
			luck‑granting item card in the item selection menu. Handles hover
			effects, click delay, and the addition of luck to the player's stats.
			The script also manages visual feedback (texture changes, scaling)
			and plays corresponding audio cues.

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/

#include "LuckItemScript.h"
#include "../CoreLib/SceneManager.h"

// Retrieves button and sprite components, sets hover callbacks, and updates textures.
void LuckItemScript::OnStart(Registry& r)
{
	buttonComp = this->GetComponent<ButtonComponent>(r);
	spriteComp = this->GetComponent<SpriteRendererComponent>(r);

	if (buttonComp) {
		buttonComp->onHoverExit = std::bind(&LuckItemScript::onHoverExit, this->GetComponent<LuckItemScript>(r));
		buttonComp->onHoverEnter = std::bind(&LuckItemScript::onHover, this->GetComponent<LuckItemScript>(r));
	}

	UpdateScriptParams(r);
};

// Per‑frame update; empty as no per‑frame logic needed.
void LuckItemScript::OnUpdate(Registry&, float, bool)
{
};

// Accumulates time and enables click callback after the delay.
void LuckItemScript::OnFixedUpdate(Registry& r, float dt, bool)
{
	timeElapsed += dt;
	if (clickDelay && timeElapsed > clickDelay) {
		if (buttonComp) buttonComp->onClick = std::bind(&LuckItemScript::onClick, this->GetComponent<LuckItemScript>(r));
		clickDelay = 0;
	}
};

// Handles card click: plays sound, changes texture, deposits luck, closes menu.
void LuckItemScript::onClick() {
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\ItemPickUp.wav").Play();
	if (spriteComp && hoverTex) {
		spriteComp->texture = hoverTex;
	}
	CEO::Get<StatsManager>()->DepositLuck(luckAdded);

	CEO::Get<SceneManager>()->QueueSceneAction("", SceneManager::POP);
}

// Handles hover enter: plays sound, changes texture, scales up.
void LuckItemScript::onHover() {
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav").Play();
	if (spriteComp && hoverTex) {
		spriteComp->texture = hoverTex;
		if (UITransformComponent* comp = GetComponent<UITransformComponent>(*CEO::Get<Registry>());
			comp) {
			comp->size.x += 25.f;
			comp->size.y += 25.f;
		}
	}
}

// Handles hover exit: restores texture and scale.
void LuckItemScript::onHoverExit() {
	if (spriteComp && cardBackgroundTex) {
		spriteComp->texture = cardBackgroundTex;
		if (UITransformComponent* comp = GetComponent<UITransformComponent>(*CEO::Get<Registry>());
			comp) {
			comp->size.x -= 25.f;
			comp->size.y -= 25.f;
		}
	}
}

// Loads card background and hover textures, and sets the background.
void LuckItemScript::UpdateScriptParams(Registry&) {
	cardBackgroundTex = &CEO::Get<ResourceManager>()->GetTexture(cardBackgroundPath);
	hoverTex = &CEO::Get<ResourceManager>()->GetTexture(hoverTexPath);

	if (spriteComp) {
		spriteComp->texture = cardBackgroundTex;
	}
}