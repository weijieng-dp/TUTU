/*!
@file       BlindManipulationScript.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026 (DD/MM/YYYY0
@brief		Simple script to increase / decrease the vignette for blind
            gimmick tile.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class BlindManipulationScript: public ScriptInstance {
public:
	void BindFrom() { 
        GetComponent<BlindManipulationScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<BlindManipulationScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<BlindManipulationScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief Function to enable / disable blind mode.
    * \param[in] blind - True to enable blind, false to disable blind.
    */
    void EnableBlind(bool blind);

    float vignetteFalloffDuration{ 1.5f },       // lerp duration for vignette falloff
        vignetteIntensityDuration{ 1.5f };       // lerp duration for vignette intensity

    float targetVignetteFalloff{ 2.f },         // target vignette falloff setting to lerp to for enabling blind 
        targetVignetteIntensity{ 1.f };         // target vignette intensity setting to lerp to for enabling blind

    bool disableMinimap{ true };                // whether to disable minimap when blind is on

    REFLECTABLE_PROPERTIES;
private:
    float lerpTimer{};                          // timer for lerping
    float initialVignetteFalloff{}, initialVignetteIntensity{};   // initial vignette values to restore for disabling blind
    float falloffDelta{}, intensityDelta{};     // pre-calcualted delta to make lerping less expensive
    bool lerping{ false };                      // whether it is lerping
    bool toBlind{ true };                       // whether to lerp is for blind on or off
};
REFL_AUTO(
    type(BlindManipulationScript),
    field(vignetteFalloffDuration),
    field(vignetteIntensityDuration),
    field(targetVignetteFalloff),
    field(targetVignetteIntensity),
    field(disableMinimap)
)