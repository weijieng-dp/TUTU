	/*!
@file       GameStateManagerScript.h    
@author     Ng Wei Jie
@date       5/2/2026
@brief
    Implements the GameStateManagerScript, which manages high-level
    game states such as main menu, map, and pause states.

    The script functions as a singleton-style controller responsible
    for initializing the game state, handling pause toggling, and
    managing state-dependent audio transitions.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*/
/*________________________________________________________________________*/


#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class GameStateManagerScript : public ScriptInstance
{
    static GameStateManagerScript instance;
public:
	void BindFrom() { 
        GetComponent<GameStateManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<GameStateManagerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<GameStateManagerScript>(*CEO::Get<Registry>()) = *this; };

    	/*!
* \brief
*   Default constructor for the GameStateManagerScript.
*
*   Initialization logic is deferred to OnStart to ensure the script
*   is only initialized once globally.
*/
    GameStateManagerScript();
    enum GameStates { MainMenu, Map, Game };
    static GameStateManagerScript& Instance() { return instance; }
    bool initialized = false;

    	/*!
* \brief
*   Performs one-time initialization of the game state manager.
*
*   Sets the initial game state, resets pause status, and starts
*   the main menu background music. Initialization is guarded to
*   ensure this logic only runs once.
*/
    void OnStart(Registry& registry);

    	/*!
* \brief
*   Handles per-frame game state input checks.
*
*   On supported platforms, listens for pause input and toggles
*   the paused state accordingly.
*/
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    	/*!
* \brief
*   Fixed timestep update callback.
*
*   Currently unused as game state logic is not dependent on
*   physics or fixed-step updates.
*/
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);


    GameStates GetState() const { return currentGameState; }

    	/*!
* \brief
*   Changes the current game state and performs state-specific logic.
*
* \param
*   state - The new game state to transition into.
*
*   Handles audio transitions and updates internal state tracking
*   based on the specified game state.
*/
    void SetState(GameStates state);

        	/*!
* \brief
*   Toggles the game's paused state.
*
*   Updates the active update stack to enable or disable gameplay
*   systems and logs transitions between paused and unpaused states.
*/
	void PauseGame(Registry& registry);
    bool paused{};
private:
    GameStates currentGameState{};
public:
    REFLECTABLE_PROPERTIES;

};
REFL_AUTO(
    type(GameStateManagerScript)
)