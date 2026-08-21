/**___________________________________________________________________________/
@file       StampManagerScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       05/03/2026   (DD/MM/YYYY)
@brief      Script that handles the achievement stamp display on the Main Menu
            scene. Maps each stamp GameObject to an Achievement, updates
            their sprites based on unlock status, and manages a hover tooltip
            that displays achievement details when a stamp is hovered over.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/GameObjects.h"


class StampManagerScript: public ScriptInstance
{
private:

    /*!
     * \brief Initializes the stamp-to-achievement map, instantiates the hover
     *        prefab, and calls UpdateStamps for the first time.
     */
    void Init();

    /*!
     * \brief Shows the hover tooltip and populates it with the given achievement's data.
     * \param[in] ach - The achievement whose data will be displayed in the tooltip.
     */
    void ShowHover(Achievement ach);

    /*!
     * \brief Hides the hover tooltip.
     */
    void HideHover();

public:
	void BindFrom() { 
        GetComponent<StampManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<StampManagerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<StampManagerScript>(*CEO::Get<Registry>()) = *this; };


    /*!
     * \brief Called once when the script starts. Calls Init to set up stamps.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Updates the hover tooltip position to follow
     *        the cursor.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the update.
     */
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
     * \brief Called every fixed timestep. Currently unused.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Fixed delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the fixed update.
     */
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
     * \brief Updates each stamp's sprite based on whether its associated
     *        achievement is unlocked. Also binds hover callbacks to each stamp's
     *        button component.
     */
    void UpdateStamps();
   
    GameObject stamp0{};  // First stamp GameObject
    GameObject stamp1{};  // Second stamp GameObject
    GameObject stamp2{};  // Third stamp GameObject
    GameObject stamp3{};  // Fourth stamp GameObject
    GameObject stamp4{};  // Fifth stamp GameObject
    GameObject stamp5{};  // Sixth stamp GameObject
    GameObject stamp6{};  // Seventh stamp GameObject
    GameObject stamp7{};  // Eighth stamp GameObject
    GameObject stamp8{};  // Ninth stamp GameObject
    GameObject hoverPrefab; // Prefab to instantiate for the hover tooltip
    GameObject hover;       // Instantiated hover tooltip GameObject
    GameObject lastStamp;   // Special stamp shown when the ending cutscene has been displayed
    GameObject selectedEntity;
    std::map<GameObject*, Achievement> stampAchievements{}; // Maps each stamp GameObject to its corresponding Achievement
    std::map<int, std::string> stampSprite{                 // Maps achievement index to its filled stamp sprite path
        {0, "ui\\menus\\stamp1.png"},
        {1, "ui\\menus\\stamp2.png"},
        {2, "ui\\menus\\stamp3.png"},
        {3, "ui\\menus\\stamp4.png"},
        {4, "ui\\menus\\stamp5.png"},
        {5, "ui\\menus\\stamp6.png"},
        {6, "ui\\menus\\stamp7.png"},
        {7, "ui\\menus\\stamp8.png"},
        {8, "ui\\menus\\stamp9.png"}
    };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(StampManagerScript),
    field(stamp0),
    field(stamp1),
    field(stamp2),
    field(stamp3),
    field(stamp4),
    field(stamp5),
    field(stamp6),
    field(stamp7),
    field(stamp8),
    field(hoverPrefab),
    field(lastStamp)
)