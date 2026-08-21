/*!
@file       MinimapScript.h
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Handles minimap behaviour (such as moving / updating 
            minimap camera when player enters a new tile/room, lerping 
            the minimap's position and viewport when moving, and toggling 
            it on/off if "ToggleMinimapOffDuringCombat" flag is set.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class MinimapScript : public ScriptInstance {
public:
    void BindFrom() {
        GetComponent<MinimapScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<MinimapScript>(*CEO::Get<Registry>());
    };
    void BindTo() { *GetComponent<MinimapScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *	Update minimap camera to snap center of the specified tile. If 
    *   IsCombat flag is set, zooms in the viewport of the camera to fit
    *   only the the tile, else sets target viewport size to the initial size.
    * 
    * \param[in] tileIndex 
    *   - The tile instance index of the tile to snap minimap camera to.
    */
    void UpdateMinimapCam(int tileIndex);

    /*!
    * \brief Update minimap camera position to lerp to target and viewport size to vpSize.
    * \param[in] target - The target world position to lerp minimap camera to.
    * \param[in] vpSize - The target viewport size to lerp minimap camera to.
    */
    void UpdateMinimapCam(const Vec2& target, const Vec2& vpSize);

    float minimapCamPosLerpDuration{ 3.f };     // Lerp duration for minimap camera position
    float minimapCamSizeLerpDuration{ 1.5f };   // Lerp duration for minimap camera viewport size

    bool toggleMinimapOffDuringCombat{ false };  // turn off minimap during combat
    REFLECTABLE_PROPERTIES;
private:
    Vec2 targetPos{}, posDelta{};               // minimap camera position target to lerp to and position delta
    Vec2 targetVpSize{}, vpDelta{};             // minimap camera viewport size target to lerp to and viewport size delta

    float lerpTimer{};
  
    bool isMinimapCamLerping{ false };          // a flag to track whether minimap camera is lerping
};
REFL_AUTO(
    type(MinimapScript),
    field(minimapCamPosLerpDuration),
    field(minimapCamSizeLerpDuration),
    field(toggleMinimapOffDuringCombat)
)