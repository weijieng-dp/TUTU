/*!
@file       ConfirmSceneScript.h
@author     De Guzman Adrian Lorenzo Yongoyong (d.lorenzoyongoyong) 100%
@date       20/01/2026
@brief		Declares the ConfirmScene script class responsible for handling
            functionality of the various Confirm Scenes in the game

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class ConfirmSceneScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *	Called once when the script is initialized. Used for setup logic such as
    *	logging or initializing internal state.
    * \param
    *	registry - ECS registry instance used to access entity components
    */
    void OnStart(Registry& registry);
    
    /*!
    * \brief
    *	Called every frame to process if player wants to exit the confirm scene
    * \param
    *	registry - ECS registry instance used to access entity components
    * \param
    *	dt - delta time for frame update
    * \param
    *	firstframe - true if this is the first frame after initialization
    */
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *	Called at a fixed timestep for physics or deterministic updates.
    *	This implementation is currently empty but can be extended.
    * \param
    *	registry - ECS registry instance used to access entity components
    * \param
    *	dt - fixed timestep delta
    * \param
    *	firstframe - true if this is the first fixed update after startup
    */
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    
    /*!
    * \brief
    *	Function that is attached to button on click
    * \param
    *   pressedSprite - file path of pressed state button texture
    * \param
    *   button - Button GameObject to attach button click to
    */
    void ButtonClicked(std::string pressedSprite, GameObject button);

    /*!
    * \brief
    *	Function that is attached to button on hover enter
    * \param
    *   hoverSprite - file path of hover state button texture
    * \param
    *   button - Button GameObject to attach button click to
    */
    void ButtonHoverEnter(std::string hoverSprite, GameObject button);

    /*!
    * \brief
    *	Function that is attached to button on hover exit
    * \param
    *   defaultSprite - file path of default state button texture
    * \param
    *   button - Button GameObject to attach button click to
    */
    void ButtonHoverExit(std::string defaultSprite, GameObject button);

    /*!
    * \brief
    *	Function that handles grabbing of texture from ResourceManager
    *   and assigning it to the button.
    * \param
    *   texturePath - file path of texture to assign
    * \param
    *   buttonSprite - SpriteRendererComponent of the button to attach to
    * \param
    *   hover - true if on hover enter
    */
    void AssignButtonTexture(std::string texturePath, SpriteRendererComponent* buttonSprite, bool hover);

    GameObject yesButton;
    GameObject noButton;
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ConfirmSceneScript),
    field(yesButton),
    field(noButton)
)