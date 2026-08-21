/**___________________________________________________________________________/
@file       LandingPageScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       05/03/2026   (DD/MM/YYYY)
@brief      Script attached to the landing page scene. Handles BGM playback
			on start, triggers a fade to black on mouse click to transition
			to the next scene, and provides a debug shortcut to fast-forward
			achievement tracking and jump to the achievement scene.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "LandingPageScript.h"
#include "FadeScript.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/SceneManager.h"

void LandingPageScript::OnStart(Registry &)
{
	CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
};


void LandingPageScript::OnUpdate(Registry &, float, bool)
{
	// debug shortcut: on PC press grave accent, on mobile tap to fast-forward
	// all achievement trackers and jump directly to the achievement scene
	#ifdef PLATFORM_WINDOWS

	if (Input::IsKeyPressed(GLFW_KEY_GRAVE_ACCENT))
	{
		#else
    if(Input::IsPointerPressed(0))
	{
#endif
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ENEMY");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("ITEM");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("WIN");
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("WIN");
		SceneManager::QueueSceneAction("Achievement", SceneManager::CHANGE);
	}
	#ifdef PLATFORM_WINDOWS
	if (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || Input::IsGamepadButtonPressed(Input::GamepadButton::A,0))
#else
    if(Input::IsPointerPressed(0))
#endif
	{
		fadeGameObject.GetComponent<FadeScript>()->FadeToBlack();
	}
};

void LandingPageScript::OnFixedUpdate(Registry&, float, bool)
{
};
