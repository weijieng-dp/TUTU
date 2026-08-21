/*!
@file       AudioLinker.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple script to link audio playing to a button

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class AudioLinker : public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<AudioLinker>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<AudioLinker>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<AudioLinker>(*CEO::Get<Registry>()) = *this; };
    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *    Plays attached audio component on button press
    */
    void PlayAudio();
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AudioLinker)
)