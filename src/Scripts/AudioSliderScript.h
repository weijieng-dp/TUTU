/*!
@file       AudioSliderScript.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Slider to control volume of audio in game

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class AudioSliderScript : public ScriptInstance
{
public:
	void BindFrom() { 
        		GetComponent<AudioSliderScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<AudioSliderScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<AudioSliderScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    
    /*!
    * \brief
    *    setter for slider value
    * \param
    * val - 0 to 1 value to set
    */
    void SetSliderValue(float val);
    /*!
    * \brief
    *    update slider input/visuals
    */
    void UpdateSlider();
    /*!
    * \brief
    *    reset flag to capture last mouse click position
    */
    void ResetMouseX();
    void ResetMouseXHoverExit();
    float originPosX{};
    float prevMouseX{}, currMouseX{};
    bool resetMouse = true;

    float value = 0.5f;
    float maxVolume = 2.f;
    float offset{};
    Registry::Entity button{}, back{}, front{};
    std::string audioGroup;

    GameObject handle{};
    GameObject fill{};
    GameObject background{};
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AudioSliderScript),
    field(maxVolume),
    field(audioGroup),
    field(handle),
    field(fill),
    field(background)
)        