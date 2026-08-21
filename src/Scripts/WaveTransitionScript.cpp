/**___________________________________________________________________________/
@file       WaveTransitionScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       01/03/2026   (DD/MM/YYYY)
@brief      Script that handles the transition between dungeon and wave scene.
			Waits for a specified duration, then triggers a fade-to-black
			effect on a target FadeScript object.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "WaveTransitionScript.h"
#include "FadeScript.h"
void WaveTransitionScript::OnStart(Registry &)
{
};


void WaveTransitionScript::OnUpdate(Registry & registry, float dt, bool)
{
	// Increment the timer by the time elapsed since the last frame
	timer += dt;

	// Once the timer reaches the configured duration, trigger the fade effect
	if (timer >= duration) {
		registry.GetComponent<FadeScript>(fadeObj.GetEntityID())->FadeToBlack();
	}
};

void WaveTransitionScript::OnFixedUpdate(Registry&, float, bool)
{
};
