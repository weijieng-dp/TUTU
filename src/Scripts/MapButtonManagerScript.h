/**___________________________________________________________________________/
@file       MapButtonManagerScript.h
@author     Tan Jun Jie (t.junjie) (50%)
@author     Ng Wei Jie (weijie.ng) (30%)
@author     LORENZO YONGYONG De Guzman Adrian (d.lorenzoyongoyong) (20%)
@date		03/02/2026 (DD/MM/YYYY)
@brief		Handles button behaviour in Map.scene, for player to cycle through
            available tiles for players to place down.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class MapButtonManagerScript : public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<MapButtonManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<MapButtonManagerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<MapButtonManagerScript>(*CEO::Get<Registry>()) = *this; };

    EntityRegistry::Entity DisplayTileEntity = 0;           // entity id of the display tile
    EntityRegistry::Entity tileManagerScriptEntity = 0;     // entity id of the tile manager script

    int TileNumber = 0;         // which tile is on display (0, 1, 2)
    std::string currentSceneName;
    std::string switchScene;
    bool isTransitioning = false;
    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    GameObject fadeBG;

    /*!
    * \brief Helper function for binding buttons.
    * \param[in, out] registry      - Entity registry.
    * \param[in, out] button        - The button component to bind.
    * \param[in, out] buttonSprite  - The default sprite component of button entity.
    * \param[in] hoverTex           - The sprite filename for hovering on this button (empty string for no hover sprite change).
    * \param[in] idleTex            - The default sprite for button (to change back upon exiting hover).
    * \param[in] switchScene        - Whether this button switches scene.
    * \param[in] sceneToSwitch      - What scene this button switches to.
    */
    void AssignButton(Registry& registry, ButtonComponent& button, SpriteRendererComponent* buttonSprite, 
        std::string hoverTex, std::string idleTex, bool switchScene, std::string sceneToSwitch,
        bool active = true);

    /*!
    * \brief Helper function to bind for on button clicked in Map scene.
    * \param[in, out] registry          - Entity registry.
    * \param[in, out] mainMenuButton    - The mapbuttonmanagerscript component.
    * \param[in] switchScene            - Whether this button switches scene.
    * \param[in] sceneToSwitch          - What scene this button switches to.
    */
    static void ButtonClicked(Registry& registry, MapButtonManagerScript* mainMenuButton,
        bool sceneSwitch, std::string sceneToSwitch);

    /*!
    * \brief Helper function to bind for on button pressed in Map scene.
    * \param[in, out] registry          - Entity registry.
    * \param[in, out] mainMenuButton    - The mapbuttonmanagerscript component.
    * \param[in] switchScene            - Whether this button switches scene.
    * \param[in] sceneToSwitch          - What scene this button switches to.
    */
    static void ButtonPressed(Registry& registry, MapButtonManagerScript* mainMenuButton,
        bool sceneSwitch, std::string sceneToSwitch);

    /*!
    * \brief Helper function update button sprite texture on hover enter and exit.
    * \param[in, out] buttonSprite  - The button's sprite component
    * \param[in] texPath            - The sprite texture file name to change the button sprite to.
    * \param[in] hover              - Whether current behaviour is on hover (true) or exiting hover (exiting).
    */
    static void SetSprite(SpriteRendererComponent* buttonSprite, std::string texPath, bool hover);

    /*!
    * \brief Update the Play button sprite when map tile has been placed.
    * \param[in] placed - Whether map tile has been placed.
    * \param[in] reset
    */
    void UpdatePlayButton(bool placed);
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(MapButtonManagerScript),
    field(currentSceneName),
    field(fadeBG)
)