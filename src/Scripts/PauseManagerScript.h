/*!
@file       PauseManagerScript.h
@author     De Guzman Adrian Lorenzo Yongoyong (d.lorenzoyongoyong) 100%
@date       15/01/2026
@brief		Declares the PauseManager script class responsible for handling
            functionality of the Pause Scene in the game

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class PauseManagerScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<PauseManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<PauseManagerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<PauseManagerScript>(*CEO::Get<Registry>()) = *this; };

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
    *	Called every frame to process in scene transition by playing
    *   the book animation
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

    GameObject bookTransition;

    GameObject settingsPanel;
    GameObject faqPanel;
    GameObject windowsPanel;
    GameObject androidPanel;

    GameObject resumeButton;
    GameObject homeButton;
    GameObject settingsButton;
    GameObject faqButton;

    std::vector<GameObject> buttonList;
    bool isTransitioning{};

    enum MenuState
    {
        Settings,
        FAQ
    };

    MenuState currentState;

    /*!
    * \brief
    *	Assigns the three main states for a given button.
    * \param
    *   button - Button GameObject to settle all the states
    * \param
    *   buttonName - string name of given button
    */
    void AssignAllButtons();

    /*!
    * \brief
    *	Assigns the three main states for a given button.
    * \param
    *   button - Button GameObject to settle all the states
    * \param
    *   buttonName - string name of given button
    */
    void AssignButton(GameObject button, std::string buttonName);

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
   
    REFLECTABLE_PROPERTIES;

};
REFL_AUTO(
    type(PauseManagerScript),
    field(settingsPanel),
    field(faqPanel),
    field(resumeButton),
    field(homeButton),
    field(settingsButton),
    field(faqButton),
    field(bookTransition),
    field(windowsPanel),
    field(androidPanel)
)