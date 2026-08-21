/**___________________________________________________________________________/
@file       PopupMangerScript.h
@author		d.lorenzoyongoyong@digipen.edu
@date       03/03/2026	(DD/MM/YYYY)
@brief		Manager that handles achievements popup and 

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/GameObjects.h"


class PopupManagerScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<PopupManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<PopupManagerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<PopupManagerScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);


    /*!
     * \brief Elastic easing function for smooth popup animation.
     *        Maps a normalized time value t [0,1] to an eased output.
     * \param[in] t - Normalized time value between 0 and 1.
     * \return Eased float value.
     */
    float EaseInOutElastic(float t);

    float startX{ -2700 };             // Starting X position of the popup (off-screen left)
    float endX{ 2700 };                // Ending X position of the popup (off-screen right)
   
    float timer = 0.0f;                // Tracks elapsed time for the current animation state
    float slideDuration = 1.0f;        // Duration in seconds for the slide in/out animation
    float waitDuration = 0.5f;         // Duration in seconds the popup stays visible at center
    bool isAnimatingIn = false;        // Whether the popup is currently sliding in
    bool isAnimatingOut = false;       // Whether the popup is currently sliding out
    bool soundPlayed = false;          // Whether the popup sound has been played for the current popup
    bool isWaiting = false;            // Whether the popup is currently in the wait state at center
    
    GameObject popupPrefab;            // Prefab to instantiate for each achievement popup
    int index{ 0 };                    // Current index in the popup sequence
    bool showingPopup = true;          // Whether the popup system is currently active
    
    std::queue<GameObject> popupQueue; // Queue of instantiated popup GameObjects to display

   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(PopupManagerScript),
    field(popupPrefab)
)