/*!
@file       PostProcessToggleScript.cpp
@author     Ou Yukang (yukang.ou)
@date       25/03/2026
@brief		UI script to toggle post processing

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "PostProcessToggleScript.h"
#include "../CoreLib/UserSettingsManager.h"
#include "../CoreLib/EventsDispatcher.h"
#include <functional>

void PostProcessToggleScript::OnStart(Registry & r)
{
    UserSettingsManager* settings = CEO::Get<UserSettingsManager>();
    if(settings->settings.HasMember("postProcessFlag"))
        isOn = settings->settings["postProcessFlag"].GetBool();

    GetComponent<SpriteRendererComponent>(r)->texture = isOn ? onTex : offTex;
    GetComponent<ButtonComponent>(r)->onClick = std::bind(&PostProcessToggleScript::ToggleState, this);
};


void PostProcessToggleScript::OnUpdate(Registry&, float, bool)
{
};

void PostProcessToggleScript::OnFixedUpdate(Registry&, float, bool)
{
};

void PostProcessToggleScript::ToggleState()
{
    PostProcessToggleScript* toggle = GetComponent<PostProcessToggleScript>(*CEO::Get<Registry>());
    // toggle state
    toggle->isOn = !toggle->isOn;
    CEO::Get<EventsDispatcher>()->Dispatch<Events::TogglePostProcessEvent>(Events::TogglePostProcessEvent{toggle->isOn});
    UserSettingsManager* settings = CEO::Get<UserSettingsManager>();
    // set values and save to device
    settings->SetBool("postProcessFlag", toggle->isOn);
    settings->SaveSettings();

    // update sprite
    GetComponent<SpriteRendererComponent>(*CEO::Get<Registry>())->texture = toggle->isOn ? onTex : offTex;
}