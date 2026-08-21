/*!
@file       AudioSliderScript.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Slider to control volume of audio in game

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "AudioSliderScript.h"
#include "../CoreLib/InputManager.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Registry.h"
#include "../CoreLib/Camera.h"
#include "../CoreLib/ScreenManager.h"
#include "../CoreLib/AudioManager.h"
#include <functional>

void AudioSliderScript::OnStart(Registry& registry)
{
	auto entities = registry.GetEntitiesWithComponent<NameComponent>();
	NameComponent* name = nullptr;
	(void*)name;
	AudioSliderScript* slider = GetComponent<AudioSliderScript>(registry);
	ButtonComponent* buttonA = handle.GetComponent<ButtonComponent>();
	offset = 0;

	// binding a non-member/static function that takes parameters
	buttonA->onHeld = std::bind(		//bind hides the param of the function since onClick takes in a function that has no params
		&AudioSliderScript::UpdateSlider,	// function name
		GetComponent<AudioSliderScript>(*CEO::Get<Registry>()));

	buttonA->onClick = std::bind(&AudioSliderScript::ResetMouseX, GetComponent<AudioSliderScript>(*CEO::Get<Registry>()));
	buttonA->onHoverEnter = std::bind(&AudioSliderScript::ResetMouseX, GetComponent<AudioSliderScript>(*CEO::Get<Registry>()));
	buttonA->onHoverExit = std::bind(&AudioSliderScript::ResetMouseXHoverExit, GetComponent<AudioSliderScript>(*CEO::Get<Registry>()));

	GameObject go(entity);
	for (auto& ahandle : go.GetChildrenWithComponent<ButtonComponent>())
	{
		ahandle.GetComponent<SpriteRendererComponent>()->texture = &CEO::Get<ResourceManager>()->GetTexture("ui/menus/slidericon_nowhite.png");
	}

	SetSliderValue(CEO::Instance().GetManager<ResourceManager>()->GetGroupVolume(slider->audioGroup) / maxVolume);
};



void AudioSliderScript::OnUpdate(Registry& ,float , bool ){/*empty by design*/ };
void AudioSliderScript::OnFixedUpdate(Registry& ,float , bool ){/*empty by design*/ }
void AudioSliderScript::SetSliderValue(float val)
{
	Registry& registry = *CEO::Instance().GetManager < Registry>();
	AudioSliderScript* slider = GetComponent<AudioSliderScript>(registry);
	UITransformComponent* backTransform = background.GetComponent<UITransformComponent>();
	UITransformComponent* buttonTransform = handle.GetComponent<UITransformComponent>();
	UITransformComponent* frontTransform = fill.GetComponent<UITransformComponent>();
	SpriteRendererComponent* frontSprite = fill.GetComponent<SpriteRendererComponent>();

	// clamp value to 0 - 1
	if (val > 1.f) val = 1.f;
	if (val < 0) val = 0;

	// update transform to value
	buttonTransform->relativePos.x = backTransform->relativePos.x + val * backTransform->size.x;
	frontTransform->size.x = val * backTransform->size.x;
	frontSprite->size.x = val;

	slider->value = val;
}
void AudioSliderScript::UpdateSlider()
{
	Registry& registry = *CEO::Instance().GetManager < Registry>();
	AudioSliderScript* slider = GetComponent<AudioSliderScript>(registry);
	UITransformComponent* backTransform = background.GetComponent<UITransformComponent>();
	UITransformComponent* buttonTransform = handle.GetComponent<UITransformComponent>();
#ifdef PLATFORM_WINDOWS
	currMouseX = Input::GetGameX();
#endif
#ifdef PLATFORM_ANDROID
	currMouseX = Input::GetScreenX(0);
    if (resetMouse)
    {
        resetMouse = false;
        originPosX = buttonTransform->relativePos.x;
        prevMouseX = currMouseX;
        LOGI("%f , %f", currMouseX, prevMouseX);
    }
#endif

#ifdef PLATFORM_WINDOWS
    if (resetMouse && !Input::IsGamepadConnected(0))
    {
        resetMouse = false;
        originPosX = buttonTransform->relativePos.x;
        prevMouseX = currMouseX;
        LOGI("%f , %f", currMouseX, prevMouseX);
    }
	if (!Input::IsGamepadConnected(0))
	{
		 offset = (currMouseX - prevMouseX);
	}
	else
	{
	
		if (Input::GetLeftStick().x > .5f || Input::GetLeftStick().x < -.5f)
			offset += Input::GetLeftStick().x * 5;

		offset = std::clamp(offset, -backTransform->size.x * .5f, backTransform->size.x * .5f);

	}
#else
	 offset = (currMouseX - prevMouseX);
#endif
	LOGI("Offset %f", offset);

		buttonTransform->relativePos.x = originPosX + offset;

	SetSliderValue((buttonTransform->relativePos.x - backTransform->relativePos.x) / backTransform->size.x);

	CEO::Instance().GetManager<ResourceManager>()->SetGroupVars(slider->audioGroup, slider->value * slider->maxVolume);
}

void AudioSliderScript::ResetMouseX()
{
	resetMouse = true;
	GameObject go(entity);
	for (auto& handleGo : go.GetChildrenWithComponent<ButtonComponent>())
	{
		handleGo.GetComponent<SpriteRendererComponent>()->texture = &CEO::Get<ResourceManager>()->GetTexture("ui/menus/slidericon.png");
	}

}


void AudioSliderScript::ResetMouseXHoverExit()
{
	LOGI("WEEEE");
	resetMouse = true;
	GameObject go(entity);
	for (auto& handleGo : go.GetChildrenWithComponent<ButtonComponent>())
	{
		handleGo.GetComponent<SpriteRendererComponent>()->texture = &CEO::Get<ResourceManager>()->GetTexture("ui/menus/slidericon_nowhite.png");
	}
}