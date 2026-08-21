/**___________________________________________________________________________/
@file          SimpleAudioScript.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Basic audio scripting used to test the audio object and dragdrop functionality

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "SimpleAudioScript.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/InputManager.h"
#include "../CoreLib/ResourceManager.h"
#include "../CoreLib/AudioManager.h"

void SimpleAudioScript::OnStart(Registry& registry) { (void)registry; }
void SimpleAudioScript::OnUpdate(Registry& registry, float dt, bool firstframe) {
	(void)dt; (void)firstframe;
#ifdef PLATFORM_WINDOWS
	if (AudioComponent* ac = GetComponent<AudioComponent>(registry); ac) {
		if (Input::IsKeyPressed(GLFW_KEY_F10)) {
			if (ac->audio->IsActive()) {
				ac->audio->Play();
			}
		}
	}
#endif
}
void SimpleAudioScript::OnFixedUpdate(Registry& registry, float dt, bool firstframe) { (void)registry; (void)dt; (void)firstframe; }