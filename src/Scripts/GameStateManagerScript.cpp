	/*!
@file       GameStateManagerScript.cpp
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

	
	#include "GameStateManagerScript.h"
	#include "../CoreLib/CEO.h"
	#include "../CoreLib/UpdateStackManager.h"
	#include <functional>
	#include "../CoreLib/Camera.h"

	GameStateManagerScript GameStateManagerScript::instance;


	GameStateManagerScript::GameStateManagerScript()
	{
	}


	void GameStateManagerScript::OnStart(Registry&){
		if (GameStateManagerScript::Instance().initialized) return;
		GameStateManagerScript::Instance().initialized = true;
		GameStateManagerScript::Instance().paused = false;
		GameStateManagerScript::Instance().currentGameState = MainMenu;
		//CEO::Instance().GetManager<ResourceManager>()->GetAudio("BGM\\Get Me Out Of Hell.wav", "BGM", true, true).Play();
	};


	void GameStateManagerScript::OnUpdate(Registry& r,float , bool ){
		// Toggling pause state.
	#ifdef PLATFORM_WINDOWS
		if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
			PauseGame(r);
		}
	#endif
	}


	void GameStateManagerScript::OnFixedUpdate(Registry& ,float , bool ){}


	void GameStateManagerScript::PauseGame(Registry& )
	{
		auto& usm = *CEO::Instance().GetManager<UpdateStackManager>();
		//auto cams = r.GetEntitiesWithComponent<CameraComponent>()[0];
		//auto cam = r.GetComponent<CameraComponent>(cams);
		//auto layerManager = CEO::Instance().GetManager<LayerManager>();
		usm.SetStack((usm.GetStack() == 0 ? 1 : 0));
		if (usm.GetStack() == 1)
		{
			LOGI("Entering paused state");
			/*cam->SetMainCamMask(r, 23, true);
			cam->SetMainCamMask(r, 20, true);*/
			//cam->cullingMask |= layerManager->GetLayerMask(0);
			//cam->cullingMask |= layerManager->GetLayerMask(23);
			//cam->cullingMask |= layerManager->GetLayerMask(20);

		}
		else
		{
			LOGI("Exiting paused state");
			/*cam->SetMainCamMask(r, 23, false);
			cam->SetMainCamMask(r, 20, false);*/
			//cam->cullingMask &= layerManager->GetLayerMask(23);
			//cam->cullingMask &= layerManager->GetLayerMask(20);
			//cam->cullingMask |= layerManager->GetLayerMask(0);
		}
	}



	void GameStateManagerScript::SetState(GameStates state) {
		// state changing logic
		switch (state) {
		case GameStateManagerScript::Map:
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\It_s Scary Here.wav");
			break;
		}
		currentGameState = state;
	}