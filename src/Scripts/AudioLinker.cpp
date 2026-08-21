/*!
@file       AudioLinker.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple script to link audio playing to a button

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "AudioLinker.h"
#include "../CoreLib/InputManager.h"
#include <functional>

void AudioLinker::OnStart(Registry& registry)
{
	ButtonComponent& button = *GetComponent<ButtonComponent>(registry);
	
	// binding a non-member/static function that takes parameters
	button.onClick = std::bind(		//bind hides the param of the function since onClick takes in a function that has no params
		&AudioLinker::PlayAudio,	// function name
		this						// member instance to call the play audio function
		);					
};
void AudioLinker::OnUpdate(Registry& ,float , bool ){/*empty by design*/ };
void AudioLinker::OnFixedUpdate(Registry& ,float , bool ){/*empty by design*/ }

void AudioLinker::PlayAudio()
{
	if (AudioComponent* ac = GetComponent<AudioComponent>(*CEO::Instance().GetManager<Registry>()); ac) {
		
		if (ac->audio) {
			LOGI("playing audio");
			ac->audio->Play(ac->volume, ac->pitch);
			return;
		}
	}
	LOGI("Audio component not found!!");
}
