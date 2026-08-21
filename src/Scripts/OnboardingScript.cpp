/**___________________________________________________________________________/
@file       OnboardingScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       08/03/2026   (DD/MM/YYYY)
@brief      Script that handles the onboarding tutorial overlay displayed
			on the player's first run. Cycles through a series of instructional
			images on click and hides itself once all slides have been shown.
			Skipped automatically on subsequent runs.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "OnboardingScript.h"
#include "../CoreLib/AchievementManager.h"
void OnboardingScript::OnStart(Registry & r)
{
	// skip onboarding entirely if this is not the player's first run
	if (CEO::Instance().GetManager<AchievementManager>()->firstPlay == false)
	{
		GetComponent<ActiveComponent>(r)->isActiveSelf = false;
		return;
	}
	index = 1;
	timer = 0.0f;
	auto button = GetComponent<ButtonComponent>(r);
	button->onClick = std::bind(&OnboardingScript::Next, GetComponent<OnboardingScript>(r));
	CEO::Instance().GetManager<AchievementManager>()->firstPlay = false;
};


void OnboardingScript::OnUpdate(Registry & , float dt, bool )
{
	timer += dt;
#ifdef PLATFORM_WINDOWS
	if (Input::IsGamepadConnected(0) && Input::IsGamepadButtonPressed(Input::GamepadButton::A))
	{
		Next();
	}
#endif
};

void OnboardingScript::OnFixedUpdate(Registry&, float, bool)
{
};

void OnboardingScript::Next()
{
	// ignore clicks within the first 0.5 seconds to prevent accidental skips
	if (timer <= 0.5f) return;
	Registry* registry = CEO::Get<Registry>();
	index++;
	if (index >= 7)
	{
		registry->GetComponent<ActiveComponent>(entity)->isActiveSelf = false;
		return;
	}
	std::string path = "Select your tile here\\" + std::to_string(index) + ".png";
	registry->GetComponent<SpriteRendererComponent>(entity)->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(path);
}
