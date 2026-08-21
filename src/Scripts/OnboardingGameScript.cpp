/**___________________________________________________________________________/
@file       OnboardingGameScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       26/03/2026   (DD/MM/YYYY)
@brief      Script that handles the onboarding tutorial overlay displayed
			on the player's first run. Cycles through a series of instructional
			images on click and hides itself once all slides have been shown.
			Skipped automatically on subsequent runs.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "OnboardingGameScript.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/MapManager.h"
#include "DialogueScript.h"

void OnboardingGameScript::OnStart(Registry & r)
{
	
	std::string path = "";

#ifdef PLATFORM_WINDOWS
		path = "Select your tile here 2\\Windows.png";
#endif // PLATFORM_WINDOWS
		
#ifdef PLATFORM_ANDROID
		path = "Select your tile here 2\\Android.png";
#endif // PLATFORM_ANDROID
	
		r.GetComponent<SpriteRendererComponent>(entity)->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(path);

	// skip onboarding entirely if this is not the player's first run

	if (CEO::Instance().GetManager<MapManager>()->GetTileInstanceSize() != 4)
	{
		GetComponent<ActiveComponent>(r)->isActiveSelf = false;
		isActive = false;
		return;
	}
	timer = 0.0f;
	auto button = GetComponent<ButtonComponent>(r);
	button->onClick = std::bind(&OnboardingGameScript::Next, GetComponent<OnboardingGameScript>(r));
	//CEO::Instance().GetManager<AchievementManager>()->firstPlay = false;
};


void OnboardingGameScript::OnUpdate(Registry & , float dt, bool )
{
	if(isActive) timer += dt;
#ifdef PLATFORM_WINDOWS
	if (Input::IsGamepadConnected(0) && Input::IsGamepadButtonPressed(Input::GamepadButton::A))
	{
		Next();
	}
#endif
};

void OnboardingGameScript::OnFixedUpdate(Registry&, float, bool)
{
};

void OnboardingGameScript::Next()
{
	// ignore clicks within the first 0.5 seconds to prevent accidental skips
	if (timer <= 0.5f) return;
	Registry* registry = CEO::Get<Registry>();
	{
		for (Registry::Entity ent : registry->GetEntitiesWithComponent<DialogueScript>()) {
			registry->GetComponent<DialogueScript>(ent)->Trigger("ENTERDUNGEON");
			break;
		}
		registry->GetComponent<ActiveComponent>(entity)->isActiveSelf = false;
		isActive = false;
		return;
	}
}
